/* $Id: RodT.cpp,v 1.8 2002-06-29 16:16:05 paklein Exp $ */
/* created: paklein (10/22/1996) */
#include "RodT.h"

#include <math.h>

#include "fstreamT.h"
#include "eControllerT.h"
#include "OutputSetT.h"
#include "dArray2DT.h"

/* material types */
#include "LinearSpringT.h"
#include "LJSpringT.h"

/* constructors */
RodT::RodT(const ElementSupportT& support, const FieldT& field):
	ElementBaseT(support, field),
	fCurrMaterial(NULL)
{
	/* set matrix format */
	fLHS.SetFormat(ElementMatrixT::kSymmetricUpper);
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
}

/* form of tangent matrix */
GlobalT::SystemTypeT RodT::TangentType(void) const
{
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
	StringT set_ID;
	ArrayT<StringT> e_labels;
	set_ID.Append(ElementSupport().ElementGroupNumber(this) + 1);
	OutputSetT output_set(set_ID, GeometryT::kLine, block_ID, fConnectivities, 
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
			fLHS *= constK*(dU/l - ddU)/(l*l);
		
			/* 2nd term */
			fLHS.AddScaled(-constK*dU/l, fOneOne);
		} 
		else 
			fLHS = 0.0;
	
		/* mass contribution */
		if (formM) fLHS.PlusIdentity(constK*mass);
	
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
		f0.SetToScaled(-constKd*dU/l, fBond);
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
		if (LTfnum > 0)
			fMaterialsList[matnum]->SetThermalSchedule(ElementSupport().Schedule(LTfnum));	
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
