/* $Id: DiffusionElementT.cpp,v 1.2 2001-02-20 00:42:13 paklein Exp $ */
/* created: paklein (10/02/1999)                                          */

#include "DiffusionElementT.h"

#include <iostream.h>
#include <iomanip.h>
#include <math.h>

#include "Constants.h"

#include "fstreamT.h"
#include "FEManagerT.h"
#include "NodeManagerT.h"
#include "DiffusionMaterialT.h"
#include "ElementCardT.h"
#include "ShapeFunctionT.h"
#include "eControllerT.h"
#include "MaterialListT.h"
#include "iAutoArrayT.h"
//#include "OutputSetT.h"

/* materials lists */
#include "DiffusionMatListT.h"

/* initialize static data */
const int DiffusionElementT::NumOutputCodes = 3;

/* parameters */
const int kDiffusionNDOF = 1;

/* constructor */
DiffusionElementT::DiffusionElementT(FEManagerT& fe_manager):
	ContinuumElementT(fe_manager),
	fLocVel(LocalArrayT::kVel),
	fD(fNumSD),
	fq(fNumSD)
{
	/* check base class initializations */
	if (fNumDOF != kDiffusionNDOF) throw eGeneralFail;
}

/* data initialization */
void DiffusionElementT::Initialize(void)
{
	/* inherited */
	ContinuumElementT::Initialize();

	/* allocate */
	fB.Allocate(fNumSD, fNumElemNodes);

	/* setup for material output */
	if (fNodalOutputCodes[iMaterialData])
	{
		/* check compatibility of output */
		if (!CheckMaterialOutput())
		{
			cout << "\n DiffusionElementT::Initialize: error with material output" << endl;
			throw eBadInputValue;
		}
		/* no material output variables */
	 	else if ((*fMaterialList)[0]->NumOutputVariables() == 0)
		{
			cout << "\n DiffusionElementT::ReadMaterialData: there are no material outputs"
			     << endl;
			fNodalOutputCodes[iMaterialData] = 0;
		}
	}
}

/* set the controller */
void DiffusionElementT::SetController(eControllerT* controller)
{
	/* inherited */
	ContinuumElementT::SetController(controller);

	//should check the controller for compatibility
}

/* compute nodal force */
void DiffusionElementT::AddNodalForce(int node, dArrayT& force)
{
	//not implemented
#pragma unused(node)
#pragma unused(force)
}
	
/* returns the energy as defined by the derived class types */
double DiffusionElementT::InternalEnergy(void)
{
	double energy = 0.0;

	Top();
	while ( NextElement() )
	{
		/* shape function derivatives, jacobians, local coords */
		SetGlobalShape();
		
		/* get displacements */
		SetLocalU(fLocDisp);
		
		/* integration */
		const double* Det    = fShapes->IPDets();
		const double* Weight = fShapes->IPWeights();

		/* material properties */
		double heat_capacity = fCurrMaterial->Capacity();

		fShapes->TopIP();
		while ( fShapes->NextIP() )
		{
			/* ip value */
			fShapes->InterpolateU(fLocDisp, fDOFvec);
		
			/* accumulate */
			energy += heat_capacity*(*Det++)*(*Weight++)*fDOFvec[0];
		}
	}
	return energy;
}

void DiffusionElementT::SendOutput(int kincode)
{
	/* output flags */
	iArrayT flags(fNodalOutputCodes.Length());

	/* set flags to get desired output */
	flags = IOBaseT::kAtNever;
	switch (kincode)
	{
		case iNodalDisp:
		    flags[iNodalDisp] = fNumDOF;
			break;
		default:
			cout << "\n DiffusionElementT::SendKinematic: invalid output code: ";
			cout << kincode << endl;
	}

	/* number of output values */
	iArrayT n_counts;
	SetNodalOutputCodes(IOBaseT::kAtInc, flags, n_counts);

	/* reset averaging workspace */
	fNodes->ResetAverage(n_counts.Sum());

	/* generate nodal values */
	iArrayT e_counts;
	dArray2DT e_values, n_values;
	ComputeOutput(n_counts, n_values, e_counts, e_values);
}

/***********************************************************************
* Protected
***********************************************************************/

/* print element group data */
void DiffusionElementT::PrintControlData(ostream& out) const
{
	/* inherited */
	ContinuumElementT::PrintControlData(out);

	// anything else
}

void DiffusionElementT::EchoOutputCodes(ifstreamT& in, ostream& out)
{
	/* allocate */
	fNodalOutputCodes.Allocate(NumOutputCodes);

	/* read in at a time to allow comments */
	for (int i = 0; i < fNodalOutputCodes.Length(); i++)
	{
		in >> fNodalOutputCodes[i];
		
		/* convert all to "at print increment" */
		if (fNodalOutputCodes[i] != IOBaseT::kAtNever)
			fNodalOutputCodes[i] = IOBaseT::kAtInc;
	}		

	/* checks */
	if (fNodalOutputCodes.Min() < IOBaseT::kAtFail ||
	    fNodalOutputCodes.Max() > IOBaseT::kAtInc) throw eBadInputValue;

	/* default behavior with output formats */
//	fNodalOutputCodes[iNodalCoord] = IOBaseT::kAtNever;
//	fNodalOutputCodes[iNodalDisp ] = IOBaseT::kAtNever;
// what to do about default behavior

	/* control parameters */
	out << " Number of nodal output codes. . . . . . . . . . = " << NumOutputCodes << '\n';
	out << "    [" << fNodalOutputCodes[iNodalCoord   ] << "]: initial nodal coordinates\n";
	out << "    [" << fNodalOutputCodes[iNodalDisp    ] << "]: nodal displacements\n";
	out << "    [" << fNodalOutputCodes[iMaterialData ] << "]: nodal material output parameters\n";
}

/* initialize local arrays */
void DiffusionElementT::SetLocalArrays(void)
{
	/* inherited */
	ContinuumElementT::SetLocalArrays();

	/* dimension */
	fLocVel.Allocate(fNumElemNodes, fNumDOF);
	
	/* set source */
	fFEManager.RegisterLocal(fLocVel);
}

/* construct output labels array */
void DiffusionElementT::SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* initialize */
	counts.Allocate(flags.Length());
	counts = 0;

	if (flags[iNodalCoord] == mode)
		counts[iNodalCoord] = fNumSD;
	if (flags[iNodalDisp] == mode)
		counts[iNodalDisp] = fNumDOF;
	if (flags[iMaterialData] == mode)
		counts[iMaterialData ] = (*fMaterialList)[0]->NumOutputVariables();
}

void DiffusionElementT::SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
#pragma unused(mode)
#pragma unused(flags)
	if (counts.Sum() != 0)
	{
		cout << "\n DiffusionElementT::SetElementOutputCodes: not yet supported" << endl;
		throw eBadInputValue;
	}
}

/* set the correct shape functions */
void DiffusionElementT::SetShape(void)
{
	fShapes = new ShapeFunctionT(fGeometryCode, fNumIP,
		fLocInitCoords, ShapeFunctionT::kStandardB);
	if (!fShapes ) throw eOutOfMemory;
	fShapes->Initialize();
}

/* construct the effective mass matrix */
void DiffusionElementT::LHSDriver(void)
{
	/* inherited */
	ContinuumElementT::LHSDriver();

	/* set components and weights */
	double constC = 0.0;
	double constK = 0.0;
	
	int formC = fController->FormC(constC);
	int formK = fController->FormK(constK);

	/* loop over elements */
	Top();
	while (NextElement())
	{
		/* initialize */
		fLHS = 0.0;
		
		/* set shape function derivatives */
		SetGlobalShape();
			
		/* element mass */
		if (formC) FormMass(kConsistentMass, constC*(fCurrMaterial->Capacity()));

		/* element stiffness */
		if (formK) FormStiffness(constK);
	
		/* add to global equations */
		AssembleLHS();		
	}
}

void DiffusionElementT::RHSDriver(void)
{
	/* inherited */
	ContinuumElementT::RHSDriver();

	/* set components and weights */
	double constCv = 0.0;
	double constKd = 0.0;
	
	/* components dicated by the algorithm */
	int formCv = fController->FormCv(constCv);
	int formKd = fController->FormKd(constKd);

	/* body forces */
	int formBody = 0;
	if (fBodyForceLTf > -1 && fBody.Magnitude() > kSmall)
	{	
		formBody = 1;
		if (!formCv) constCv = 1.0; // correct value ??
	}

	Top();
	while (NextElement())
	{
		/* nodal temperature */
		if (formKd)
		{
			SetLocalU(fLocDisp);
			fLocDisp *= constKd;
		}
		else
			fLocDisp = 0.0;

		/* nodal temperature rate */
		fLocVel = 0.0;
		if (formBody)
		{
			AddBodyForce(fLocVel);
			fLocVel *= constCv/fCurrMaterial->SpecificHeat();
		}

		/* last check w/ effective v and d - override controller */
		int eformCv = fLocVel.AbsMax() > 0.0;
		int eformKd = fLocDisp.AbsMax() > 0.0;
		if (eformCv || eformKd)
		{
			/* initialize */
			fRHS = 0.0;
		
			/* global shape function values */
			SetGlobalShape();
			
			/* internal force contribution */	
			if (eformKd) FormKd(-1.0);

			/* inertia forces */
			if (eformCv) FormMa(kConsistentMass, -fCurrMaterial->Capacity(), fLocVel);			  		
								
			/* assemble */
			AssembleRHS();
		}
	}
}

/* current element operations */
bool DiffusionElementT::NextElement(void)
{
	/* inherited */
	bool result = ContinuumElementT::NextElement();
	
	/* get material pointer */
	if (result)
	{
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[CurrentElement().MaterialNumber()];
	
		/* cast is safe since class contructs materials list */
		fCurrMaterial = (DiffusionMaterialT*) pcont_mat;
	}
	
	return result;
}

/* form the element stiffness matrix */
void DiffusionElementT::FormStiffness(double constK)
{
	/* matrix format */
	dMatrixT::SymmetryFlagT format =
		(fLHS.Format() == ElementMatrixT::kNonSymmetric) ?
		dMatrixT::kWhole :
		dMatrixT::kUpperOnly;

	/* integration parameters */
	const double* Det    = fShapes->IPDets();
	const double* Weight = fShapes->IPWeights();
	
	/* integrate element stiffness */
	fShapes->TopIP();
	while ( fShapes->NextIP() )
	{
		double scale = constK*(*Det++)*(*Weight++);
	
		/* strain displacement matrix */
		fShapes->B_q(fB);

		/* get D matrix */
		fD.SetToScaled(scale, fCurrMaterial->k_ij());
							
		/* multiply b(transpose) * db, taking account of symmetry, */
		/* and accumulate in elstif */
		fLHS.MultQTBQ(fB, fD, format, dMatrixT::kAccumulate);	
	}
}

/* calculate the internal force contribution ("-k*d") */
void DiffusionElementT::FormKd(double constK)
{
	/* integration parameters */
	const double* Det    = fShapes->IPDets();
	const double* Weight = fShapes->IPWeights();
	
	fShapes->TopIP();
	while ( fShapes->NextIP() )
	{
		/* get strain-displacement matrix */
		fShapes->B_q(fB);

		/* compute heat flow */
		fB.MultTx(fCurrMaterial->q_i(), fNEEvec);

		/* accumulate */
		fRHS.AddScaled(constK*(*Weight++)*(*Det++), fNEEvec);
	}	
}

/* return a pointer to a new material list */
MaterialListT* DiffusionElementT::NewMaterialList(int size) const
{
	/* allocate */
	return new DiffusionMatListT(size, *this);
}

/* driver for calculating output values */
void DiffusionElementT::ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	const iArrayT& e_codes, dArray2DT& e_values)
{
	/* number of output values */
	int n_out = n_codes.Sum();
	int e_out = e_codes.Sum();

	/* nothing to output */
	if (n_out == 0 && e_out == 0) return;

//TEMP
#pragma unused(e_values)
if (e_out > 0)
{
	cout << "\n DiffusionElementT::ComputeOutput: element output not yet supported" << endl;
	throw eGeneralFail;
}

	/* reset averaging workspace */
	fNodes->ResetAverage(n_out);

	/* work arrays */
	dArray2DT nodal_space(fNumElemNodes, n_out);
	dArray2DT nodal_all(fNumElemNodes, n_out);
	dArray2DT coords, disp;
	dArray2DT nodalstress, princstress, matdat;
	dArray2DT energy, speed;

	/* ip values */
	dSymMatrixT cauchy(fNumSD);
	dArrayT ipmat(n_codes[iMaterialData]), ipenergy(1);
	dArrayT ipspeed(fNumSD), ipprincipal(fNumSD);

	/* set shallow copies */
	double* pall = nodal_space.Pointer();
	coords.Set(fNumElemNodes, n_codes[iNodalCoord], pall);
	pall += coords.Length();
	disp.Set(fNumElemNodes, n_codes[iNodalDisp], pall);
	pall += disp.Length();
	matdat.Set(fNumElemNodes, n_codes[iMaterialData], pall);

	Top();
	while (NextElement())
	{
		/* initialize */
	    nodal_space = 0.0;

		/* global shape function values */
		SetGlobalShape();
		SetLocalU(fLocDisp);
		
		/* coordinates and displacements all at once */
		if (n_codes[iNodalCoord]) fLocInitCoords.ReturnTranspose(coords);
		if (n_codes[iNodalDisp])  fLocDisp.ReturnTranspose(disp);

		/* integrate */
		fShapes->TopIP();
		while (fShapes->NextIP())
		{
			/* material stuff */
			if (n_codes[iMaterialData])
			{
				fCurrMaterial->ComputeOutput(ipmat);
				fShapes->Extrapolate(ipmat,matdat);
			}
		}

		/* copy in the cols (in sequence of output) */
		int colcount = 0;
		nodal_all.BlockColumnCopyAt(disp  , colcount); colcount += disp.MinorDim();
		nodal_all.BlockColumnCopyAt(coords, colcount); colcount += coords.MinorDim();
		nodal_all.BlockColumnCopyAt(matdat, colcount);

		/* accumulate - extrapolation done from ip's to corners => X nodes */
		fNodes->AssembleAverage(CurrentElement().NodesX(), nodal_all);
	}
	
	/* get nodally averaged values */
	//was:
	//const iArrayT& node_used = fFEManager.OutputSet(fOutputID).NodesUsed();
	//fNodes->OutputAverage(node_used, n_values);
	fNodes->OutputUsedAverage(n_values);
}

/***********************************************************************
* Private
***********************************************************************/

/* construct output labels array */
void DiffusionElementT::GenerateOutputLabels(const iArrayT& n_codes,
	ArrayT<StringT>& n_labels, const iArrayT& e_codes, 
	ArrayT<StringT>& e_labels) const
{
	/* allocate node labels */
	n_labels.Allocate(n_codes.Sum());
	int count = 0;	

	if (n_codes[iNodalDisp])
	{
		if (fNumDOF > 6) throw eGeneralFail;
		const char* dlabels[] = {"d1", "d2", "d3", "d4", "d5", "d6"};
		for (int i = 0; i < fNumDOF; i++)
			n_labels[count++] = dlabels[i];
	}

	if (n_codes[iNodalCoord])
	{
		const char* xlabels[] = {"x1", "x2", "x3"};
		for (int i = 0; i < fNumSD; i++)
			n_labels[count++] = xlabels[i];
	}

	/* material output labels */
	if (n_codes[iMaterialData])
	{
		ArrayT<StringT> matlabels;
		(*fMaterialList)[0]->OutputLabels(matlabels);	
		
		for (int i = 0; i < n_codes[iMaterialData]; i++)
			n_labels[count++] = matlabels[i];
	}
	
//TEMP - no element labels for now
#pragma unused(e_labels)
	if (e_codes.Sum() != 0)
	{
		cout << "\n DiffusionElementT::GenerateOutputLabels: not expecting any element\n"
		     <<   "     output codes:\n" << e_codes << endl;	
		throw eGeneralFail;
	}
}
