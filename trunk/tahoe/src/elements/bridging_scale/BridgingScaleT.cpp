/* $Id: BridgingScaleT.cpp,v 1.22 2002-08-19 17:59:57 paklein Exp $ */
#include "BridgingScaleT.h"

#include <iostream.h>
#include <iomanip.h>

#include "ShapeFunctionT.h"
#include "RodT.h"
#include "fstreamT.h"
#include "iAutoArrayT.h"
#include "OutputSetT.h"
#include "AutoFill2DT.h"
#include "RaggedArray2DT.h"
#include "iGridManagerT.h"
#include "iNodeT.h"
#include "nArrayGroupT.h"

using namespace Tahoe;

/* constructor */
BridgingScaleT::BridgingScaleT(const ElementSupportT& support, 
	const FieldT& field,
	const RodT& particle,
	const ElasticT& solid):
	ElementBaseT(support, field),
	fParticle(particle),
	fSolid(solid),
	fElMat(ShapeFunction().ParentDomain().NumNodes(), ElementMatrixT::kSymmetric),
	fLocInitCoords(LocalArrayT::kInitCoords),
	fLocDisp(LocalArrayT::kDisp),
	fDOFvec(NumDOF()),
	fGlobal(support.Output(),1),
	fMass(ShapeFunction().ParentDomain().NumNodes()),
	fConnect(solid.NumElements(), solid.NumElementNodes()),
	fWtempU(ShapeFunction().ParentDomain().NumNodes(), NumDOF()),
	fMassInv(ShapeFunction().ParentDomain().NumNodes())
{

}

/* destructor */
BridgingScaleT::~BridgingScaleT(void)
{	

}

/* allocates space and reads connectivity data */
void BridgingScaleT::Initialize(void)
{
	/* inherited */
	ElementBaseT::Initialize();

	/* stream */
	ostream& out = ElementSupport().Output();
	
	/* initial coordinates of all nodes/points in the calculation */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();

	/* distinguish FEM nodes vs. atoms to separate their respective coordinates */
	iArrayT atoms_used, nodes_used;
	fParticle.NodesUsed(atoms_used);
	fSolid.NodesUsed(nodes_used);
	iArrayT node1(nodes_used.Max() + 1); // +1 because starts from 0
	node1 = -1;
	for (int i = 0; i < nodes_used.Length(); i++)
	    node1[nodes_used[i]] = i + 1;

	atoms_used++;
	out << " Particles used:\n" << atoms_used.wrap(5) << '\n';
	atoms_used--;
	nodes_used++;
	out << " Nodes used:\n" << nodes_used.wrap(5) << '\n';
	nodes_used--;

	/* now take an atom, check against every element */
	const ParentDomainT& parent = ShapeFunction().ParentDomain();
	int totalatoms = atoms_used.Length();
	fTotalNodes = nodes_used.Length();
	AutoFill2DT<int> auto_fill(fSolid.NumElements(), 10, 10);
	dArrayT x_atom, centroid;
	LocalArrayT cell_coords(LocalArrayT::kCurrCoords, fSolid.NumElementNodes(), NumSD());
	cell_coords.SetGlobal(init_coords); // Sets address of cell_coords

	iGridManagerT grid(10, 100, init_coords, &atoms_used);
	grid.Reset();
	for (int i = 0; i < fSolid.NumElements(); i++) {
	
			/* gives domain (global) nodal coordinates */
	                cell_coords.SetLocal(fSolid.ElementCard(i).NodesX());

			/* get global connectivity matrix */
		        fConnect.SetRow(i, fSolid.ElementCard(i).NodesX());
			/* inverse map from global node numbers to local node numbers */
			for (int j = 0; j < fSolid.NumElementNodes(); j++)
			{
			    int x = fConnect(i,j);
			    fConnect(i,j) = node1[x];
			}

			/* centroid and radius */
			double radius = parent.AverageRadius(cell_coords, centroid);

			/* candidate particles */
			const AutoArrayT<iNodeT>& hits = grid.HitsInRegion(centroid.Pointer(), 1.01*radius);

			for (int j = 0; j < hits.Length(); j++)
			{
				x_atom.Set(NumSD(), hits[j].Coords());
				if (parent.PointInDomain(cell_coords, x_atom)) 
				        auto_fill.Append(i, hits[j].Tag());
			}
	}

	/* copy/compress contents */
	fParticlesInCell.Copy(auto_fill);
	iArrayT tmp(fParticlesInCell.Length(), fParticlesInCell.Pointer());
	out << " Particles in cells:\n"
	    << setw(kIntWidth) << "no." << '\n';
	tmp++;
	fParticlesInCell.WriteNumbered(out);
	tmp--;
	out.flush();

	if (fParticlesInCell.Length() > atoms_used.Length()) {
		cout << "\n BridgingScaleT::Initialize: WARNING: number of particles in cells " 
		     << fParticlesInCell.Length() << " exceeds\n" 
		     <<   "     the total number of particles " << atoms_used.Length() << endl;
	}

	// (2) compute the inverse map using list fParticlesInCell
	LocalArrayT cell_coord(LocalArrayT::kInitCoords, fSolid.NumElementNodes(), NumSD());
	cell_coord.SetGlobal(init_coords); // Sets address of cell_coords
	iArrayT atom_nums;
	dArrayT mapped(NumSD()), point(NumSD());
	dArray2DT atom_coords(fParticlesInCell.MaxMinorDim(), NumSD());
	AutoFill2DT<double> inverse(fParticlesInCell.MajorDim(),10,10);
	for (int i = 0; i < fParticlesInCell.MajorDim(); i++) {

	                fParticlesInCell.RowAlias(i,atom_nums);
	                atom_coords.RowCollect(atom_nums,init_coords);
			/* gives domain (global) nodal coordinates */
	                cell_coord.SetLocal(fSolid.ElementCard(i).NodesX()); 
			
	                for (int j = 0; j < fParticlesInCell.MinorDim(i); j++) 
			{
			    //need a ColumnAlias function in nArray2DT similar to RowAlias!
			    atom_coords.RowAlias(j, point);
			    if (parent.MapToParentDomain(cell_coord,point,mapped))
				inverse.Append(i,mapped);   
			}
	}
        fInverseMapInCell.Copy(inverse);
	dArrayT temp(fInverseMapInCell.Length(), fInverseMapInCell.Pointer());
	out << " Inverse maps in cell:\n"
	    << setw(kDoubleWidth) << "no." << '\n';
	fInverseMapInCell.WriteNumbered(out);
	out.flush();	

	/* store solid nodes used */
	fSolidNodesUsed.Dimension(fTotalNodes, 1);
	fSolidNodesUsed.SetColumn(0, nodes_used);

	/* store particles used */
	fParticlesUsed.Dimension(atoms_used.Length(), 1);
	fParticlesUsed.SetColumn(0, atoms_used);
}

void BridgingScaleT::Equations(AutoArrayT<const iArray2DT*>& eq_1,
	AutoArrayT<const RaggedArray2DT<int>*>& eq_2)
{
	/* inherited */
	ElementBaseT::Equations(eq_1, eq_2);
}

/* initialize/finalize step */
void BridgingScaleT::InitStep(void)
{
	/* inherited */
	ElementBaseT::InitStep();
}

/* initialize/finalize step */
void BridgingScaleT::CloseStep(void)
{
	/* inherited */
	ElementBaseT::CloseStep();
	
	/* Do Bridging scale calculations after MD displacements
	   computed using RodT */
	CoarseFineU();
}

/* resets to the last converged solution */
void BridgingScaleT::ResetStep(void)
{
	/* inherited */
	ElementBaseT::ResetStep();
}

/* writing output */
void BridgingScaleT::RegisterOutput(void)
{
	/* collect variable labels */
	int ndof = NumDOF();
	if (ndof > 3) throw;
	ArrayT<StringT> n_labels(2*ndof);
	const char* coarse_labels[] = {"FE_X", "FE_Y", "FE_Z"};
	const char* fine_labels[] = {"fine_X", "fine_Y", "fine_Z"};
	int dex = 0;
	for (int i = 0; i < ndof; i++) n_labels[dex++] = coarse_labels[i];
	for (int i = 0; i < ndof; i++) n_labels[dex++] = fine_labels[i];

	/* group ID */	
	StringT set_ID;
	set_ID.Append(ElementSupport().ElementGroupNumber(this) + 1);

	/* register output at solid nodes */
#if 0
	OutputSetT output_set_solid(set_ID, GeometryT::kPoint, fSolidNodesUsed, n_labels);
	fSolidOutputID = ElementSupport().RegisterOutput(output_set_solid);
#endif

	/* register output at particles */
	OutputSetT output_set_particle(set_ID, GeometryT::kPoint, fParticlesUsed, n_labels);
	fParticleOutputID = ElementSupport().RegisterOutput(output_set_particle);
}

//NOTE - this function is/was identical to CSEBaseT::WriteOutput
void BridgingScaleT::WriteOutput(IOBaseT::OutputModeT mode)
{
//TEMP - not handling general output modes yet
	if (mode != IOBaseT::kAtInc)
	{
		cout << "\n BridgingScaleT::WriteOutput: only handling \"at increment\"\n"
		     <<   "     print mode. SKIPPING." << endl;
		return;
	}

	/* calculate output values */
	dArray2DT n_values; // dimension: [number of output nodes] x [number of values]
//	ComputeOutput(n_counts, n_values);
//  rows in n_values correspond to the nodes listed in fSolidNodesUsed

	/* send to output */
//	ElementSupport().WriteOutput(fOutputID, n_values);
}

/***********************************************************************
* Protected
***********************************************************************/

/* initialize local arrays */
void BridgingScaleT::SetLocalArrays(void)
{
	/* dimension */
	fLocInitCoords.Allocate(NumElementNodes(), NumSD());
	fLocDisp.Allocate(NumElementNodes(), NumDOF());

	/* set source */
	ElementSupport().RegisterCoordinates(fLocInitCoords);
	Field().RegisterLocal(fLocDisp);	
}

/* print element group data */
void BridgingScaleT::PrintControlData(ostream& out) const
{
	/* inherited */
	ElementBaseT::PrintControlData(out);

	out << " Particle group number . . . . . . . . . . . . . = " << ElementSupport().ElementGroupNumber(&fParticle) + 1 << '\n';
	out << " Continuum group number. . . . . . . . . . . . . = " << ElementSupport().ElementGroupNumber(&fSolid) + 1 << '\n';
}

/* write all current element information to the stream */
void BridgingScaleT::CurrElementInfo(ostream& out) const
{
	/* inherited */
	ElementBaseT::CurrElementInfo(out);
	dArray2DT temp;
	temp.Allocate(fLocInitCoords.NumberOfNodes(), fLocInitCoords.MinorDim());
	
	out <<   " initial coords:\n";
	temp.Allocate(fLocInitCoords.NumberOfNodes(), fLocInitCoords.MinorDim());
	fLocInitCoords.ReturnTranspose(temp);
	temp.WriteNumbered(out);

	out <<   " displacements:\n";
	temp.Allocate(fLocDisp.NumberOfNodes(), fLocDisp.MinorDim());
	fLocDisp.ReturnTranspose(temp);
	temp.WriteNumbered(out);
}

/***********************************************************************
* Private
***********************************************************************/

void BridgingScaleT::CoarseFineU(void)
{
  /* computes coarse and fine scale displacement fields */
  const ParentDomainT& parent = ShapeFunction().ParentDomain();
  const FieldT& field = Field();
  iArrayT atoms, elemconnect(parent.NumNodes());
  const dArray2DT& displacements = field[0];
  fFineScaleU.Dimension(fParticlesUsed.MajorDim(), displacements.MinorDim());
  dArrayT map, shape(parent.NumNodes()), temp(NumSD()), disp;
  dMatrixT tempmass(parent.NumNodes()), Nd(parent.NumNodes(), NumSD()), wglobal(fTotalNodes, NumSD());
  /* set global matrices for bridging scale calculations */
  fGlobal.AddEquationSet(fConnect);
  fGlobal.Initialize(fTotalNodes,fTotalNodes,1); // hard wired for serial calculations
  fGlobal.Clear();
  wglobal = 0.0;

  for (int i = 0; i < fParticlesInCell.MajorDim(); i++)
  {
      fElMat = 0.0, fWtempU = 0.0;
      fParticlesInCell.RowAlias(i,atoms);
      fInverseMapInCell.RowAlias(i,map);
      fConnect.RowAlias(i,elemconnect);
      for (int j = 0; j < fParticlesInCell.MinorDim(i); j++)
      {
	  // still need to access individual atomic masses!!!
	  displacements.RowAlias(atoms[j],disp);
	  temp.CopyPart(0, map, NumSD()*j, NumSD());
	  parent.EvaluateShapeFunctions(temp,shape);
	  tempmass.Outer(shape,shape);
	  fElMat += tempmass;
	  Nd.Outer(shape, disp);
	  fWtempU += Nd;
      }
      
      /* Assemble local equations to global equations for LHS */
      fGlobal.Assemble(fElMat, elemconnect);

      /* Assemble local equations to global equations for RHS */
      for (int i = 0; i < parent.NumNodes(); i++)
      {
	for (int j = 0; j < NumSD(); j++)
	  wglobal(elemconnect[i] - 1,j) += fWtempU(i,j);
      }
  }
  
  /* Solve for global coarse scale (FEM) displacements */
  if (NumSD() == 1)
    {
      wglobal.ColumnAlias(0, fWx);
      fGlobal.Solve(fWx);
      cout << "Wx = \n" << fWx << endl;
    }
  else if (NumSD() == 2)
    {
      wglobal.ColumnAlias(0, fWx);
      wglobal.ColumnAlias(1, fWy);
      fGlobal.Solve(fWx);
      fGlobal.Solve(fWy);
      cout << "Wx = \n" << fWx << endl;
      cout << "Wy = \n" << fWy << endl;
    }

  /* Compute resulting fine scale fields = MD - FE */
  dArrayT wx(parent.NumNodes()), wy(parent.NumNodes());
  iArrayT conn(parent.NumNodes());
  double nwx, nwy;

  for (int i = 0; i < fParticlesInCell.MajorDim(); i++)
  {
    fParticlesInCell.RowAlias(i,atoms);
    fInverseMapInCell.RowAlias(i,map);
    fConnect.RowCopy(i,conn);
    conn -= 1; // so that array indices start from 0
    wx.Collect(conn, fWx);
    for (int j = 0; j < fParticlesInCell.MinorDim(i); j++)
      {
	displacements.RowAlias(atoms[j],disp);
	temp.CopyPart(0, map, NumSD()*j, NumSD());
	parent.EvaluateShapeFunctions(temp,shape);
	nwx = dArrayT::Dot(shape,wx);
	int which = atoms[j];
	double fsx = disp[0] - nwx;
	fFineScaleU(which,0) = fsx;
	if (NumSD() == 2)
	  {
	    wy.Collect(conn, fWy);
	    nwy = dArrayT::Dot(shape,wy);
	    double fsy = disp[1] - nwy;
	    fFineScaleU(which,1) = fsy; 
	  }
      }
  }
  cout << "MD = \n" << displacements << endl;
  cout << "fine scale = \n" << fFineScaleU << endl; 
}
