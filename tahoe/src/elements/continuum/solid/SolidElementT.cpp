/* $Id: SolidElementT.cpp,v 1.55.2.6 2004-03-17 18:03:32 paklein Exp $ */
#include "SolidElementT.h"

#include <iostream.h>
#include <iomanip.h>
#include <math.h>

#include "toolboxConstants.h"

#include "fstreamT.h"
#include "ElementCardT.h"
#include "ShapeFunctionT.h"
#include "eIntegratorT.h"
#include "iAutoArrayT.h"
#include "ParameterContainerT.h"

/* materials */
#include "SolidMaterialT.h"
#include "SolidMatSupportT.h"
#include "SolidMatList1DT.h"
#include "SolidMatList2DT.h"
#include "SolidMatList3DT.h"

/* exception codes */
#include "ExceptionT.h"

using namespace Tahoe;

/* initialize static data */
const int SolidElementT::NumNodalOutputCodes = 7;
static const char* NodalOutputNames[] = {
	"coordinates",
	"displacements",
	"stress",
	"principal_stress",
	"strain_energy_density",
	"wave_speeds",
	"material_output"};

const int SolidElementT::NumElementOutputCodes = 7;
static const char* ElementOutputNames[] = {
	"centroid",
	"mass",
	"strain_energy",
	"kinetic_energy",
	"linear_momentum",
	"stress",
	"material_output"};

/* constructor */
SolidElementT::SolidElementT(const ElementSupportT& support, const FieldT& field):
	ContinuumElementT(support, field),
	fLocLastDisp(LocalArrayT::kLastDisp),
	fLocVel(LocalArrayT::kVel),
	fLocAcc(LocalArrayT::kAcc),
	fLocTemp(NULL),
	fLocTemp_last(NULL),
	fStress(NumSD()),
	fD(dSymMatrixT::NumValues(NumSD())),
	fStoreInternalForce(false)
{
	SetName("solid_element");

	/* check base class initializations */
	if (NumDOF() != NumSD()) throw ExceptionT::kGeneralFail;

	ifstreamT& in = ElementSupport().Input();
	ostream&  out = ElementSupport().Output();
	
	/* control parameters */
	in >> fMassType;		

	/* checks for dynamic analysis */
	if (fIntegrator->Order() > 0 &&
	    fIntegrator->ImplicitExplicit() == eIntegratorT::kExplicit &&
	    ElementSupport().Analysis() != GlobalT::kMultiField)
	    fMassType = kLumpedMass;
}

SolidElementT::SolidElementT(const ElementSupportT& support):
	ContinuumElementT(support),
	fLocLastDisp(LocalArrayT::kLastDisp),
	fLocVel(LocalArrayT::kVel),
	fLocAcc(LocalArrayT::kAcc),
	fLocTemp(NULL),
	fLocTemp_last(NULL),
	fStoreInternalForce(false)
{
	SetName("solid_element");
}

/* destructor */
SolidElementT::~SolidElementT(void)
{
	delete fLocTemp;
	delete fLocTemp_last;
}

/* data initialization */
void SolidElementT::Initialize(void)
{
	/* inherited */
	ContinuumElementT::Initialize();

	/* allocate strain-displacement matrix */
//	fB_list.Dimension(NumIP());
//	for (int i = 0; i < fB_list.Length(); i++)
//		fB_list[i].Dimension(dSymMatrixT::NumValues(NumSD()), NumSD()*NumElementNodes());
	fB.Dimension(dSymMatrixT::NumValues(NumSD()), NumSD()*NumElementNodes());

	/* setup for material output */
	if (fNodalOutputCodes[iMaterialData] || fElementOutputCodes[iIPMaterialData])
	{
		/* check compatibility of output */
		if (!CheckMaterialOutput())
		{
			cout << "\n SolidElementT::Initialize: error with material output" << endl;
			throw ExceptionT::kBadInputValue;
		}
		/* no material output variables */
		else if ((*fMaterialList)[0]->NumOutputVariables() == 0)
		{
			cout << "\n SolidElementT::Initialize: there are no material outputs" << endl;
			fNodalOutputCodes[iMaterialData] = IOBaseT::kAtNever;
			fElementOutputCodes[iIPMaterialData] = IOBaseT::kAtNever;
		}
	}	
}

/* solution calls */
void SolidElementT::AddNodalForce(const FieldT& field, int node, dArrayT& force)
{
	/* not my field */
	if (&field != &(Field())) return;

	/* quick exit */
	bool hasnode = false;
	for (int i=0; i < fBlockData.Length() && !hasnode; i++)
		if (fConnectivities[i]->HasValue(node)) hasnode = true;
	if (!hasnode) return;

	/* set components and weights */
	double constMa = 0.0;
	double constKd = 0.0;
	
	/* components dicated by the algorithm */
	int formMa = fIntegrator->FormMa(constMa);
	int formKd = fIntegrator->FormKd(constKd);

	/* body forces */
	int formBody = 0;
	if (fMassType != kNoMass &&
	   (fBodySchedule && fBody.Magnitude() > kSmall))
	{	
		formBody = 1;
		if (!formMa) constMa = 1.0; /* override */
	}

	/* override controller */
	if (fMassType == kNoMass) formMa = 0;
	
	/* temp for nodal force */
	dArrayT nodalforce;
	
	Top();
	while (NextElement())
	{
		int nodeposition;
		const iArrayT& nodes_u = CurrentElement().NodesU();
		if (nodes_u.HasValue(node, nodeposition))
		{
			/* initialize */
			fRHS = 0.0;

			/* global shape function values */
			SetGlobalShape();

			/* internal force contribution */	
			if (formKd) FormKd(constKd);

			/* inertia forces */
			if (formMa || formBody)
			{
				/* nodal accelerations */
				if (formMa)
					SetLocalU(fLocAcc);
				else 
					fLocAcc = 0.0;
			
				/* body force contribution */
				if (formBody) AddBodyForce(fLocAcc);
				
				/* calculate inertial forces */
				FormMa(fMassType, constMa*fCurrMaterial->Density(), &fLocAcc, NULL);
			}

			/* loop over nodes (double-noding OK) */
			int dex = 0;
			for (int i = 0; i < nodes_u.Length(); i++)
			{
				if (nodes_u[i] == node)
				{
					/* components for node */
					nodalforce.Set(NumDOF(), fRHS.Pointer(dex));
	
					/* accumulate */
					force += nodalforce;
				}
				dex += NumDOF();
			}
		}
	}
}

void SolidElementT::AddLinearMomentum(dArrayT& momentum)
{
	/* check */
	if (momentum.Length() != NumDOF()) throw ExceptionT::kSizeMismatch;
		
	/* loop over elements */
	Top();
	while (NextElement())
	{
		/* global shape function derivatives, jacobians, local coords */
		SetGlobalShape();
		
		/* get velocities */
		SetLocalU(fLocVel);

		/* material density */
		double density = fCurrMaterial->Density();

		/* integration */
		const double* Det    = fShapes->IPDets();
		const double* Weight = fShapes->IPWeights();
	
		fShapes->TopIP();
		while ( fShapes->NextIP() )
		{					
			double temp  = density*(*Det++)*(*Weight++);

			/* integration point velocities */
			fShapes->InterpolateU(fLocVel, fDOFvec);

			double* p    = momentum.Pointer();
			double* pvel = fDOFvec.Pointer();					
			for (int dof = 0; dof < NumDOF(); dof++)			
				*p++ += temp*(*pvel++);
		}
	}
}

/* returns the energy as defined by the derived class types */
double SolidElementT::InternalEnergy(void)
{
	double energy = 0.0;

	Top();
	while ( NextElement() )
	{
		/* global shape function derivatives, jacobians, local coords */
		SetGlobalShape();

		/* integration */
		const double* Det    = fShapes->IPDets();
		const double* Weight = fShapes->IPWeights();

		fShapes->TopIP();
		while ( fShapes->NextIP() )
			energy += fCurrMaterial->StrainEnergyDensity()*(*Det++)*(*Weight++);
	}
	
	return energy;
}

void SolidElementT::SendOutput(int kincode)
{
	/* output flags */
	iArrayT flags(fNodalOutputCodes.Length());

	/* set flags to get desired output */
	flags = IOBaseT::kAtNever;
	switch (kincode)
	{
		case iNodalDisp:
		    flags[iNodalDisp] = 1;
			break;
		case iNodalStress:
		    flags[iNodalStress] = 1;
			break;
		case iEnergyDensity:
		    flags[iEnergyDensity] = 1;
			break;
		case iPrincipal:
			flags[iPrincipal] = 1;
			break;
		case iMaterialData:
			flags[iMaterialData] = 1;
			break;
		default:
			cout << "\n SolidElementT::SendOutput: invalid output code: " 
			     << kincode << endl;
	}

	/* number of output values */
	iArrayT n_counts;
	SetNodalOutputCodes(IOBaseT::kAtInc, flags, n_counts);

	/* reset averaging workspace */
	ElementSupport().ResetAverage(n_counts.Sum());
      
	/* no element output */
	iArrayT e_counts(fElementOutputCodes.Length());
	e_counts = 0;

	/* generate output */
	dArray2DT n_values, e_values;
	ComputeOutput(n_counts, n_values, e_counts, e_values);
}

/* contribution to the nodal residual forces */
const dArray2DT& SolidElementT::InternalForce(int group)
{
	const char caller[] = "SolidElementT::InternalForce";

	/* check */
	if (group != Group())
		ExceptionT::GeneralFail(caller, "expecting solver group %d not %d", 
			Group(), group);
			
	/* must be storing force */
	if (!fStoreInternalForce)
		ExceptionT::GeneralFail(caller, "internal force not being stored");
			
	return fForce;
}

/* describe the parameters needed by the interface */
void SolidElementT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ContinuumElementT::DefineParameters(list);

	/* mass type */
	ParameterT mass_type(ParameterT::Enumeration, "mass_type");
	mass_type.AddEnumeration("no_mass", kNoMass);
    mass_type.AddEnumeration("consistent_mass", kConsistentMass);
    mass_type.AddEnumeration("lumped_mass", kLumpedMass);
    mass_type.SetDefault(kConsistentMass);
	list.AddParameter(mass_type);
}

/* information about subordinate parameter lists */
void SolidElementT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	ContinuumElementT::DefineSubs(sub_list);

	/* nodal output codes (optional) */
	sub_list.AddSub("solid_element_nodal_output", ParameterListT::ZeroOrOnce);
	sub_list.AddSub("solid_element_element_output", ParameterListT::ZeroOrOnce);
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* SolidElementT::NewSub(const StringT& list_name) const
{
	if (list_name == "solid_element_nodal_output")
	{
		ParameterContainerT* node_output = new ParameterContainerT(list_name);
		
		/* all false by default */
		for (int i = 0; i < NumNodalOutputCodes; i++)
			node_output->AddParameter(ParameterT::Integer, NodalOutputNames[i], ParameterListT::ZeroOrOnce);

		return node_output;
	}
	else if (list_name == "solid_element_element_output")
	{
		ParameterContainerT* element_output = new ParameterContainerT(list_name);
		
		/* all false by default */
		for (int i = 0; i < NumElementOutputCodes; i++)
			element_output->AddParameter(ParameterT::Integer, ElementOutputNames[i], ParameterListT::ZeroOrOnce);

		return element_output;	
	}
	else /* inherited */
		return ContinuumElementT::NewSub(list_name);
}

/* accept parameter list */
void SolidElementT::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	ContinuumElementT::TakeParameterList(list);

	/* set mass type */
	list.GetParameter("mass_type", enum2int<ContinuumElementT::MassTypeT>(fMassType));

	/* allocate work space */
//	fB_list.Dimension(NumIP());
//	for (int i = 0; i < fB_list.Length(); i++)
//		fB_list[i].Dimension(dSymMatrixT::NumValues(NumSD()), NumSD()*NumElementNodes());
	fB.Dimension(dSymMatrixT::NumValues(NumSD()), NumSD()*NumElementNodes());
	fD.Dimension(dSymMatrixT::NumValues(NumSD()));
	
	/* nodal output codes */
	fNodalOutputCodes.Dimension(NumNodalOutputCodes);
	fNodalOutputCodes = 0;
	qUseSimo = qNoExtrap = false;	
	const ParameterListT* node_output = list.List("solid_element_nodal_output");
	if (node_output)
		for (int i = 0; i < NumNodalOutputCodes; i++)
		{
			/* look for entry */
			const ParameterT* nodal_value = node_output->Parameter(NodalOutputNames[i]);
			if (nodal_value) {
				int do_write = *nodal_value;

				/* Additional smoothing flags */
				if (!qUseSimo && do_write == 3) {
	    			qUseSimo = qNoExtrap = true;
	    			fNodalOutputCodes[i] = 3;
	    		}
	    		else if (!qNoExtrap && do_write == 2) {
	    			qNoExtrap = true;
	    			fNodalOutputCodes[i] = 2;
	    		}
	    		else if (do_write == 1)
	    			fNodalOutputCodes[i] = 1;
			}
		}

	/* element output codes */
	fElementOutputCodes.Dimension(NumElementOutputCodes);
	fElementOutputCodes = 0;
	const ParameterListT* element_output = list.List("solid_element_element_output");
	if (element_output)
		for (int i = 0; i < NumElementOutputCodes; i++)
		{
			/* look for entry */
			const ParameterT* element_value = element_output->Parameter(ElementOutputNames[i]);
			if (element_value) {
				int do_write = *element_value;
				if (do_write == 1)
					fElementOutputCodes[i] = 1;
			}
		}

	/* generate list of material needs */
	fMaterialNeeds.Dimension(fMaterialList->Length());
	for (int i = 0; i < fMaterialNeeds.Length(); i++)
	{
		/* allocate */
		ArrayT<bool>& needs = fMaterialNeeds[i];
		needs.Dimension(3);

		/* casts are safe since class contructs materials list */
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[i];
		SolidMaterialT* mat = (SolidMaterialT*) pcont_mat;

		/* collect needs */
		needs[kNeedDisp] = mat->NeedDisp();
		needs[kNeedVel] = mat->NeedVel();
		needs[KNeedLastDisp] = mat->NeedLastDisp();
	}
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* construct list of materials from the input stream */
void SolidElementT::ReadMaterialData(ifstreamT& in)
{
	/* inherited */
	ContinuumElementT::ReadMaterialData(in);
	
	/* generate list of material needs */
	fMaterialNeeds.Dimension(fMaterialList->Length());
	for (int i = 0; i < fMaterialNeeds.Length(); i++)
	{
		/* allocate */
		ArrayT<bool>& needs = fMaterialNeeds[i];
		needs.Dimension(3);

		/* casts are safe since class contructs materials list */
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[i];
		SolidMaterialT* mat = (SolidMaterialT*) pcont_mat;

		/* collect needs */
		needs[kNeedDisp] = mat->NeedDisp();
		needs[kNeedVel] = mat->NeedVel();
		needs[KNeedLastDisp] = mat->NeedLastDisp();
	}
}

/* print element group data */
void SolidElementT::PrintControlData(ostream& out) const
{
	/* inherited */
	ContinuumElementT::PrintControlData(out);

	/* control parameters */
	out << " Mass type code. . . . . . . . . . . . . . . . . = " << fMassType << '\n';
	out << "    eq." << kNoMass			<< ", no mass matrix\n";
	out << "    eq." << kConsistentMass	<< ", consistent mass matrix\n";
	out << "    eq." << kLumpedMass		<< ", lumped mass matrix\n";
}

void SolidElementT::EchoOutputCodes(ifstreamT& in, ostream& out)
{
	/* allocate nodal output codes */
	fNodalOutputCodes.Dimension(NumNodalOutputCodes);

	qUseSimo = qNoExtrap = false;

	/* read in at a time to allow comments */
	for (int i = 0; i < fNodalOutputCodes.Length(); i++)
	{
		in >> fNodalOutputCodes[i];
		
		/* Additional smoothing flags */
	    if (!qUseSimo && fNodalOutputCodes[i] == 3)
	 	{
	    	qUseSimo = qNoExtrap = true;
	    }
	    else if (!qNoExtrap && fNodalOutputCodes[i] == 2)
	    {
	    	qNoExtrap = true;
	    }
  				
		/* convert all to "at print increment" */
		if (fNodalOutputCodes[i] != IOBaseT::kAtNever)
			fNodalOutputCodes[i] = IOBaseT::kAtInc;
	
		if (i == iWaveSpeeds && fNodalOutputCodes[iWaveSpeeds] != IOBaseT::kAtNever)
		{
			fNormal.Dimension(NumSD());
			in >> fNormal;
			fNormal.UnitVector();
		}
	}
		
	/* checks */
	if (fNodalOutputCodes.Min() < IOBaseT::kAtFail ||
	    fNodalOutputCodes.Max() > IOBaseT::kAtInc) throw ExceptionT::kBadInputValue;

	/* echo */
	out << " Number of nodal output codes. . . . . . . . . . = " << fNodalOutputCodes.Length() << '\n';
	out << "    [" << fNodalOutputCodes[iNodalCoord   ] << "]: initial nodal coordinates\n";
	out << "    [" << fNodalOutputCodes[iNodalDisp    ] << "]: nodal displacements\n";
	out << "    [" << fNodalOutputCodes[iNodalStress  ] << "]: nodal stresses\n";
	out << "    [" << fNodalOutputCodes[iPrincipal    ] << "]: nodal principal stresses\n";
	out << "    [" << fNodalOutputCodes[iEnergyDensity] << "]: nodal strain energy density\n";
	out << "    [" << fNodalOutputCodes[iWaveSpeeds   ] << "]: wave speeds\n";
	out << "    [" << fNodalOutputCodes[iMaterialData ] << "]: nodal material output parameters\n";
	
	if (fNodalOutputCodes[iWaveSpeeds] == 1)
	{
		out << " Wave speed sampling direction:\n";
		for (int i = 0; i < NumSD(); i++)
			out << "   N[" << i+1 << "] = " << fNormal[i] << '\n';
	}

	/* allocate nodal output codes */
	fElementOutputCodes.Dimension(NumElementOutputCodes);
	fElementOutputCodes = IOBaseT::kAtNever;

//TEMP - backward compatibility
	if (StringT::versioncmp(ElementSupport().Version(), "v3.01") < 1)
	{
		/* message */
		cout << "\n SolidElementT::EchoOutputCodes: use input file version newer than v3.01\n" 
		     <<   "     to enable element output control" << endl;
		out << "\n SolidElementT::EchoOutputCodes: use input file version newer than v3.01\n" 
		    <<   "     to enable element output control" << endl;	
	}
	else
	{
		int num_codes = (StringT::versioncmp(ElementSupport().Version(), "v3.4.1") < 0) ? 5 : 7;
	
		/* read in at a time to allow comments */
		for (int j = 0; j < num_codes; j++)
		{
			in >> fElementOutputCodes[j];
		
			/* convert all to "at print increment" */
			if (fElementOutputCodes[j] != IOBaseT::kAtNever)
				fElementOutputCodes[j] = IOBaseT::kAtInc;
		}	

		/* defaults */
		if (fIntegrator->Order() == 0)
		{
			fElementOutputCodes[iKineticEnergy] = IOBaseT::kAtNever;
			fElementOutputCodes[iLinearMomentum] = IOBaseT::kAtNever;
		}

		/* checks */
		if (fElementOutputCodes.Min() < IOBaseT::kAtFail ||
		    fElementOutputCodes.Max() > IOBaseT::kAtInc) throw ExceptionT::kBadInputValue;
	}

	/* echo */
	out << " Number of element output codes. . . . . . . . . = " << fElementOutputCodes.Length() << '\n';
	out << "    [" << fElementOutputCodes[iCentroid      ] << "]: reference centroid\n";
	out << "    [" << fElementOutputCodes[iMass          ] << "]: ip mass\n";
	out << "    [" << fElementOutputCodes[iStrainEnergy  ] << "]: strain energy\n";
	out << "    [" << fElementOutputCodes[iKineticEnergy ] << "]: kinetic energy\n";
	out << "    [" << fElementOutputCodes[iLinearMomentum] << "]: linear momentum\n";
	out << "    [" << fElementOutputCodes[iIPStress      ] << "]: ip stresses and strains\n";
	out << "    [" << fElementOutputCodes[iIPMaterialData] << "]: ip material output parameters\n";
}

/* construct output labels array */
void SolidElementT::SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* initialize */
	counts.Dimension(flags.Length());
	counts = 0;

	/* set output flags */
	if (flags[iNodalCoord] == mode)
		counts[iNodalCoord] = NumSD();
	if (flags[iNodalDisp] == mode)
		counts[iNodalDisp] = NumDOF();
	if (flags[iNodalStress] == mode)
		counts[iNodalStress] = dSymMatrixT::NumValues(NumSD());
	if (flags[iPrincipal] == mode)
		counts[iPrincipal] = NumSD();
	if (flags[iEnergyDensity] == mode)
		counts[iEnergyDensity] = 1;
	if (flags[iWaveSpeeds] == mode)
		counts[iWaveSpeeds] = NumSD();
	if (flags[iMaterialData] == mode)
		counts[iMaterialData] = (*fMaterialList)[0]->NumOutputVariables();
}

void SolidElementT::SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* initialize */
	counts.Dimension(flags.Length());
	counts = 0;

	/* set output flags */
	if (fElementOutputCodes[iCentroid] == mode) counts[iCentroid] = NumSD();
	if (fElementOutputCodes[iMass] == mode) counts[iMass] = NumIP();
	if (fElementOutputCodes[iStrainEnergy] == mode) counts[iStrainEnergy] = 1;
	if (fElementOutputCodes[iKineticEnergy] == mode) counts[iKineticEnergy] = 1;
	if (fElementOutputCodes[iLinearMomentum] == mode) counts[iLinearMomentum] = NumDOF();
	if (fElementOutputCodes[iIPStress] == mode) counts[iIPStress] = 2*dSymMatrixT::NumValues(NumSD())*NumIP();
	if (fElementOutputCodes[iIPMaterialData] == mode) 
		counts[iIPMaterialData] = (*fMaterialList)[0]->NumOutputVariables()*NumIP();
}

/* initialize local arrays */
void SolidElementT::SetLocalArrays(void)
{
	/* inherited */
	ContinuumElementT::SetLocalArrays();

	/* allocate */
	int nen = NumElementNodes();
	fLocLastDisp.Dimension(nen, NumDOF());
	fLocAcc.Dimension(nen, NumDOF());
	fLocVel.Dimension(nen, NumDOF());

	/* register */
	Field().RegisterLocal(fLocLastDisp);
	if (fIntegrator->Order() == 2)
	{
		Field().RegisterLocal(fLocVel);
		Field().RegisterLocal(fLocAcc);
	}

	/* look for a temperature field */
	const FieldT* temperature = ElementSupport().Field("temperature");
	if (temperature) {
	
		/* construct */
		fLocTemp = new LocalArrayT(LocalArrayT::kDisp, nen, temperature->NumDOF());
		fLocTemp_last = new LocalArrayT(LocalArrayT::kLastDisp, nen, temperature->NumDOF());

		/* register */
		temperature->RegisterLocal(*fLocTemp);
		temperature->RegisterLocal(*fLocTemp_last);
	}
}

/* set the correct shape functions */
void SolidElementT::SetShape(void)
{
	/* construct shape functions */
	fShapes = new ShapeFunctionT(GeometryCode(), NumIP(), fLocInitCoords);
	if (!fShapes) throw ExceptionT::kOutOfMemory;

	/* initialize */
	fShapes->Initialize();
}

/* form shape functions and derivatives */
void SolidElementT::SetGlobalShape(void)
{
	/* inherited */
	ContinuumElementT::SetGlobalShape();

	/* material needs */
	const ArrayT<bool>& needs = fMaterialNeeds[CurrentElement().MaterialNumber()];

	/* material dependent local arrays */
	if (needs[kNeedDisp])     SetLocalU(fLocDisp);	
	if (needs[KNeedLastDisp]) SetLocalU(fLocLastDisp);	
	if (needs[kNeedVel])
	{
		/* have velocity */
		if (fLocVel.IsRegistered())
			SetLocalU(fLocVel);
		else /* finite difference approximation */
		{
			double onebydt = 1.0/ElementSupport().TimeStep();
			fLocVel.SetToCombination(onebydt, fLocDisp, -onebydt, fLocLastDisp);
		}
	}
	
	/* get nodal temperatures if available */
	if (fLocTemp) SetLocalU(*fLocTemp);
	if (fLocTemp_last) SetLocalU(*fLocTemp_last);
}

/* construct a new material support and return a pointer */
MaterialSupportT* SolidElementT::NewMaterialSupport(MaterialSupportT* p) const
{
	/* allocate new */
	if (!p) p = new SolidMatSupportT(NumSD(), NumDOF(), NumIP());

	/* inherited initializations */
	ContinuumElementT::NewMaterialSupport(p);

	/* set SolidMatSupportT fields */
	SolidMatSupportT* ps = TB_DYNAMIC_CAST(SolidMatSupportT*, p);
	if (ps) {
		/* set pointers to local arrays */
		ps->SetLocalArray(fLocLastDisp);
		ps->SetLocalArray(fLocVel);
		ps->SetLocalArray(fLocAcc);

		/* temperatures if available */
		if (fLocTemp) ps->SetTemperatures(*fLocTemp);
		if (fLocTemp_last) ps->SetLastTemperatures(*fLocTemp_last);
	}

	return p;
}

/* set the \e B matrix using the given shape function derivatives */
void SolidElementT::Set_B(const dArray2DT& DNa, dMatrixT& B) const
{
#if __option(extended_errorcheck)
	if (B.Rows() != dSymMatrixT::NumValues(DNa.MajorDim()) ||
	    B.Cols() != DNa.Length())
	    throw ExceptionT::kSizeMismatch;
#endif

	int nnd = DNa.MinorDim();
	double* pB = B.Pointer();

	/* 1D */
	if (DNa.MajorDim() == 1)
	{
		const double* pNax = DNa(0);
		for (int i = 0; i < nnd; i++)
			*pB++ = *pNax++;
	}
	/* 2D */
	else if (DNa.MajorDim() == 2)
	{
		const double* pNax = DNa(0);
		const double* pNay = DNa(1);
		for (int i = 0; i < nnd; i++)
		{
			/* see Hughes (2.8.20) */
			*pB++ = *pNax;
			*pB++ = 0.0;
			*pB++ = *pNay;

			*pB++ = 0.0;
			*pB++ = *pNay++;
			*pB++ = *pNax++;
		}
	}
	/* 3D */
	else		
	{
		const double* pNax = DNa(0);
		const double* pNay = DNa(1);
		const double* pNaz = DNa(2);
		for (int i = 0; i < nnd; i++)
		{
			/* see Hughes (2.8.21) */
			*pB++ = *pNax;
			*pB++ = 0.0;
			*pB++ = 0.0;
			*pB++ = 0.0;
			*pB++ = *pNaz;
			*pB++ = *pNay;

			*pB++ = 0.0;
			*pB++ = *pNay;
			*pB++ = 0.0;
			*pB++ = *pNaz;
			*pB++ = 0.0;
			*pB++ = *pNax;

			*pB++ = 0.0;
			*pB++ = 0.0;
			*pB++ = *pNaz++;
			*pB++ = *pNay++;
			*pB++ = *pNax++;
			*pB++ = 0.0;
		}
	}
}

/* set B-bar as given by Hughes (4.5.11-16) */
void SolidElementT::Set_B_bar(const dArray2DT& DNa, const dArray2DT& mean_gradient, 
	dMatrixT& B)
{
#if __option(extended_errorcheck)
	if (B.Rows() != dSymMatrixT::NumValues(DNa.MajorDim()) ||
	    B.Cols() != DNa.Length() ||
	    mean_gradient.MinorDim() != DNa.MinorDim() ||
	    mean_gradient.MajorDim() != DNa.MajorDim())
	    throw ExceptionT::kSizeMismatch;
#endif

	int nnd = DNa.MinorDim();
	double* pB = B.Pointer();

	/* 1D */
	if (DNa.MajorDim() == 1)
	{
		cout << "\n SolidElementT::Set_B_bar: not implemented yet for 1D B-bar" << endl;
		throw ExceptionT::kGeneralFail;
	}
	/* 2D */
	else if (DNa.MajorDim() == 2)
	{
		const double* pNax = DNa(0);
		const double* pNay = DNa(1);
			
		const double* pBmx = mean_gradient(0);
		const double* pBmy = mean_gradient(1);
			
		for (int i = 0; i < nnd; i++)
		{
			double factx = ((*pBmx++) - (*pNax))/3.0;
			double facty = ((*pBmy++) - (*pNay))/3.0;
			
			/* Hughes (4.5.11-16) */
			*pB++ = *pNax + factx;
			*pB++ = factx;
			*pB++ = *pNay;
	
			*pB++ = facty;
			*pB++ = *pNay + facty;
			*pB++ = *pNax;
				
			pNax++; pNay++;
		}
	}
	/* 3D */
	else		
	{
		const double* pNax = DNa(0);
		const double* pNay = DNa(1);
		const double* pNaz = DNa(2);

		const double* pBmx = mean_gradient(0);
		const double* pBmy = mean_gradient(1);
		const double* pBmz = mean_gradient(2);
			
		for (int i = 0; i < nnd; i++)
		{
			double factx = ((*pBmx++) - (*pNax))/3.0;
			double facty = ((*pBmy++) - (*pNay))/3.0;
			double factz = ((*pBmz++) - (*pNaz))/3.0;

			/* Hughes (4.5.11-16) */
			*pB++ = *pNax + factx;
			*pB++ = factx;
			*pB++ = factx;
			*pB++ = 0.0;
			*pB++ = *pNaz;
			*pB++ = *pNay;

			*pB++ = facty;
			*pB++ = *pNay + facty;
			*pB++ = facty;
			*pB++ = *pNaz;
			*pB++ = 0.0;
			*pB++ = *pNax;
	
			*pB++ = factz;
			*pB++ = factz;
			*pB++ = *pNaz + factz;
			*pB++ = *pNay;
			*pB++ = *pNax;
			*pB++ = 0.0;
				
			pNax++; pNay++; pNaz++;
		}
	}
}

/* construct the effective mass matrix */
void SolidElementT::LHSDriver(GlobalT::SystemTypeT sys_type)
{
	/* inherited */
	ContinuumElementT::LHSDriver(sys_type);

	/* element contribution */
	ElementLHSDriver();
}

void SolidElementT::ElementLHSDriver(void)
{
	/* set components and weights */
	double constM = 0.0;
	double constK = 0.0;
	
	int formM = fIntegrator->FormM(constM);
	int formK = fIntegrator->FormK(constK);

	/* override algorithm */
	if (fMassType == kNoMass) formM = 0;

	/* quick exit */
	if ((formM == 0 && formK == 0) ||
	    (fabs(constM) < kSmall &&
	     fabs(constK) < kSmall)) return;

	/* loop over elements */
	Top();
	while (NextElement())
		if (CurrentElement().Flag() != kOFF)
		{
			double constKe = constK;
			double constMe = constM;
	
			/* initialize */
			fLHS = 0.0;
		
			/* set shape function derivatives */
			SetGlobalShape();

			/* element mass */
			if (fabs(constMe) > kSmall)
				FormMass(fMassType, constMe*(fCurrMaterial->Density()));

			/* element stiffness */
			if (fabs(constKe) > kSmall)
				FormStiffness(constKe);

			/* add to global equations */
			AssembleLHS();
		}
}

void SolidElementT::RHSDriver(void)
{
	/* inherited */
	ContinuumElementT::RHSDriver();

	/* element contribution */
	ElementRHSDriver();
}

/* form the residual force vector */
void SolidElementT::ElementRHSDriver(void)
{
	/* heat source if needed */
	const FieldT* temperature = ElementSupport().Field("temperature");

	/* initialize sources */
	if (temperature && fIncrementalHeat.Length() == 0) {
	
		/* allocate the element heat */
		fElementHeat.Dimension(fShapes->NumIP());
			
		/* initialize heat source arrays */
		fIncrementalHeat.Dimension(fBlockData.Length());
		for (int i = 0; i < fIncrementalHeat.Length(); i++)
		{
			/* dimension */
			fIncrementalHeat[i].Dimension(fBlockData[i].Dimension(), NumIP());

			/* register */
			temperature->RegisterSource(fBlockData[i].ID(), fIncrementalHeat[i]);
		}
	}
	
	/* storage for entire internal force */
	if (fStoreInternalForce) {
		fForce.Dimension(ElementSupport().NumNodes(), NumDOF());
		fForce = 0.0;	
	} else 
		fForce.Dimension(0, NumDOF());

	/* set components and weights */
	double constMa = 0.0;
	double constKd = 0.0;
	
	/* components dicated by the algorithm */
	int formMa = fIntegrator->FormMa(constMa);
	int formKd = fIntegrator->FormKd(constKd);

	/* body forces */
	int formBody = 0;
	if (fMassType != kNoMass &&
	   (fBodySchedule && fBody.Magnitude() > kSmall))
	{	
		formBody = 1;
		if (!formMa) constMa = 1.0; /* override */
	}

	/* override controller */
	if (fMassType == kNoMass) formMa = 0;

	int block_count = 0, block_dex = 0;
	Top();
	while (NextElement())
	{
		/* advance to block [skip empty blocks] */		
		while (block_count == fBlockData[block_dex].Dimension()) {
			block_count = 0;
			block_dex++;
		}

		/* current element */
		const ElementCardT& element = CurrentElement();

		if (element.Flag() != kOFF)
		{
			/* initialize */
			fRHS = 0.0;
			fElementHeat = 0.0;
		
			/* global shape function values */
			SetGlobalShape();
		
			/* internal force contribution */	
			if (formKd) FormKd(-constKd);
				
			/* inertia forces */
			if (formMa || formBody)
			{
				/* nodal accelerations */
				if (formMa)
					SetLocalU(fLocAcc);
				else 
					fLocAcc = 0.0;
			
				/* body force contribution */
				if (formBody) AddBodyForce(fLocAcc);
		
				FormMa(fMassType, -constMa*fCurrMaterial->Density(), &fLocAcc, NULL);
			}
		
			/* store incremental heat */
			if (temperature)
				fIncrementalHeat[block_dex].SetRow(block_count, fElementHeat);

			/* global assembly */
			if (fStoreInternalForce) 
			{
				double* pRHS = fRHS.Pointer();
				int ndof = NumDOF();
				const iArrayT& nodes_u = CurrentElement().NodesU();
				for (int i = 0; i < nodes_u.Length(); i++) 
				{
					double* pForce = fForce(nodes_u[i]);
					for (int j = 0; j < ndof; j++)
						*pForce++ += *pRHS++;
				}
			}
			else /* assemble element-by-element */
				AssembleRHS();
		}
		
		/* next block */
		block_count++;
	}
	
	/* assemble all at once */
	if (fStoreInternalForce)
		ElementSupport().AssembleRHS(Group(), fForce, Field().Equations());
}

/* current element operations */
bool SolidElementT::NextElement(void)
{
	/* inherited */
	bool result = ContinuumElementT::NextElement();
	
	/* get material pointer */
	if (result)
	{
		ContinuumMaterialT* pcont_mat = (*fMaterialList)[CurrentElement().MaterialNumber()];
	
		/* cast is safe since class contructs materials list */
		fCurrMaterial = (SolidMaterialT*) pcont_mat;
	}
	
	return result;
}

/* form of tangent matrix */
GlobalT::SystemTypeT SolidElementT::TangentType(void) const
{
	/* special case */
	if (fIntegrator->Order() > 0 &&
	    fIntegrator->ImplicitExplicit() ==  eIntegratorT::kExplicit &&
	    (fMassType == kNoMass ||
	     fMassType == kLumpedMass))
		return GlobalT::kDiagonal;
	else
		/* inherited */
		return ContinuumElementT::TangentType();
}

/* return the materials list */
const SolidMatListT& SolidElementT::StructuralMaterialList(void) const
{
	return TB_DYNAMIC_CAST(const SolidMatListT&, MaterialsList());
}

/* extrapolate the integration point stresses and strains and extrapolate */
void SolidElementT::ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
        const iArrayT& e_codes, dArray2DT& e_values)
{
        /* number of output values */
        int n_out = n_codes.Sum();
        int e_out = e_codes.Sum();
        
        int n_simo, n_extrap;
        if (qUseSimo)
        {
                n_simo = n_out - n_codes[iNodalDisp] - n_codes[iNodalCoord];
                n_extrap = n_codes[iNodalDisp] + n_codes[iNodalCoord];
        }
        else
        {
                n_simo = 0;
                n_extrap = n_out;
        }
                
        /* nothing to output */
        if (n_out == 0 && e_out == 0) return;

        /* dimensions */
        int nsd = NumSD();
        int ndof = NumDOF();
        int nen = NumElementNodes();
        int nnd = ElementSupport().NumNodes();

        /* reset averaging workspace */
        ElementSupport().ResetAverage(n_extrap);

        /* allocate element results space */
        e_values.Dimension(NumElements(), e_out);

        /* nodal work arrays */
        dArray2DT nodal_space(nen, n_extrap);
        dArray2DT nodal_all(nen, n_extrap);
        dArray2DT coords, disp;
        dArray2DT nodalstress, princstress, matdat;
        dArray2DT energy, speed;

        /* ip values */
        dSymMatrixT cauchy(nsd), nstr_tmp;
        dArrayT ipmat(n_codes[iMaterialData]), ipenergy(1);
        dArrayT ipspeed(nsd), ipprincipal(nsd);

        /* set shallow copies */
        double* pall = nodal_space.Pointer();
        coords.Set(nen, n_codes[iNodalCoord], pall)      ; pall += coords.Length();
        disp.Set(nen, n_codes[iNodalDisp], pall)         ; pall += disp.Length();
        
        /* workspaces for Simo */
        int simo_offset = coords.MinorDim() + disp.MinorDim();
        dArray2DT simo_space(nen,qUseSimo ? n_simo : 0);
        dArray2DT simo_all(nen,qUseSimo ? n_simo : 0);
        dArray2DT simoNa_bar(nen,qUseSimo ? 1 : 0);
        dArray2DT simo_force;
        dArray2DT simo_mass;
        iArrayT simo_counts;
        
        if (!qUseSimo) 
        {
                nodalstress.Set(nen, n_codes[iNodalStress], pall); pall += nodalstress.Length();
                princstress.Set(nen, n_codes[iPrincipal], pall)  ; pall += princstress.Length();
                energy.Set(nen, n_codes[iEnergyDensity], pall)   ; pall += energy.Length();
                speed.Set(nen, n_codes[iWaveSpeeds], pall)       ; pall += speed.Length();
                matdat.Set(nen, n_codes[iMaterialData], pall);  
        }
        else
        {
                simo_force.Dimension(ElementSupport().NumNodes(),qUseSimo ? n_simo : 0);
                simo_mass.Dimension(ElementSupport().NumNodes(),qUseSimo ? 1 : 0);
                simo_counts.Dimension(ElementSupport().NumNodes());
        
                pall = simo_space.Pointer();
                nodalstress.Set(nen, n_codes[iNodalStress], pall); pall += nodalstress.Length();
                princstress.Set(nen, n_codes[iPrincipal], pall)  ; pall += princstress.Length();
                energy.Set(nen, n_codes[iEnergyDensity], pall)   ; pall += energy.Length();
                speed.Set(nen, n_codes[iWaveSpeeds], pall)       ; pall += speed.Length();
                matdat.Set(nen, n_codes[iMaterialData], pall); 
                
                simo_mass = 0.;
                simo_force = 0.;
                simo_counts = 0;
        }
        
        /* element work arrays */
        dArrayT element_values(e_values.MinorDim());
        pall = element_values.Pointer();
        dArrayT centroid, ip_centroid, ip_mass;
        if (e_codes[iCentroid])
        {
                centroid.Set(nsd, pall); pall += nsd;
                ip_centroid.Dimension(nsd);
        }
        if (e_codes[iMass]) {
                ip_mass.Set(NumIP(), pall); 
                pall += NumIP();
        }
        double w_tmp, ke_tmp;
        double mass;
        double& strain_energy = (e_codes[iStrainEnergy]) ? *pall++ : w_tmp;
        double& kinetic_energy = (e_codes[iKineticEnergy]) ? *pall++ : ke_tmp;
        dArrayT linear_momentum, ip_velocity;
        if (e_codes[iLinearMomentum])
        {
                linear_momentum.Set(ndof, pall); pall += ndof;
                ip_velocity.Dimension(ndof);
        }
        else if (e_codes[iKineticEnergy]) ip_velocity.Dimension(ndof);
        
        dArray2DT ip_stress;
        if (e_codes[iIPStress])
        {
                ip_stress.Set(NumIP(), e_codes[iIPStress]/NumIP(), pall);
                pall += ip_stress.Length();
        }
        dArray2DT ip_material_data;
        if (e_codes[iIPMaterialData])
        {
                ip_material_data.Set(NumIP(), e_codes[iIPMaterialData]/NumIP(), pall);
                pall += ip_material_data.Length();
                ipmat.Dimension(ip_material_data.MinorDim());
        }

        /* check that degrees are displacements */
        int interpolant_DOF = InterpolantDOFs();

        Top();
        while (NextElement())
        if (CurrentElement().Flag() != kOFF)
        {
			/* initialize */
            nodal_space = 0.0;
            simo_space = 0.;
            simo_all = 0.;
            simoNa_bar = 0.;

			/* global shape function values */
			SetGlobalShape();

                /* collect nodal values */
                if (e_codes[iKineticEnergy] || e_codes[iLinearMomentum])
                        SetLocalU(fLocVel);
                
                /* coordinates and displacements all at once */
                if (n_codes[iNodalCoord]) fLocInitCoords.ReturnTranspose(coords);
                if (n_codes[ iNodalDisp])
                {
                        if (interpolant_DOF)
                                fLocDisp.ReturnTranspose(disp);
                        else
                                NodalDOFs(CurrentElement().NodesX(), disp);
                }
                
                /* initialize element values */
                mass = strain_energy = kinetic_energy = 0;
                if (e_codes[iCentroid]) centroid = 0.0;
                if (e_codes[iLinearMomentum]) linear_momentum = 0.0;
                const double* j = fShapes->IPDets();
                const double* w = fShapes->IPWeights();
                double density = fCurrMaterial->Density();

                /* integrate */
                dArray2DT Na_X_ip_w;
                fShapes->TopIP();
                while (fShapes->NextIP())
                {
                        /* element integration weight */
                        double ip_w = (*j++)*(*w++);
                        if (qUseSimo || qNoExtrap)
                        {
                          Na_X_ip_w.Dimension(nen,1);
                          if (qUseSimo)
                            {
                                const double* Na_X = fShapes->IPShapeX();
                                Na_X_ip_w = ip_w;
                                for (int k = 0; k < nen; k++)
                                {
                                        Na_X_ip_w(k,0) *= *Na_X++;
                                }
                                simoNa_bar += Na_X_ip_w;
                            }
                          else
                            for (int k = 0; k < nen; k++)
                                Na_X_ip_w(k,0) = 1.;
                        }
                
                        /* get Cauchy stress */
			const dSymMatrixT& stress = fCurrMaterial->s_ij();
			int dim = stress.Length();
			if (dim == 4)
			{
			  cauchy[0] = stress[0];
			  cauchy[1] = stress[1];
			  cauchy[2] = stress[2];
			} 
			else
			  cauchy = stress;

                        /* stresses */
                        if (n_codes[iNodalStress])
                        {        
                                if (qNoExtrap)
                                        for (int k = 0; k < nen; k++)
                                                nodalstress.AddToRowScaled(k,Na_X_ip_w(k,0),cauchy);
                                else
                                        fShapes->Extrapolate(cauchy, nodalstress);
                        }
                        
                        if (e_codes[iIPStress]) {
                        	double* row = ip_stress(fShapes->CurrIP());
                        	nstr_tmp.Set(nsd, row);
                        	nstr_tmp = cauchy;
                        	row += cauchy.Length();
                        	nstr_tmp.Set(nsd, row);
                        	fCurrMaterial->Strain(nstr_tmp);
                        }

                        /* wave speeds */
                        if (n_codes[iWaveSpeeds])
                        {
                                /* acoustic wave speeds */
                                fCurrMaterial->WaveSpeeds(fNormal, ipspeed);
                                if (qNoExtrap)
                                        for (int k = 0; k < nen; k++)
                                                speed.AddToRowScaled(k,Na_X_ip_w(k,0),ipspeed);
                                else
                                        fShapes->Extrapolate(ipspeed, speed);
                        }

                        /* principal values - compute principal before smoothing */
                        if (n_codes[iPrincipal])
                        {
                                /* compute eigenvalues */
                                cauchy.PrincipalValues(ipprincipal);
                                if (qNoExtrap)
                                        for (int k = 0; k < nen; k++)
                                                princstress.AddToRowScaled(k,Na_X_ip_w(k,0),ipprincipal);
                                else
                                        fShapes->Extrapolate(ipprincipal, princstress);        
                        }

                        /* strain energy density */
                        if (n_codes[iEnergyDensity] || e_codes[iStrainEnergy])
                        {
                                double ip_strain_energy = fCurrMaterial->StrainEnergyDensity();
                        
                                /* nodal average */
                                if (n_codes[iEnergyDensity])
                                {
                                        ipenergy[0] = ip_strain_energy;
                                        if (qNoExtrap)
                                          for (int k = 0; k < nen; k++)
                                                energy.AddToRowScaled(k,Na_X_ip_w(k,0),ipenergy);
                                        else
                                                fShapes->Extrapolate(ipenergy,energy);
                                }
                                
                                /* integrate over element */
                                if (e_codes[iStrainEnergy])
                                        strain_energy += ip_w*ip_strain_energy;
                        }

                        /* material stuff */
                        if (n_codes[iMaterialData] || e_codes[iIPMaterialData])
                        {
                                /* compute material output */
                                fCurrMaterial->ComputeOutput(ipmat);
                                
                                /* store nodal data */
                                if (n_codes[iMaterialData])
                                {
                                        if (qNoExtrap)
                                                for (int k = 0; k < nen; k++)
                                                        matdat.AddToRowScaled(k,Na_X_ip_w(k,0),ipmat);
                                        else 
                                                fShapes->Extrapolate(ipmat, matdat);
                                }
                                
                                /* store element data */
                                if (e_codes[iIPMaterialData]) ip_material_data.SetRow(fShapes->CurrIP(), ipmat);
                        }
                        
                        /* mass averaged centroid */
                        if (e_codes[iCentroid] || e_codes[iMass])
                        {
                                /* mass */
                                mass += ip_w*density;
                                
                                /* integration point mass */
                                if (e_codes[iMass]) ip_mass[fShapes->CurrIP()] = ip_w*density;
                        
                                /* moment */
                                if (e_codes[iCentroid])
                                {
                                        fShapes->IPCoords(ip_centroid);
                                        centroid.AddScaled(ip_w*density, ip_centroid);
                                }
                        }
                        
                        /* kinetic energy/linear momentum */
                        if (e_codes[iKineticEnergy] || e_codes[iLinearMomentum])
                        {
                                /* velocity at integration point */
                                fShapes->InterpolateU(fLocVel, ip_velocity);
                                
                                /* kinetic energy */
                                if (e_codes[iKineticEnergy])
                                        kinetic_energy += 0.5*ip_w*density*dArrayT::Dot(ip_velocity, ip_velocity);
                                        
                                /* linear momentum */
                                if (e_codes[iLinearMomentum])
                                        linear_momentum.AddScaled(ip_w*density, ip_velocity);
                        }
                }

                /* copy in the cols */
                int colcount = 0;
                nodal_all.BlockColumnCopyAt(disp       , colcount); colcount += disp.MinorDim();
                nodal_all.BlockColumnCopyAt(coords     , colcount); colcount += coords.MinorDim();

                if (!qUseSimo)
                {
                  if (qNoExtrap)
                    {
                      double nip(fShapes->NumIP());
                      nodalstress /= nip;
                      princstress /= nip;
                      energy /= nip;
                      speed /= nip;
                      matdat /= nip;
                    }
                        nodal_all.BlockColumnCopyAt(nodalstress, colcount); colcount += nodalstress.MinorDim();
                        nodal_all.BlockColumnCopyAt(princstress, colcount); colcount += princstress.MinorDim();
                        nodal_all.BlockColumnCopyAt(energy     , colcount); colcount += energy.MinorDim();
                        nodal_all.BlockColumnCopyAt(speed      , colcount); colcount += speed.MinorDim();
                        nodal_all.BlockColumnCopyAt(matdat     , colcount); colcount += matdat.MinorDim();
                }
                else
                {        
                        colcount = 0;
                        simo_all.BlockColumnCopyAt(nodalstress, colcount); colcount += nodalstress.MinorDim();
                        simo_all.BlockColumnCopyAt(princstress, colcount); colcount += princstress.MinorDim();
                        simo_all.BlockColumnCopyAt(energy     , colcount); colcount += energy.MinorDim();
                        simo_all.BlockColumnCopyAt(speed      , colcount); colcount += speed.MinorDim();
                        simo_all.BlockColumnCopyAt(matdat     , colcount); colcount += matdat.MinorDim();
                        
                        iArrayT currIndices = CurrentElement().NodesX();
                        simo_force.Accumulate(currIndices,simo_all);
                        simo_mass.Accumulate(currIndices,simoNa_bar);
                        for (int i = 0; i < currIndices.Length(); i++)
                        	simo_counts[currIndices[i]]++;
                }
            
                /* accumulate - extrapolation done from ip's to corners => X nodes */
                ElementSupport().AssembleAverage(CurrentElement().NodesX(), nodal_all);
                
                /* element values */
                if (e_codes[iCentroid]) centroid /= mass;
                
                /* store results */
                e_values.SetRow(CurrElementNumber(), element_values);
        }

        /* get nodally averaged values */
        dArray2DT extrap_values;
        ElementSupport().OutputUsedAverage(extrap_values);

		int tmpDim = extrap_values.MajorDim();
        n_values.Dimension(tmpDim,n_out);
        n_values.BlockColumnCopyAt(extrap_values,0);
        if (qUseSimo)
        {        
        	int rowNum = 0;
        	iArrayT nodes_used(tmpDim);
        	dArray2DT tmp_simo(tmpDim, n_simo);
			for (int i = 0; i < simo_force.MajorDim(); i++)
				if (simo_counts[i] > 0)
				{
					nodes_used[rowNum] = i;
					simo_force.ScaleRow(i, 1./simo_mass(i,0));
					tmp_simo.SetRow(rowNum, simo_force(i));
					rowNum++;
		  		}
			
			/* collect final values */
			n_values.BlockColumnCopyAt(tmp_simo, simo_offset);
			
			/* write final values back into the averaging workspace */
			if (extrap_values.MinorDim() != n_values.MinorDim()) {
				ElementSupport().ResetAverage(n_values.MinorDim());
				ElementSupport().AssembleAverage(nodes_used, n_values);
			}
        }
}

/***********************************************************************
* Private
***********************************************************************/

/* construct output labels array */
void SolidElementT::GenerateOutputLabels(const iArrayT& n_codes, ArrayT<StringT>& n_labels, 
	const iArrayT& e_codes, ArrayT<StringT>& e_labels) const
{
	/* allocate */
	n_labels.Dimension(n_codes.Sum());

	int count = 0;
	if (n_codes[iNodalDisp])
	{
		/* labels from the field */
		const ArrayT<StringT>& labels = Field().Labels();
		for (int i = 0; i < labels.Length(); i++)
			n_labels[count++] = labels[i];
	}

	if (n_codes[iNodalCoord])
	{
		const char* xlabels[] = {"x1", "x2", "x3"};
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = xlabels[i];
	}

	if (n_codes[iNodalStress])
	{
		const char* slabels2D[] = {"s11", "s22", "s12"};
		const char* slabels3D[] = {"s11", "s22", "s33", "s23", "s13", "s12"};
		const char**    slabels = (NumSD() == 2) ? slabels2D : slabels3D;
		for (int i = 0; i < dSymMatrixT::NumValues(NumSD()); i++)
			n_labels[count++] = slabels[i];
	}
		
	if (n_codes[iPrincipal])
	{
		const char* plabels[] = {"s1", "s2", "s3"};
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = plabels[i];
	}
		
	if (n_codes[iEnergyDensity]) n_labels[count++] = "phi";
	if (n_codes[iWaveSpeeds])
	{
		const char* clabels2D[] = {"cd", "cs"};
		const char* clabels3D[] = {"cd", "cs_min", "cs_max"};
		const char**    clabels = (NumSD() == 2) ? clabels2D : clabels3D;
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = clabels[i];		
	}

	/* material output labels */
	if (n_codes[iMaterialData])
	{
		ArrayT<StringT> matlabels;
		(*fMaterialList)[0]->OutputLabels(matlabels);	
		
		for (int i = 0; i < matlabels.Length(); i++)
			n_labels[count++] = matlabels[i];
	}

	/* allocate */
	e_labels.Dimension(e_codes.Sum());
	count = 0;
	if (e_codes[iCentroid])
	{
		const char* xlabels[] = {"xc_1", "xc_2", "xc_3"};
		for (int i = 0; i < NumSD(); i++)
			e_labels[count++] = xlabels[i];
	}
	if (e_codes[iMass])
	{
		/* over integration points */
		for (int j = 0; j < NumIP(); j++)
		{
			StringT ip_label;
			ip_label.Append("ip", j+1);
			ip_label.Append(".mass");	
			e_labels[count++] = ip_label;
		}
	}
	if (e_codes[iStrainEnergy]) e_labels[count++] = "U";
	if (e_codes[iKineticEnergy]) e_labels[count++] = "T";
	if (e_codes[iLinearMomentum])
	{
		const char* plabels[] = {"L_X", "L_Y", "L_Z"};
		for (int i = 0; i < NumDOF(); i++)
			e_labels[count++] = plabels[i];
	}
	if (e_codes[iIPStress])
	{
		const char* slabels2D[] = {"s11", "s22", "s12","e11", "e22", "e12"};
		const char* slabels3D[] = {"s11", "s22", "s33", "s23", "s13", "s12", "e11", "e22", "e33", "e23", "e13", "e12"};
		const char**    slabels = (NumSD() == 2) ? slabels2D : slabels3D;

		/* over integration points */
		for (int j = 0; j < NumIP(); j++)
		{
			StringT ip_label;
			ip_label.Append("ip", j+1);
			
			/* over stress/strain components */
			for (int i = 0; i < 2*dSymMatrixT::NumValues(NumSD()); i++)
			{
				e_labels[count].Clear();
				e_labels[count].Append(ip_label, ".", slabels[i]);
				count++;
			}
		}		
	}

	/* material output labels */
	if (e_codes[iIPMaterialData])
	{
		ArrayT<StringT> matlabels;
		(*fMaterialList)[0]->OutputLabels(matlabels);	

		/* over integration points */
		for (int j = 0; j < NumIP(); j++)
		{
			StringT ip_label;
			ip_label.Append("ip", j+1);
			
			/* over stress components */
			for (int i = 0; i < matlabels.Length(); i++)
			{
				e_labels[count].Clear();
				e_labels[count].Append(ip_label, ".", matlabels[i]);
				count++;
			}
		}		
	}
}
