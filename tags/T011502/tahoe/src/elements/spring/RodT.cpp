/* $Id: RodT.cpp,v 1.4 2002-01-06 06:58:37 cbhovey Exp $ */
/* created: paklein (10/22/1996) */

#include "RodT.h"

#include <math.h>

#include "NodeManagerT.h"
#include "fstreamT.h"
#include "FEManagerT.h"
#include "eControllerT.h"
#include "OutputSetT.h"
#include "dArray2DT.h"

/* material types */
#include "LinearSpringT.h"
#include "LJSpringT.h"

/* Element type parameters */
const int RodT::kRodTndof = 2; /* number of degrees of freedom per node */
const int RodT::kRodTnsd = 2; /* number of spatial dimensions */

/* constructors */
RodT::RodT(FEManagerT& fe_manager):
	ElementBaseT(fe_manager),
	fCurrMaterial(NULL),
	fLocInitCoords(LocalArrayT::kInitCoords),
	fLocDisp(LocalArrayT::kDisp)
{
	//TEMP - only 2D for now. need to rewrite force and stiffness calculations
	if (NumSD() != 2) {
		cout << "\n RodT::RodT: only 2D" << endl;
		throw eGeneralFail;
	}

	/* dimensions */
	fNumElemNodes = 2;
	fNumElemEqnos = fNumElemNodes*fNumDOF;

	/* set matrix format */
	fLHS.SetFormat(ElementMatrixT::kSymmetricUpper);
}

/* initialization */
void RodT::Initialize(void)
{
	/* inherited */
	ElementBaseT::Initialize();

	/* set local arrays */
	fLocInitCoords.Allocate(fNumElemNodes, fNumSD);
	fLocDisp.Allocate(fNumElemNodes, fNumDOF);
	fFEManager.RegisterLocal(fLocInitCoords);
	fFEManager.RegisterLocal(fLocDisp);

	/* echo material properties */
	ReadMaterialData(fFEManager.Input());	
	WriteMaterialData(fFEManager.Output());
}

/* form of tangent matrix */
GlobalT::SystemTypeT RodT::TangentType(void) const
{
	return GlobalT::kSymmetric;
}

/* NOT implemented. Returns an zero force vector */
void RodT::AddNodalForce(int node, dArrayT& force)
{
#pragma unused(node)
#pragma unused(force)
}

/* returns the energy as defined by the derived class types */
double RodT::InternalEnergy(void)
{
	double energy = 0.0;

	Top();
	while ( NextElement() )
	{
		/* local arrays */
		SetLocalX(fLocInitCoords);
		SetLocalU(fLocDisp);
		
		/* form element stiffness */
		energy += ElementEnergy();
	}

	return(energy);
}

/* writing output */
void RodT::RegisterOutput(void)
{
	/* block ID's */
	iArrayT block_ID(fBlockData.MajorDim());
	fBlockData.ColumnCopy(kID, block_ID);

	/* variable labels */
	ArrayT<StringT> n_labels(2);
	n_labels[0] = "D_X";
	n_labels[1] = "D_Y";
	ArrayT<StringT> e_labels;

	/* set output specifier */
	int ID = fFEManager.ElementGroupNumber(this) + 1;
	OutputSetT output_set(ID, GeometryT::kLine, block_ID, fConnectivities, n_labels, e_labels, 
		ChangingGeometry());
		
	/* register and get output ID */
	fOutputID = fFEManager.RegisterOutput(output_set);
}

void RodT::WriteOutput(IOBaseT::OutputModeT mode)
{
//TEMP - not handling general output modes yet
	if (mode != IOBaseT::kAtInc)
	{
		cout << "\n ContinuumElementT::WriteOutput: only handling \"at increment\"\n"
		     <<   "     print mode. SKIPPING." << endl;
		return;
	}

	/* get list of nodes used by the group */
	iArrayT nodes_used;
	NodesUsed(nodes_used);

	/* temp space for group displacements */
	dArray2DT disp(nodes_used.Length(), 2);
	
	/* collect group displacements */
	disp.RowCollect(nodes_used, fNodes->Displacements());

	/* send */
	dArray2DT e_values;
	fFEManager.WriteOutput(fOutputID, disp, e_values);
}

/* compute specified output parameter and send for smoothing */
void RodT::SendOutput(int kincode)
{
#pragma unused(kincode)
	//TEMP: for now, do nothing
}

/***********************************************************************
* Protected
***********************************************************************/

/* construct the element stiffness matrix */
void RodT::LHSDriver(void)
{
	/* time integration dependent */
	double constK;
	if (!fController->FormK(constK)) return;
	
	Top();
	while ( NextElement() )
	{
		/* initialize */
		fLHS = 0.0;
		
		/* local arrays */
		SetLocalX(fLocInitCoords);
		SetLocalU(fLocDisp);
		
		/* form element stiffness */
		ElementStiffness(constK);
	
		/* add to global equations */
		AssembleLHS();
	}
}

/* construct the element force vectors */
void RodT::RHSDriver(void)
{
	/* time integration dependent */
	double constKd;
	if (!fController->FormK(constKd)) return;

	Top();
	while ( NextElement() )
	{
		/* local displacement */
		SetLocalU(fLocDisp);
	
		if (fLocDisp.AbsMax() > 0.0 || fCurrMaterial->HasInternalStrain())
		{
			/* initialize */
			fRHS = 0.0;
	
			/* local coordinates */
			SetLocalX(fLocInitCoords);

			/* form element force */
			ElementForce(-1.0);
	
			/* add to global equations */
			AssembleRHS();
		}
	}
}

/* load next element */
bool RodT::NextElement(void)
{
	bool result = ElementBaseT::NextElement();
	
	/* initialize element calculation */
	if (result)
		fCurrMaterial = fMaterialsList[CurrentElement().MaterialNumber()];
	
	return result;
}
	
/* element data */
void RodT::ReadMaterialData(ifstreamT& in)
{
	/* allocate space */
	int	nummaterials;
	in >> nummaterials;
	fMaterialsList.Allocate(nummaterials);

	/* read data */
	for (int i = 0; i < nummaterials; i++)
	{
		int matnum, matcode;
		in >> matnum;
		in >> matcode;	
		
		/* add to the list of materials */
		switch (matcode)
		{
			case kQuad:			
				fMaterialsList[--matnum] = new LinearSpringT(in);
				break;
				
			case kLJ612:
				fMaterialsList[--matnum] = new LJSpringT(in);
				break;

			default:
			
				cout << "\n RodT::ReadMaterialData: unknown material type\n" << endl;
				throw eBadInputValue;
		}

		/* check */
		if (!fMaterialsList[matnum]) throw(eOutOfMemory);
	
		/* set thermal LTf pointer */
		int LTfnum = fMaterialsList[matnum]->ThermalLTfNumber();
		if (LTfnum > 0)
			fMaterialsList[matnum]->SetThermalLTfPtr(GetLTfPtr(LTfnum));	
	}
}

void RodT::WriteMaterialData(ostream& out) const
{
	out << "\n Material Set Data:\n";
	for (int i = 0; i < fMaterialsList.Length(); i++)
	{
		out << "\n Material number . . . . . . . . . . . . . . . . = " << i+1 << '\n';
		fMaterialsList[i]->Print(out);
	}
}

/* element calculations */
double RodT::ElementEnergy(void)
{
	double z1, z2, z3, z4, z5, z6, z7, z8;

	z1 = fLocDisp(0,0);
	z2 = fLocDisp(0,1);
	z3 = fLocDisp(1,0);
	z4 = fLocDisp(1,1);
	z5 = fLocInitCoords(0,0);
	z6 = fLocInitCoords(0,1);
	z7 = fLocInitCoords(1,0);
	z8 = fLocInitCoords(1,1);
	z3 = -z3;
	z4 = -z4;
	z7 = -z7;
	z8 = -z8;
	z5 = z5 + z7;
	z6 = z6 + z8;
	z2 = z2 + z4 + z6;
	z1 = z1 + z3 + z5;
	z3 = z5*z5;
	z4 = z6*z6;
	z2 = z2*z2;
	z1 = z1*z1;
	z3 = z3 + z4;
	z1 = z1 + z2;
	z2 = sqrt(z3);
	z1 = sqrt(z1);
	
	return( fCurrMaterial->Potential(z1,z2) );
}

void RodT::ElementForce(double constKd)
{
	double z1, z2, z3, z4, z5, z6, z7, z8;

	z1 = fLocDisp(0,0);
	z2 = fLocDisp(0,1);
	z3 = fLocDisp(1,0);
	z4 = fLocDisp(1,1);
	z5 = fLocInitCoords(0,0);
	z6 = fLocInitCoords(0,1);
	z7 = fLocInitCoords(1,0);
	z8 = fLocInitCoords(1,1);
	z3 = -z3;
	z4 = -z4;
	z7 = -z7;
	z8 = -z8;
	z5 = z5 + z7;
	z6 = z6 + z8;
	z2 = z2 + z4 + z6;
	z1 = z1 + z3 + z5;
	z3 = z5*z5;
	z4 = z6*z6;
	z5 = z2*z2;
	z6 = z1*z1;
	z3 = z3 + z4;
	z4 = z5 + z6;
	z3 = sqrt(z3);
	z4 = sqrt(z4);
	z5 = 1.0/z4;
	z3 = fCurrMaterial->DPotential(z4,z3);
	z4 = -z3*z5;
	z3 = z3*z5;
	z5 = z2*z4;
	z4 = z1*z4;
	z2 = z2*z3;
	z1 = z1*z3;

	// z1 = List(z4,z5,z1,z2);

	/* "-k*d" */
	fRHS[0] = -constKd*z4;
	fRHS[1] = -constKd*z5;
	fRHS[2] = -constKd*z1;
	fRHS[3] = -constKd*z2;
}

void RodT::ElementStiffness(double constK)
{
	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11, z12;
	double z13, z14, z15, z16;

	z1 = fLocDisp(0,0);
	z2 = fLocDisp(0,1);
	z3 = fLocDisp(1,0);
	z4 = fLocDisp(1,1);
	z5 = fLocInitCoords(0,0);
	z6 = fLocInitCoords(0,1);
	z7 = fLocInitCoords(1,0);
	z8 = fLocInitCoords(1,1);
	z3 = -z3;
	z4 = -z4;
	z7 = -z7;
	z8 = -z8;
	z5 = z5 + z7;
	z6 = z6 + z8;
	z2 = z2 + z4 + z6;
	z1 = z1 + z3 + z5;
	z3 = z5*z5;
	z4 = z6*z6;
	z5 = z2*z2;
	z6 = z1*z1;
	z3 = z3 + z4;
	z4 = z5 + z6;
	z3 = sqrt(z3);
	z7 = pow(z4,-1.5);
	z8 = 1.0/z4;
	z4 = pow(z4,0.5);
	z9 = 1.0/z4;
	z10 = fCurrMaterial->DPotential(z4,z3);
	z3 =  fCurrMaterial->DDPotential(z4,z3);
	z4 = -z10*z7;
	z7 = z10*z7;
	z11 = -z10*z9;
	z9 = z10*z9;
	z10 = -z1*z2*z3*z8;
	z12 = z1*z2*z3*z8;
	z13 = -z3*z5*z8;
	z14 = z3*z5*z8;
	z15 = -z3*z6*z8;
	z3 = z3*z6*z8;
	z8 = z1*z2*z4;
	z16 = z4*z5;
	z4 = z4*z6;
	z1 = z1*z2*z7;
	z2 = z5*z7;
	z5 = z6*z7;
	z6 = z12 + z8;
	z7 = z14 + z16 + z9;
	z3 = z3 + z4 + z9;
	z1 = z1 + z10;
	z2 = z11 + z13 + z2;
	z4 = z11 + z15 + z5;

	/*
		z3   z6   z4   z1
		     z7   z1   z2
		          z3   z6
		               z7
	*/

	fLHS(0,0) = z3*constK;
	fLHS(0,1) = z6*constK;
	fLHS(0,2) = z4*constK;
	fLHS(0,3) = z1*constK;
	fLHS(1,1) = z7*constK;
	fLHS(1,2) = z1*constK;
	fLHS(1,3) = z2*constK;
	fLHS(2,2) = z3*constK;
	fLHS(2,3) = z6*constK;
	fLHS(3,3) = z7*constK;
}