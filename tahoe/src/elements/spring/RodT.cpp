/* $Id: RodT.cpp,v 1.23 2002-09-12 17:46:13 paklein Exp $ */
/* created: paklein (10/22/1996) */
#include "RodT.h"

#include <math.h>
#include <iostream.h>
#include "fstreamT.h"
#include "eControllerT.h"
#include "OutputSetT.h"
#include "dArray2DT.h"

/* material types */
#include "LinearSpringT.h"
#include "LJSpringT.h"

/* Element type parameters */

using namespace Tahoe;

const int RodT::kRodTndof = 2; /* number of degrees of freedom per node */
const int RodT::kRodTnsd = 2; /* number of spatial dimensions */

/* constructors */
RodT::RodT(const ElementSupportT& support, const FieldT& field):
	ElementBaseT(support, field),
	fCurrMaterial(NULL),
	fInstKE(0.0),
	fInstPE(0.0),
	fInstTotalE(0.0),
	fInstTemp(0.0),
	fInstPressure(0.0),
	fAvgKE(0.0),
	fAvgPE(0.0),
	fAvgTotalE(0.0),
	fAvgTemp(0.0),
	fAvgPressure(0.0),
	fSumKE(0.0),
	fSumPE(0.0),
	fSumTotalE(0.0),
	fSumTemp(0.0),
	fSumPressure(0.0),
	fStepNumber(support.StepNumber()),
	fHardyStress(NumSD()),
	fHardyHeatFlux(NumSD()),
	fLocVel(LocalArrayT::kVel)
{
	/* set matrix format */
	fLHS.SetFormat(ElementMatrixT::kSymmetricUpper);
	fKb = 1.38054;
}

/* initialization */
void RodT::Initialize(void)
{
	/* inherited */
	ElementBaseT::Initialize();
	
	/* constant matrix needed to calculate stiffness */
	fOneOne.Dimension(fLHS);
	dMatrixT one(NumDOF());
	one.Identity();
	fOneOne.SetBlock(0, 0, one);
	fOneOne.SetBlock(NumDOF(), NumDOF(), one);
	one *= -1;
	fOneOne.SetBlock(0, NumDOF(), one);
	fOneOne.SetBlock(NumDOF(), 0, one);
	
	/* bond vector */
	fBond.Dimension(NumSD());
	fBond0.Dimension(NumSD());

	/* echo material properties */
	ReadMaterialData(ElementSupport().Input());	
	WriteMaterialData(ElementSupport().Output());

	/* get form of tangent */
	GlobalT::SystemTypeT type = TangentType();
	
	/* set form of element stiffness matrix */
	if (type == GlobalT::kSymmetric)
		fLHS.SetFormat(ElementMatrixT::kSymmetricUpper);
	else if (type == GlobalT::kNonSymmetric)
		fLHS.SetFormat(ElementMatrixT::kNonSymmetric);
	else if (type == GlobalT::kDiagonal)
		fLHS.SetFormat(ElementMatrixT::kDiagonal);

	/* initialize and allocate velocity array IF dynamic (MD) calculation*/
	const FieldT& field = Field();
	if (field.Order() > 0) {
	  fLocVel.Allocate(NumElementNodes(), NumDOF());
	  Field().RegisterLocal(fLocVel);
	}
}

/* form of tangent matrix */
GlobalT::SystemTypeT RodT::TangentType(void) const
{
	/* special case */
	if (fController->Order() > 0 &&
	    fController->ImplicitExplicit() ==  eControllerT::kExplicit)
		return GlobalT::kDiagonal;
	else
		return GlobalT::kSymmetric;
}

/* NOT implemented. Returns an zero force vector */
void RodT::AddNodalForce(const FieldT& field, int node, dArrayT& force)
{
#pragma unused(field)
#pragma unused(node)
#pragma unused(force)
}

/* returns the energy as defined by the derived class types */
double RodT::InternalEnergy(void)
{
	/* coordinates arrays */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
	const dArray2DT& curr_coords = ElementSupport().CurrentCoordinates();

	double energy = 0.0;
	Top();
	while (NextElement())
	{
		/* node numbers */
		const iArrayT& nodes = CurrentElement().NodesX();
		int n0 = nodes[0];
		int n1 = nodes[1];
	
		/* reference bond */
		fBond0.DiffOf(init_coords(n1), init_coords(n0));

		/* current bond */
		fBond.DiffOf(curr_coords(n1), curr_coords(n0));
		
		/* form element stiffness */
		energy += fCurrMaterial->Potential(fBond.Magnitude(), fBond0.Magnitude());
	}
	return energy;
}

/* writing output */
void RodT::RegisterOutput(void)
{
	/* block ID's */
	ArrayT<StringT> block_ID(fBlockData.Length());
	for (int i = 0; i < block_ID.Length(); i++)
		block_ID[i] = fBlockData[i].ID();

	/* set output specifier */
	ArrayT<StringT> e_labels;
	OutputSetT output_set(GeometryT::kLine, block_ID, fConnectivities, 
		Field().Labels(), e_labels, ChangingGeometry());
		
	/* register and get output ID */
	fOutputID = ElementSupport().RegisterOutput(output_set);
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
	dArray2DT disp(nodes_used.Length(), NumDOF());
	
	/* collect group displacements */
	disp.RowCollect(nodes_used, Field()[0]);

	/* send */
	dArray2DT e_values;
	ElementSupport().WriteOutput(fOutputID, disp, e_values);
}

/* compute specified output parameter and send for smoothing */
void RodT::SendOutput(int kincode)
{
#pragma unused(kincode)
	//TEMP: for now, do nothing
}

/* initialize/finalize step */
void RodT::InitStep(void)
{
  /* inherited */
  ElementBaseT::InitStep();
  /* set material variables */
  //fMaterialList->InitStep();
}

void RodT::CloseStep(void)
{
  /* inherited */
  ElementBaseT::CloseStep();
  /* set material variables */
  //fMaterialList->CloseStep(); 
  Top();
  const FieldT& field = Field();
  while (NextElement()) {
    ComputeHardyStress();
    if (field.Order() > 0)
      {
     	/* get velocities */
	SetLocalU(fLocVel);
	
	/* compute MD quantities of interest */
	ComputeInstPE();
	ComputeInstKE();
	ComputeInstTotalE();
	ComputeInstTemperature();
	//ComputeInstPressure();
      }
  }
  ComputeAvgPE();
  ComputeAvgKE();
  ComputeAvgTotalE();
  ComputeAvgTemperature();
  //ComputeAvgPressure()
  PrintMDToFile();
}


/***********************************************************************
* Protected
***********************************************************************/

/* construct the element stiffness matrix */
void RodT::LHSDriver(void)
{
	/* time integration dependent */
	double constK = 0.0;
	double constM = 0.0;
	int formK = fController->FormK(constK);
	int formM = fController->FormM(constM);

	/* coordinates arrays */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
	const dArray2DT& curr_coords = ElementSupport().CurrentCoordinates();
	
	/* use RHS as temp space */
	dArrayT v0(NumDOF(), fRHS.Pointer());
	dArrayT v1(NumDOF(), fRHS.Pointer() + NumDOF());

	Top();
	while (NextElement())
	{
		/* particle mass */
		double mass = fCurrMaterial->Mass();

		/* node numbers */
		const iArrayT& nodes = CurrentElement().NodesX();
		int n0 = nodes[0];
		int n1 = nodes[1];

		/* form element stiffness */
		if (formK) {		

			/* reference bond */
			fBond0.DiffOf(init_coords(n1), init_coords(n0));
			double l0 = fBond0.Magnitude();

			/* current bond */
			v1.DiffOf(curr_coords(n1), curr_coords(n0));
			double l = v1.Magnitude();
			v0.SetToScaled(-1.0, v1);
			
			/* bond force and stiffness */
			double dU = fCurrMaterial->DPotential(l, l0);
			double ddU = fCurrMaterial->DDPotential(l, l0);

			/* 1st term */
			fLHS.Outer(fRHS, fRHS);
			fLHS *= constK*(ddU - dU/l)/(l*l);
		
			/* 2nd term */
			fLHS.AddScaled(constK*dU/l, fOneOne);
		} 
		else 
			fLHS = 0.0;
	
		/* mass contribution */
		if (formM) fLHS.PlusIdentity(constM*mass);
	
		/* add to global equations */
		AssembleLHS();
	}
}

/* construct the element force vectors */
void RodT::RHSDriver(void)
{
	/* set components and weights */
	double constMa = 0.0;
	double constKd = 0.0;
	
	/* components dicated by the algorithm */
	int formMa = fController->FormMa(constMa);
	int formKd = fController->FormKd(constKd);
	
//TEMP - inertia term in residual
if (formMa) {
	cout << "\n ParticleT::RHSDriver: M*a term not implemented" << endl;
	throw eGeneralFail;
}

	/* coordinates arrays */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
	const dArray2DT& curr_coords = ElementSupport().CurrentCoordinates();
	
	/* point forces */
	dArrayT f0(NumDOF(), fRHS.Pointer());
	dArrayT f1(NumDOF(), fRHS.Pointer() + NumDOF());

	Top();
	while (NextElement())
	{
		/* node numbers */
		const iArrayT& nodes = CurrentElement().NodesX();
		int n0 = nodes[0];
		int n1 = nodes[1];
	
		/* reference bond */
		fBond0.DiffOf(init_coords(n1), init_coords(n0));
		double l0 = fBond0.Magnitude();

		/* current bond */
		fBond.DiffOf(curr_coords(n1), curr_coords(n0));
		double l = fBond.Magnitude();
		
		/* bond force magnitude */
		double dU = fCurrMaterial->DPotential(l, l0);

		/* particle forces (extra -1 since moved to the RHS) */
		f0.SetToScaled(constKd*dU/l, fBond);
		f1.SetToScaled(-1.0, f0);

		/* add to global equations */
		AssembleRHS();
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
		int LTfnum = fMaterialsList[matnum]->ThermalScheduleNumber();
		if (LTfnum > -1)
			fMaterialsList[matnum]->SetThermalSchedule(ElementSupport().Schedule(LTfnum));	
	}
}

void RodT::WriteMaterialData(ostream& out) const
{
	out << "\n Material Set Data:\n";
	for (int i = 0; i < fMaterialsList.Length(); i++)
	{
		out << "\n Material number . . . . . . . . . . . . . . . . = " << i+1 << '\n';
		fMaterialsList[i]->PrintName(out);
		fMaterialsList[i]->Print(out);
	}
}

/* read connectivity and determine the nodes used */
void RodT::EchoConnectivityData(ifstreamT& in, ostream& out)
{
	/* inherited */
	ElementBaseT::EchoConnectivityData(in, out);
	
	/* determine the nodes used strictly based on those in the connectivities */
	NodesUsed(fGroupNodes);
}

/***********************************************************************
* Private
***********************************************************************/

/* Below are functions implementing Hardy ideas deriving continuum 
 * measures from MD notions */

void RodT::ComputeHardyStress(void)
{
  double constKd = 0.0;
	
  /* components dicated by the algorithm */
  int formKd = fController->FormKd(constKd);
  
  /* coordinates arrays */
  const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
  const dArray2DT& curr_coords = ElementSupport().CurrentCoordinates();
  
  /* point forces */
  dArrayT f0(NumDOF(), fRHS.Pointer());
  dArrayT f1(NumDOF(), fRHS.Pointer() + NumDOF());
  
  /* node numbers */
  const iArrayT& nodes = CurrentElement().NodesX();
  int n0 = nodes[0];
  int n1 = nodes[1];
	
  /* reference bond */
  fBond0.DiffOf(init_coords(n1), init_coords(n0));
  double l0 = fBond0.Magnitude();

  /* current bond */
  fBond.DiffOf(curr_coords(n1), curr_coords(n0));
  double l = fBond.Magnitude();
		
  /* bond force magnitude */
  double dU = fCurrMaterial->DPotential(l, l0);
	
  /* particle forces */
  f0.SetToScaled(constKd*dU/l, fBond);

  /* Potential part of Hardy stress */
  fHardyStress.Outer(fBond,f0);
  fHardyStress *= -.5;

  /* Kinetic part of Hardy stress */
  const FieldT& field = Field();
  if (field.Order() > 0) {
   
    dArrayT vel;
    dMatrixT kinstress(NumSD());
    const dArray2DT& velocities = field[1];
    for (int i = 0; i < fGroupNodes.Length(); i++)
      {
	velocities.RowAlias(fGroupNodes[i], vel);
	kinstress.Outer(vel,vel);
	kinstress *= .5;
	kinstress *= fCurrMaterial->Mass();
	fHardyStress += kinstress;
      }
  }
}

void RodT::ComputeHardyHeatFlux(void)
{





}


/* Below are MD related computational functions */

void RodT::ComputeInstKE(void)
{
	/* computes the instantaneous kinetic energy of the system of atoms */
	double& ke = fInstKE;
	double& total = fSumKE;
  
	ke = 0.0;
	const FieldT& field = Field();
	if (field.Order() > 0) {
   
		dArrayT vel;
		const dArray2DT& velocities = field[1];
		for (int i = 0; i < fGroupNodes.Length(); i++)
  		{
  			velocities.RowAlias(fGroupNodes[i], vel);
  			ke += dArrayT::Dot(vel,vel);
		}
		ke *= fCurrMaterial->Mass()/2.0;
	}
	total += ke;
}

void RodT::ComputeAvgKE(void)
{
  double& tempavg = fAvgKE;
  tempavg = fSumKE / fStepNumber;
}

void RodT::ComputeInstPE(void)
{
  /* computes the potential energy of the system of atoms */
  
  /* coordinates arrays */
  const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
  const dArray2DT& curr_coords = ElementSupport().CurrentCoordinates();
  double& pe = fInstPE;
  double& total = fSumPE;
  pe = 0.0;

  /* node numbers */
  const iArrayT& nodes = CurrentElement().NodesX();
  int n0 = nodes[0];
  int n1 = nodes[1];
  
  /* reference bond */
  fBond0.DiffOf(init_coords(n1), init_coords(n0));
  
  /* current bond */
  fBond.DiffOf(curr_coords(n1), curr_coords(n0));

  pe += fCurrMaterial->Potential(fBond.Magnitude(), fBond0.Magnitude());
  total += pe;
}

void RodT::ComputeAvgPE(void)
{
  double& tempavg = fAvgPE;
  tempavg = fSumPE / fStepNumber;
}

void RodT::ComputeInstTotalE(void)
{
  /* computes instantaneous total energy = kinetic energy + potential energy of the system */
  double& totale = fInstTotalE;
  double& total = fSumTotalE;
  totale = 0.0;
  totale = fInstKE + fInstPE;
  total += totale;
}

void RodT::ComputeAvgTotalE(void)
{
  double& tempavg = fAvgTotalE;
  tempavg = fSumTotalE / fStepNumber;

}

void RodT::ComputeInstTemperature(void)
{
  /* computes instantaneous temperature of the atomic system */
  double& temp = fInstTemp;
  temp = 0.0;
  temp = 2 * fInstKE / (fKb * NumSD() * fGroupNodes.Length());
}

void RodT::ComputeAvgTemperature(void)
{
  double& temp = fAvgTemp;
  temp = 2 * fAvgKE / (fKb * NumSD() * fGroupNodes.Length());
}

void RodT::ComputeInstPressure(void)
{
  /* computes the instantaneous pressure of the atomic system */

}

void RodT::ComputeAvgPressure(void)
{
  /* computes the average pressure of the atomic system */

}

int RodT::PrintMDToFile(void)
{
	/* print MD quantities (temperature/energy/pressure) to an output file */
	ofstreamT out;
	int d_width = OutputWidth(out, &fAvgTotalE); 
	if (ElementSupport().StepNumber() == 1)
  	{
  		out.open("MD.out");
		if (!out.is_open()) {
			cout << "Cannot open MD.out file.\n";
			return 1;
		}

  		out << setw(d_width) << "Timestep"
		    << setw(d_width) << "InstKE"  
  		    << setw(d_width) << "InstPE" 
  		    << setw(d_width) << "InstTemp" 
  		    << setw(d_width) << "InstTotalE" 
  		    << setw(d_width) << "AvgKE" 
  		    << setw(d_width) << "AvgPE" 
  		    << setw(d_width) << "AvgTemp" 
  		    << setw(d_width) << "AvgTotalE" << endl;
	}
	else /* appending to existing file */
	{
  		out.open_append("MD.out");
		if (!out.is_open()) {
			cout << "Cannot open MD.out file.\n";
			return 1;
		}
	
		out << setw(d_width) << fStepNumber
		    << setw(d_width) << fInstKE 
		    << setw(d_width) << fInstPE 
		    << setw(d_width) << fInstTemp 
		    << setw(d_width) << fInstTotalE 
		    << setw(d_width) << fAvgKE 
		    << setw(d_width) << fAvgPE 
		    << setw(d_width) << fAvgTemp 
		    << setw(d_width) << fAvgTotalE 
		    << setw(d_width) << endl;
	}
	out.close();
	return 0;
}
