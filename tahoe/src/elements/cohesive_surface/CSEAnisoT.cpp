
/* $Id: CSEAnisoT.cpp,v 1.55.6.1 2003-11-04 19:47:08 bsun Exp $ */

/* created: paklein (11/19/1997) */
#include "CSEAnisoT.h"

#ifdef __DEVELOPMENT__
#include "DevelopmentElementsConfig.h"
#endif

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "fstreamT.h"
#include "toolboxConstants.h"
#include "SurfaceShapeT.h"
#include "SurfacePotentialT.h"
#ifndef _FRACTURE_INTERFACE_LIBRARY_
#include "eIntegratorT.h"
#include "NodeManagerT.h"
#endif
#include "ElementSupportT.h"
#include "dSymMatrixT.h"

/* potential functions */
#ifndef _FRACTURE_INTERFACE_LIBRARY_
#include "XuNeedleman2DT.h"
#include "TvergHutch2DT.h"
#include "ViscTvergHutch2DT.h"
#include "Tijssens2DT.h"
#include "RateDep2DT.h"
#include "TiedPotentialT.h"
#include "TiedPotentialBaseT.h"
#include "YoonAllen2DT.h"
#include "From2Dto3DT.h"
#include "TvergHutchRigid2DT.h"
#endif

#ifdef COHESIVE_SURFACE_ELEMENT_DEV
#include "InelasticDuctile2DT.h"
#include "InelasticDuctile_RP2DT.h"
#include "MR2DT.h"
#include "MR_RP2DT.h"
#endif

#include "TvergHutch3DT.h"
#include "YoonAllen3DT.h"
#include "XuNeedleman3DT.h"

using namespace Tahoe;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
/* constructor */
CSEAnisoT::CSEAnisoT(const ElementSupportT& support, const FieldT& field, bool rotate):
	CSEBaseT(support, field),
	fRotate(rotate),
	fCurrShapes(NULL),
	fQ(NumSD()),
	fdelta(NumSD()),
	fT(NumSD()),
	fddU(NumSD()),
	fRunState(support.RunState()),
	fIPArea(0.0)
{
	SetName("anisotropic_CSE");

	/* reset format for the element stiffness matrix */
	if (fRotate) fLHS.SetFormat(ElementMatrixT::kNonSymmetric);
}

/* constructor */
CSEAnisoT::CSEAnisoT(const ElementSupportT& support):
	CSEBaseT(support),
	fRotate(true),
	fCurrShapes(NULL),
	fRunState(support.RunState())
{
	SetName("anisotropic_CSE");
}
#else
CSEAnisoT::CSEAnisoT(ElementSupportT& support, bool rotate):
	CSEBaseT(support),
	fRotate(rotate),
	fCurrShapes(NULL),
	fQ(NumSD()),
	fdelta(NumSD()),
	fT(NumSD()),
	fddU(NumSD())
{
	SetName("anisotropic_CSE");

	/* reset format for the element stiffness matrix */
	if (fRotate) fLHS.SetFormat(ElementMatrixT::kNonSymmetric);
}
#endif

/* destructor */
CSEAnisoT::~CSEAnisoT(void)
{
	if (fRotate)
	{
		delete fCurrShapes;
		fCurrShapes = NULL;
	}
}

/* form of tangent matrix */
GlobalT::SystemTypeT CSEAnisoT::TangentType(void) const
{
	if (fRotate)
		/* tangent matrix is not symmetric */
		return GlobalT::kNonSymmetric;
	else
	{ // symmetric lest cohesive model says otherwise
		
		for (int i = 0; i < fSurfPots.Length(); i++)
			if (fSurfPots[i]->TangentType() == GlobalT::kNonSymmetric)
				return GlobalT::kNonSymmetric;
			
		return GlobalT::kSymmetric;
	}
}

void CSEAnisoT::Initialize(void)
{
	const char caller[] = "CSEAnisoT::Initialize";

	/* inherited */
	CSEBaseT::Initialize();
	
	/* rotating local frame */
	if (fRotate)
	{
		/* shape functions wrt. current coordinates (linked parent domains) */
		fCurrShapes = new SurfaceShapeT(*fShapes, fLocCurrCoords);
		if (!fCurrShapes) ExceptionT::OutOfMemory(caller);
		fCurrShapes->Initialize();
 		
		/* allocate work space */
		int nee = NumElementNodes()*NumDOF();
		fnsd_nee_1.Dimension(NumSD(), nee);
		fnsd_nee_2.Dimension(NumSD(), nee);
		fdQ.Dimension(NumSD());
		for (int k = 0; k < NumSD(); k++)
			fdQ[k].Dimension(NumSD(), nee);
	}
	else
		fCurrShapes = fShapes;

	/* streams */
	ifstreamT& in = ElementSupport().Input();
	ostream&   out = ElementSupport().Output();
#ifndef _FRACTURE_INTERFACE_LIBRARY_		
	fCalcNodalInfo = false;

	/* construct props */
	int numprops;
	in >> numprops;
	
	fTiedPots.Dimension(numprops);
	fTiedPots = NULL;
#else
	int numprops;
	numprops = 1;
	fCalcNodalInfo = false;
#endif

	fSurfPots.Dimension(numprops);
	fNumStateVariables.Dimension(numprops);

	for (int i = 0; i < fSurfPots.Length(); i++)
	{
		int num, code;
#ifndef _FRACTURE_INTERFACE_LIBRARY_		
		in >> num >> code;
#else
		num = 1; 
		code = ElementSupport().ReturnInputInt(ElementSupportT::kMaterialCode);
#endif
		num--;

		/* check for repeated number */
		if (fSurfPots[num] != NULL) ExceptionT::BadInputValue(caller, "surface property %d is already defined", num+1);

		switch (code)
		{
			case SurfacePotentialT::kXuNeedleman:
			{			
				if (NumDOF() == 2)
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new XuNeedleman2DT(in);			
#else
					throw ExceptionT::kBadInputValue;
#endif
				}
				else
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new From2Dto3DT(in, code, ElementSupport().TimeStep());
#else
					dArrayT *params = ElementSupport().FloatInput();
					fSurfPots[num] = new XuNeedleman3DT(*params);
#endif
				}
				break;
			}
			case SurfacePotentialT::kTvergaardHutchinson:
			{
				if (NumDOF() == 2)
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new TvergHutch2DT(in);
#else
					throw ExceptionT::kBadInputValue;
#endif
				}
				else
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new From2Dto3DT(in, code, ElementSupport().TimeStep());
#else
					dArrayT *params = ElementSupport().FloatInput();
					fSurfPots[num] = new TvergHutch3DT(*params);				
#endif
				}
				break;
			}
			
			case SurfacePotentialT::kTvergaardHutchinsonRigid:
			{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
				if (NumDOF() != 2) ExceptionT::GeneralFail();	
				fSurfPots[num] = new TvergHutchRigid2DT(in);
#else
				throw ExceptionT::kBadInputValue;
#endif
				break;
			}
#ifndef _FRACTURE_INTERFACE_LIBRARY_
			case SurfacePotentialT::kViscTvergaardHutchinson:
			{
				if (NumDOF() == 2)
					fSurfPots[num] = new ViscTvergHutch2DT(in, ElementSupport().TimeStep());
				else
					fSurfPots[num] = new From2Dto3DT(in, code, ElementSupport().TimeStep());
				break;
			}
			case SurfacePotentialT::kTijssens:
			{	
				if (NumDOF() == 2)
					fSurfPots[num] = new Tijssens2DT(in, ElementSupport().TimeStep());
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
			}
			case SurfacePotentialT::kRateDep:
			{	
				if (NumDOF() == 2)
					fSurfPots[num] = new RateDep2DT(in, ElementSupport().TimeStep());
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
			}
#endif
			case SurfacePotentialT::kYoonAllen:
			{	
				if (NumDOF() == 2)
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new YoonAllen2DT(in, ElementSupport().TimeStep());
#else
					throw ExceptionT::kBadInputValue;
#endif
				}
				else
				{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
					fSurfPots[num] = new From2Dto3DT(in, code, ElementSupport().TimeStep());
#else
					dArrayT *fparams = ElementSupport().FloatInput();
					iArrayT *iparams = ElementSupport().IntInput();
					fSurfPots[num] = new YoonAllen3DT(*fparams,*iparams,ElementSupport().TimeStep());
#endif
				}	
				break;
			}
			case SurfacePotentialT::kInelasticDuctile:
			{
#ifdef COHESIVE_SURFACE_ELEMENT_DEV
				if (NumDOF() == 2)
					fSurfPots[num] = new InelasticDuctile2DT(in, ElementSupport().TimeStep());
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT_DEV not enabled: %d", code);
#endif
			}
			case SurfacePotentialT::kInelasticDuctile_RP:
			{
#ifdef COHESIVE_SURFACE_ELEMENT_DEV
				if (NumDOF() == 2)
//					fSurfPots[num] = new InelasticDuctile_RP2DT(in, ElementSupport().TimeStep(), fIPArea);
					fSurfPots[num] = new InelasticDuctile_RP2DT(in, ElementSupport().TimeStep(), fIPArea, ElementSupport().Output());
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT_DEV not enabled: %d", code);
#endif
			}
			case SurfacePotentialT::kMR:
			{
#ifdef COHESIVE_SURFACE_ELEMENT_DEV
				if (NumDOF() == 2)
					fSurfPots[num] = new MR2DT(in);
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT_DEV not enabled: %d", code);
#endif
			}
#ifndef _FRACTURE_INTERFACE_LIBRARY_
			// Handle all tied potentials here till the code is finalized.
			case SurfacePotentialT::kTiedPotential:
			{	
				if (NumDOF() == 2)
					fSurfPots[num] = new TiedPotentialT(in);
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
				break;
			} 
			case SurfacePotentialT::kMR_RP:
			{
#ifdef COHESIVE_SURFACE_ELEMENT_DEV
				if (NumDOF() == 2)
					fSurfPots[num] = new MR_RP2DT(in);
				else
					ExceptionT::BadInputValue(caller, "potential not implemented for 3D: %d", code);
#else
				ExceptionT::BadInputValue(caller, "COHESIVE_SURFACE_ELEMENT_DEV not enabled: %d", code);
#endif
				break;
			}
#endif // ndef _FRACTURE_INTERFACE_LIBRARY_
			default:
				ExceptionT::BadInputValue(caller," unknown potential code %d \n",code);
		}
		if (!fSurfPots[num]) ExceptionT::OutOfMemory(caller);
		
		/* check for tied potential */
		SurfacePotentialT* surfpot = fSurfPots[num];
#ifndef _FRACTURE_INTERFACE_LIBRARY_
		TiedPotentialBaseT* tiedpot = dynamic_cast<TiedPotentialBaseT*>(surfpot);
		if (tiedpot)
		{
			/* only one tied potential allowed */
			if (freeNodeQ.Length() != 0) ExceptionT::GeneralFail(caller, "only 1 TiedNodes potential can be extant");
				
			/* common allocation for tied potentials */
			iTiedFlagIndex = tiedpot->TiedStatusPosition();
			freeNodeQ.Dimension(NumElements(),NumElementNodes());
			freeNodeQ = 0.;
			freeNodeQ_last = freeNodeQ;
					 
			/* initialize things if a potential needs more info than the gap vector */
			if (tiedpot->NeedsNodalInfo()) {
				fCalcNodalInfo = true;
				fNodalInfoCode = tiedpot->NodalQuantityNeeded();
				iBulkGroups = tiedpot->BulkGroups();
			}
			
			/* re-tying */
			if (tiedpot->NodesMayRetie())
				qRetieNodes = true;
			else 
				qRetieNodes = false;
				
			/* utility array for stress smoothing over tied node pairs */
			otherInds.Dimension(NumElementNodes());
			if (NumSD() == 2)
			{
				otherInds[0] = 3; otherInds[1] = 2; otherInds[2] = 1; otherInds[3] = 0;
			} 
			else if (NumSD() == 3)
			{
				otherInds[0] = 4; otherInds[1] = 5; otherInds[2] = 6; otherInds[3] = 7;
				otherInds[4] = 0; otherInds[5] = 1; otherInds[6] = 2; otherInds[7] = 3;
			}
			else 
				ExceptionT::GeneralFail(caller, "tied nodes for 2D/3D only not %dD", NumSD());

			/* store */
			fTiedPots[num] = tiedpot;
		}
#endif		
		
		/* get number of state variables */
		fNumStateVariables[num] = fSurfPots[num]->NumStateVariables(); 
	}

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* check compatibility of constitutive outputs */
	if (fSurfPots.Length() > 1 && fNodalOutputCodes[MaterialData])
		for (int k = 0; k < fSurfPots.Length(); k++)
		{
			const SurfacePotentialT* pot_k = fSurfPots[k];
			for (int i = k+1; i < fSurfPots.Length(); i++)
			{
				const SurfacePotentialT* pot_i = fSurfPots[i];
				if (!SurfacePotentialT::CompatibleOutput(*pot_k, *pot_i))
				{
					cout << "\n CSEAnisoT::Initialize: incompatible output between potentials\n"
					     <<   "     " << k+1 << " and " << i+1 << endl;
					throw ExceptionT::kBadInputValue;
				}
			}
		}
			
	/* write */
	out << " Rotating local coordinate frame . . . . . . . . = " <<
	    ((fRotate) ? "ACTIVE" : "INACTIVE") << '\n';
	out << "\n Cohesive surface potentials:\n";
	out << " Number of potentials. . . . . . . . . . . . . . = ";
	out << fSurfPots.Length() << '\n';
	for (int j = 0; j < fSurfPots.Length(); j++)
	{
		out << "\n Potential number. . . . . . . . . . . . . . . . = " << j + 1 << '\n';
		out << " Potential name:\n";
		fSurfPots[j]->PrintName(out);
		fSurfPots[j]->Print(out);
	}
#endif
	
	/* initialize state variable space */
	if (fNumStateVariables.Min() > 0)
	{
		/* number of integration points */
		int num_ip = fCurrShapes->NumIP();
	
		/* get state variables per element */
		int num_elements = fElementCards.Length();
		iArrayT num_elem_state(num_elements);
		for (int i = 0; i < num_elements; i++)
			num_elem_state[i] = num_ip*fNumStateVariables[fElementCards[i].MaterialNumber()];

#ifndef _FRACTURE_INTERFACE_LIBRARY_
		/* allocate space */
		fStateVariables.Configure(num_elem_state);
#else
		fStateVariables.Set(1,num_elem_state[0],ElementSupport().StateVariableArray());
#endif

		/* initialize state variable space */
		dArrayT state;
		for (int i = 0; i < num_elements; i++)
		{
			/* material number */
			int mat_num = fElementCards[i].MaterialNumber();
			int num_var = fNumStateVariables[mat_num];
			
			/* loop over integration points */
			double* pstate = fStateVariables(i);
			for (int j = 0; j < num_ip; j++)
			{
				state.Set(num_var, pstate);
				fSurfPots[mat_num]->InitStateVariables(state);
				pstate += num_var;
			}
		}		
	}
	else /* set dimensions to zero */
		fStateVariables.Dimension(fElementCards.Length(), 0);

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* set history */
	fStateVariables_last = fStateVariables;
	/* For SIERRA, don't do anything. Wait until InitStep. */
#endif

}

#ifdef _FRACTURE_INTERFACE_LIBRARY_	
/* Get state variables from ElementSupportT here */
void CSEAnisoT::InitStep(void) 
{
	fStateVariables_last.Set(fStateVariables.MajorDim(),fStateVariables.MinorDim(),
		ElementSupport().StateVariableArray());
};
#endif

/* close current time increment */
void CSEAnisoT::CloseStep(void)
{
	/* inherited */
	CSEBaseT::CloseStep();

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* update flags */
	if (freeNodeQ.IsAllocated())
	{
		int nel = freeNodeQ.MajorDim();
		int nen = freeNodeQ.MinorDim();
		for (int e = 0; e < nel; e++)
		{
			double* p = freeNodeQ(e);
			double* p_last = freeNodeQ_last(e);
			int count_Free = 0;
			int count_Tied = 0;
			for (int n = 0; n < nen; n++)
			{
				double change = *p++ - *p_last++;
				if (change > 0.5)
					count_Free++;
				else if (change < -0.5)
					count_Tied++;
			}
			
			/* conflict */
			if (count_Free > 0 && count_Tied > 0)
				ExceptionT::GeneralFail("CSEAnisoT::CloseStep", 
					"conflicting status changes in element %d", e+1);
					
			/* change state flag for all integration points */
			if (count_Free > 0 || count_Tied > 0)
			{
				const ElementCardT& element = ElementCard(e);
				int mat_num = element.MaterialNumber();
				TiedPotentialBaseT* tiedpot = fTiedPots[mat_num];
				if (tiedpot)
				{
					SurfacePotentialT* surfpot = fSurfPots[mat_num];
					int num_state = surfpot->NumStateVariables();
					int stat_dex = tiedpot->TiedStatusPosition();
			
					double new_status = (count_Free > 0) ? TiedPotentialBaseT::kFreeNode : TiedPotentialBaseT::kTiedNode;
					double *pstate = fStateVariables(e);
					
					int nip = fShapes->NumIP();
					for (int ip = 0; ip < nip; ip++)
					{
						pstate[stat_dex] = new_status;
						pstate += num_state;
					}
				}
			}
		}

		/* update history */
		freeNodeQ_last = freeNodeQ;
	}
#endif /* _FRACTURE_INTERFACE_LIBRARY__	*/
	
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* store history */
	fStateVariables_last = fStateVariables;
#endif /* _FRACTURE_INTERFACE_LIBRARY_ */
}

#ifndef _FRACTURE_INTERFACE_LIBRARY_
/* write restart data to the output stream. */
void CSEAnisoT::WriteRestart(ostream& out) const
{
	/* inherited */
	CSEBaseT::WriteRestart(out);
	
	/* write state variable data */
	fStateVariables.WriteData(out);
	out << '\n';
	
	out << freeNodeQ.Length() << '\n';
	out << freeNodeQ.wrap_tight(10) << endl;
}

/* read restart data to the output stream */
void CSEAnisoT::ReadRestart(istream& in)
{
	/* inherited */
	CSEBaseT::ReadRestart(in);

	/* read state variable data */
	fStateVariables.ReadData(in);

	/* set history */
	fStateVariables_last = fStateVariables;
	
	int freeNode_length;
	in >> freeNode_length;
	if (freeNodeQ.Length() != freeNode_length)
		ExceptionT::GeneralFail("CSEAnisoT::ReadRestart","Length mismatch for freeNodeQ");
	in >> freeNodeQ;
	freeNodeQ_last = freeNodeQ;
}

#else

void CSEAnisoT::WriteRestart(double* outgoingData) const
{
	/* inherited */
	CSEBaseT::WriteRestart(outgoingData);

	// Nothing to do here right now since Sierra controls state variables	
	/* write state variable data */
//	fStateVariables.WriteData(out);

}

/* read restart data to the output stream */
void CSEAnisoT::ReadRestart(double* incomingData)
{
	/* inherited */
	CSEBaseT::ReadRestart(incomingData);

	// Nothing to do here right now since Sierra controls state variables	
	
	/* read state variable data */
//	fStateVariables.ReadData(in);

	/* set history */
//	fStateVariables_last = fStateVariables;
//	if (freeNodeQ.IsAllocated()) //This is useless
//		freeNodeQ_last = freeNodeQ;
}
#endif

/* describe the parameters needed by the interface */
void CSEAnisoT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	CSEBaseT::DefineParameters(list);

	ParameterT rotate_frame(ParameterT::Boolean, "rotate_frame");
	rotate_frame.SetDefault(true);
	list.AddParameter(rotate_frame);
}

/***********************************************************************
 * Protected
 ***********************************************************************/

void CSEAnisoT::LHSDriver(GlobalT::SystemTypeT)
{
	const char caller[] = "CSEAnisoT::LHSDriver";

	/* matrix format */
	dMatrixT::SymmetryFlagT format = (fRotate) ?
		dMatrixT::kWhole : dMatrixT::kUpperOnly;

	/* time-integration parameters */
	double constK = 1.0;
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	int formK = fIntegrator->FormK(constK);
	if (!formK) return;
#endif

	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);

	/* work space for collecting element variables */
	LocalArrayT nodal_values(LocalArrayT::kUnspecified);
	dArray2DT elementVals;
	dArrayT localFrameIP;
	iArrayT ndIndices;

	AutoArrayT<double> state2;
	dArrayT state;
	Top();
	while (NextElement())
	{
		/* current element */
		const ElementCardT& element = CurrentElement();
	
		/* surface potential */
		SurfacePotentialT* surfpot = fSurfPots[element.MaterialNumber()];
		int num_state = fNumStateVariables[element.MaterialNumber()];
		state2.Dimension(num_state);
#ifndef _FRACTURE_INTERFACE_LIBRARY_
		TiedPotentialBaseT* tiedpot = fTiedPots[element.MaterialNumber()];
#endif

		/* get ref geometry (1st facet only) */
		fNodes1.Collect(facet1, element.NodesX());
		fLocInitCoords1.SetLocal(fNodes1);

		/* get current geometry */
		SetLocalX(fLocCurrCoords); //EFFECTIVE_DVA
		
		/* initialize */
		fLHS = 0.0;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
		/* Get local bulk values for CSE*/
		if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 
			SurfaceValuesFromBulk(element, ndIndices, elementVals, nodal_values);
#endif

		/* loop over integration points */
		double* pstate = fStateVariables(CurrElementNumber());
		fShapes->TopIP();
		while (fShapes->NextIP())
		{  
			/* set state variables */
			state.Set(num_state, pstate);
			pstate += num_state;
		
			/* integration weights */
			double w = fShapes->IPWeight();		

			/* coordinate transformations */
			double j0, j;
			if (fRotate)
			{
				j0 = fShapes->Jacobian();
				j  = fCurrShapes->Jacobian(fQ, fdQ);
			}
			else
				j0 = j = fShapes->Jacobian(fQ);
			fIPArea = w*j0;

			/* check */
			if (j0 <= 0.0 || j <= 0.0) ExceptionT::BadJacobianDet(caller);
		
			/* gap vector and gradient (facet1 to facet2) */
			const dArrayT&    delta = fShapes->InterpolateJumpU(fLocCurrCoords);
			const dMatrixT& d_delta = fShapes->Grad_d();

			/* gap vector in local frame */
			fQ.MultTx(delta, fdelta);			
			
#ifndef _FRACTURE_INTERFACE_LIBRARY_
			/* Interpolate nodal info to IPs using stress tensor in local frame*/
			if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 
				FromNodesToIPs(tiedpot->RotateNodalQuantity(), localFrameIP, nodal_values);
#endif

			/* stiffness in local frame */
			const dMatrixT& K = surfpot->Stiffness(fdelta, state, localFrameIP);
			
			/* rotation */
			if (fRotate)
			{
				/* traction in local frame */
				const dArrayT& T = surfpot->Traction(fdelta, state, localFrameIP, false);

				/* 1st term */
				fT.SetToScaled(j0*w*constK, T);
				Q_ijk__u_j(fdQ, fT, fnsd_nee_1);
				fNEEmat.MultATB(d_delta, fnsd_nee_1);
				fLHS += fNEEmat;

				/* 2nd term */
				fddU.SetToScaled(j0*w*constK, K);
				fnsd_nee_1.MultATB(fQ, d_delta);
				fnsd_nee_2.MultATB(fddU, fnsd_nee_1);
				u_i__Q_ijk(delta, fdQ, fnsd_nee_1);
				fNEEmat.MultATB(fnsd_nee_2, fnsd_nee_1);
				fLHS += fNEEmat;
			}
			
			/* 3rd term */
			fddU.MultQBQT(fQ, K);
			fddU *= j0*w*constK;
			fLHS.MultQTBQ(d_delta, fddU, format, dMatrixT::kAccumulate);
		}

		/* assemble */
		AssembleLHS();
	}
}

void CSEAnisoT::RHSDriver(void)
{
	const char caller[] = "CSEAnisoT::RHSDriver";

	/* time-integration parameters */
	double constKd = 1.0;

#ifndef _FRACTURE_INTERFACE_LIBRARY_

	int formKd = fIntegrator->FormKd(constKd);
	if (!formKd) return;

	/* heat source if needed */
	const FieldT* temperature = ElementSupport().Field("temperature");

	/* initialize sources */
	if (temperature) 
		InitializeTemperature(temperature);

#else // _FRACTURE_INTERFACE_LIBRARY_ defined

    /*Read in SIERRA's new state variables. We need their memory. */	
	fStateVariables.Set(fStateVariables.MajorDim(),fStateVariables.MinorDim(),
		ElementSupport().StateVariableArray());

#endif

	/* set state to start of current step */
	fStateVariables = fStateVariables_last;
	if (freeNodeQ.IsAllocated())
		freeNodeQ = freeNodeQ_last;

	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);
	
	/* If potential needs info from nodes, start to gather it now */
	if (fCalcNodalInfo)
		StoreBulkOutput();

	/* fracture surface area */
	fFractureArea = 0.0;

	/* work space for collecting element variables */
	LocalArrayT nodal_values(LocalArrayT::kUnspecified);
	dArray2DT elementVals;
	dArrayT localFrameIP;
	iArrayT ndIndices;

	int block_count = 0, block_dex = 0;
	dArrayT state;
	Top();
	while (NextElement())
	{
		/* advance to block (skip empty blocks) */
		while (block_count == fBlockData[block_dex].Dimension()) {
			block_count = 0;
			block_dex++;		
		}

		/* current element */
		ElementCardT& element = CurrentElement();
	
		/* get ref geometry (1st facet only) */
		fNodes1.Collect(facet1, element.NodesX());
		fLocInitCoords1.SetLocal(fNodes1);
	  			
		if (element.Flag() != kOFF)
		{
			/* surface potential */
			SurfacePotentialT* surfpot = fSurfPots[element.MaterialNumber()];
			int num_state = fNumStateVariables[element.MaterialNumber()];
	
			/* get current geometry */
			SetLocalX(fLocCurrCoords);
	
	  		/* initialize */
	  		fRHS = 0.0;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
			TiedPotentialBaseT* tiedpot = fTiedPots[element.MaterialNumber()];

			/* Get local bulk values for CSE*/
			if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 	
				SurfaceValuesFromBulk(element, ndIndices, elementVals, nodal_values);
#endif
			
			/* loop over integration points */
			double* pstate = fStateVariables(CurrElementNumber());
			int all_failed = 1;
			fShapes->TopIP();
			while (fShapes->NextIP())
			{
				/* set state variables */
				state.Set(num_state, pstate);
				pstate += num_state;
			
				/* integration weights */
				double w = fShapes->IPWeight();		

				/* coordinate transformations */
				double j0, j;
				if (fRotate)
				{
					j0 = fShapes->Jacobian();
					j  = fCurrShapes->Jacobian(fQ);
				}
				else
					j0 = j = fShapes->Jacobian(fQ);
				fIPArea = w*j0;

				/* check */
				if (j0 <= 0.0 || j <= 0.0) ExceptionT::BadJacobianDet(caller);
	
				/* gap vector from facet1 to facet2 */
				const dArrayT& delta = fShapes->InterpolateJumpU(fLocCurrCoords);
	
				/* gap vector in local frame */
				fQ.MultTx(delta, fdelta);

#ifndef _FRACTURE_INTERFACE_LIBRARY_					
				/* Interpolate nodal info to IPs using stress tensor in local frame*/
				if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 
				{
					FromNodesToIPs(tiedpot->RotateNodalQuantity(), localFrameIP, nodal_values);
					UntieOrRetieNodes(CurrElementNumber(), ndIndices.Length(), 
									tiedpot, state, localFrameIP);
				}
#endif
				/* traction vector in/out of local frame */
				fQ.Multx(surfpot->Traction(fdelta, state, localFrameIP, true), fT);
				
				/* expand */
				fShapes->Grad_d().MultTx(fT, fNEEvec);
	
				/* accumulate */
				fRHS.AddScaled(-j0*w*constKd, fNEEvec);
				
				/* check status */
				SurfacePotentialT::StatusT status = surfpot->Status(fdelta, state);
				if (status != SurfacePotentialT::Failed) all_failed = 0;
				
				/* fracture area */
				if (fOutputArea && status != SurfacePotentialT::Precritical)
					fFractureArea += j0*w;
					
#ifndef _FRACTURE_INTERFACE_LIBRARY_
				/* incremental heat */
				if (temperature) 
					fIncrementalHeat[block_dex](block_count, fShapes->CurrIP()) = 
						surfpot->IncrementalHeat(fdelta, state);
#endif
			}

			/* assemble */
			AssembleRHS();
			
			/* mark elements */
			if (all_failed)
			{
				int& flag = element.Flag();
				if (flag == kON) flag = kMarked;
			}
		}
		else if (fOutputArea)
		{
			/* integrate fracture area */
			fShapes->TopIP();
			while (fShapes->NextIP())
				fFractureArea += (fShapes->Jacobian())*(fShapes->IPWeight());
		}

		/* next in block */
		block_count++;
	}
}

/* nodal value calculations */
void CSEAnisoT::SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* inherited */
	CSEBaseT::SetNodalOutputCodes(mode, flags, counts);

	/* resize for vectors not magnitudes */
	if (flags[NodalDispJump] == mode)
		counts[NodalDispJump] = NumDOF();	
	if (flags[NodalTraction] == mode)
		counts[NodalTraction] = NumDOF();
	if (flags[MaterialData] == mode)
		counts[MaterialData] = fSurfPots[0]->NumOutputVariables();
}

void CSEAnisoT::SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
	iArrayT& counts) const
{
	/* inherited */
	CSEBaseT::SetElementOutputCodes(mode, flags, counts);
	
	/* resize for vectors not magnitudes */
	if (flags[Traction] == mode) counts[Traction] = NumDOF();
}

void CSEAnisoT::SendOutput(int kincode)
{
	if (kincode != InternalData)
		CSEBaseT::SendOutput(kincode);
	else // TiedNodesT wants its freeNode info
		ComputeFreeNodesForOutput();
}

/* set the active elements */
void CSEAnisoT::SetStatus(const ArrayT<StatusT>& status)
{
	/* work space */
	dArrayT state;
	dArrayT t_in;
	iArrayT facet1;

	/* loop over elements and initial state variables */
	for (int i = 0; i < fElementCards.Length(); i++)
	{
		/* current element */
		ElementCardT& element = fElementCards[i];
		int& flag = element.Flag();
		flag = status[i];
		if (flag == kMarkON)
		{		
			/* surface potential */
			SurfacePotentialT* surfpot = fSurfPots[element.MaterialNumber()];
			int num_state = fNumStateVariables[element.MaterialNumber()];
#ifndef _FRACTURE_INTERFACE_LIBRARY_
			TvergHutchRigid2DT* surfpot_rot = dynamic_cast<TvergHutchRigid2DT*>(surfpot);
#else
			SurfacePotentialT* surfpot_rot = NULL;
#endif
	
			/* initialize state variables by rotating some values by Q */
			if (surfpot_rot)
			{
				/* get geometry */
				fNodes1.Collect(facet1, element.NodesX());
				fLocInitCoords1.SetLocal(fNodes1);
				fLocCurrCoords.SetLocal(element.NodesX());

				/* loop over integration points */
				double* pstate = fStateVariables(i);
				fShapes->TopIP();
				while (fShapes->NextIP())
				{
					/* set state variables */
					state.Set(num_state, pstate);
					pstate += num_state;

					/* rotate  */
					if (fRotate)
					{
						/* coordinate transformation */
						fCurrShapes->Jacobian(fQ);
					
						/* gap vector in local frame */
						fQ.MultTx(state, t_in);
						state = t_in;
					}
		
					/* initialize the state variables */
					surfpot->InitStateVariables(state);
				}
			}
			flag = kON;
		}
		else if (flag == kMarkOFF)
			flag = kOFF;
	}
}

/* extrapolate the integration point stresses and strains and extrapolate */
void CSEAnisoT::ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	const iArrayT& e_codes, dArray2DT& e_values)
{      

	/* number of output values */
	int n_out = n_codes.Sum();
	int e_out = e_codes.Sum();

	/* nothing to output */
	if (n_out == 0 && e_out == 0) return;

	/* dimensions */
	int  nsd = NumSD();
	int ndof = NumDOF();
	int  nen = NumElementNodes();

	/* reset averaging workspace */
	ElementSupport().ResetAverage(n_out);

	/* allocate element results space */
	e_values.Dimension(NumElements(), e_out);
	e_values = 0.0;

	/* work arrays */
	dArray2DT nodal_space(nen, n_out);
	dArray2DT nodal_all(nen, n_out);
	dArray2DT coords, disp;
	dArray2DT jump, T;
	dArray2DT matdat;	

	/* ip values */
	LocalArrayT loc_init_coords(LocalArrayT::kInitCoords, nen, nsd);
	LocalArrayT loc_disp(LocalArrayT::kDisp, nen, ndof);
	ElementSupport().RegisterCoordinates(loc_init_coords);
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	Field().RegisterLocal(loc_disp);
#else
#pragma message("This routine needs displacements from SIERRA.")
	loc_disp.SetGlobal(ElementSupport().CurrentCoordinates());
#endif
	dArrayT ipmat(n_codes[MaterialData]);
	
	/* set shallow copies */
	double* pall = nodal_space.Pointer();
	coords.Set(nen, n_codes[NodalCoord], pall) ; pall += coords.Length();
	disp.Set(nen, n_codes[NodalDisp], pall)    ; pall += disp.Length();
	jump.Set(nen, n_codes[NodalDispJump], pall); pall += jump.Length();
	T.Set(nen, n_codes[NodalTraction], pall)   ; pall += T.Length();
	matdat.Set(nen, n_codes[MaterialData], pall);

	/* element work arrays */
	dArrayT element_values(e_values.MinorDim());
	pall = element_values.Pointer();
	dArrayT centroid;
	if (e_codes[Centroid])
	{
		centroid.Set(nsd, pall); 
		pall += nsd;
	}
	double phi_tmp, area;
	double& phi = (e_codes[CohesiveEnergy]) ? *pall++ : phi_tmp;
	dArrayT traction;
	if (e_codes[Traction])
	{
		traction.Set(ndof, pall); 
		pall += ndof;
	}

	/* node map of facet 1 */
	iArrayT facet1;
	(fShapes->NodesOnFacets()).RowAlias(0, facet1);

	dArray2DT elementVals;
	LocalArrayT nodal_values(LocalArrayT::kUnspecified);
	dArrayT localFrameIP;
	iArrayT ndIndices;

	AutoArrayT<double> state;
	Top();
	while (NextElement())
	{
		/* current element */
		ElementCardT& element = CurrentElement();
		
		/* initialize */
		nodal_space = 0.0;
		element_values = 0.0;

		/* coordinates for whole element */
		if (n_codes[NodalCoord])
		{
			SetLocalX(loc_init_coords);
			loc_init_coords.ReturnTranspose(coords);
		}
		
		/* displacements for whole element */
		if (n_codes[NodalDisp])
		{
			SetLocalU(loc_disp);
			loc_disp.ReturnTranspose(disp);
		}
		
		//NOTE: will not get any element output if the element not kON
		//      although quantities like the reference element centroid could
		//      still be safely calculated.
		/* compute output */
		if (element.Flag() == kON)
		{
	  		/* surface potential */
			SurfacePotentialT* surfpot = fSurfPots[element.MaterialNumber()];
			int num_state = fNumStateVariables[element.MaterialNumber()];
			state.Dimension(num_state);

			/* get ref geometry (1st facet only) */
			fNodes1.Collect(facet1, element.NodesX());
			fLocInitCoords1.SetLocal(fNodes1);

			/* get current geometry */
			SetLocalX(fLocCurrCoords); //EFFECTIVE_DVA

			/* initialize element values */
			phi = area = 0.0;
			if (e_codes[Centroid]) centroid = 0.0;
			if (e_codes[Traction]) traction = 0.0;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
			TiedPotentialBaseT* tiedpot = fTiedPots[element.MaterialNumber()];
			
			/* Get local bulk values for CSE*/
			if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 	
				SurfaceValuesFromBulk(element, ndIndices, elementVals, nodal_values);
#endif

			/* integrate */
			fShapes->TopIP();
			while (fShapes->NextIP())
			{
				double* pstate = fStateVariables(CurrElementNumber()) + 
					fShapes->CurrIP()*num_state;
			
				/* element integration weight */
				double ip_w = fShapes->Jacobian()*fShapes->IPWeight();
				area += ip_w;

				/* gap */
				const dArrayT& gap = fShapes->InterpolateJumpU(fLocCurrCoords);

				/* coordinate transformation */
				double j = fCurrShapes->Jacobian(fQ);
				fQ.MultTx(gap, fdelta);
				
				/* gap */				
				if (n_codes[NodalDispJump])
					fShapes->Extrapolate(fdelta, jump);
	     
				/* traction */
				if (n_codes[NodalTraction] || e_codes[Traction])
				{
					/* copy state variables (not integrated) */
					state.Set(num_state,pstate);

#ifndef _FRACTURE_INTERFACE_LIBRARY_
					/* Interpolate nodal info to IPs using stress tensor in local frame*/
					if (tiedpot != NULL && tiedpot->NeedsNodalInfo()) 
						FromNodesToIPs(tiedpot->RotateNodalQuantity(), localFrameIP, nodal_values);			
#endif

					/* compute traction in local frame */
					const dArrayT& tract = surfpot->Traction(fdelta, state, localFrameIP, false);
				       
					/* project to nodes */
					if (n_codes[NodalTraction])
						fShapes->Extrapolate(tract, T);
					
					/* element average */
					if (e_codes[Traction])
						traction.AddScaled(ip_w, tract);
				}
					
				/* material output data */
				if (n_codes[MaterialData])
				{
					/* evaluate */
					surfpot->ComputeOutput(fdelta, state, ipmat);
					fShapes->Extrapolate(ipmat, matdat);
				}

				/* moment */
				if (e_codes[Centroid])
					centroid.AddScaled(ip_w, fShapes->IPCoords());
				
				/* cohesive energy */
				if (e_codes[CohesiveEnergy])
				{
					/* copy state variables (not integrated) */
					state.Copy(pstate);

					/* surface potential */
					double potential = surfpot->Potential(fdelta, state);

					/* integrate */
					phi += potential*ip_w;
				}
			}
			
			/* element values */
			if (e_codes[Centroid]) centroid /= area;
			if (e_codes[Traction]) traction /= area;
		}
		/* element has failed */
		else
		{
			/* can still be calculated */
			if (e_codes[Centroid] || e_codes[CohesiveEnergy])
			{
				/* get ref geometry (1st facet only) */
				fNodes1.Collect(facet1, element.NodesX());
				fLocInitCoords1.SetLocal(fNodes1);

				/* initialize element values */
				phi = area = 0.0;
				if (e_codes[Centroid]) centroid = 0.0;
		
		  		/* surface potential */
				SurfacePotentialT* surfpot = fSurfPots[element.MaterialNumber()];
				int num_state = fNumStateVariables[element.MaterialNumber()];
				state.Dimension(num_state);
		
				/* integrate */
				fShapes->TopIP();
				while (fShapes->NextIP())
				{

					double* pstate = fStateVariables_last(CurrElementNumber()) + fShapes->CurrIP()*num_state;
					/* element integration weight */
					double ip_w = fShapes->Jacobian()*fShapes->IPWeight();
					area += ip_w;
		
					/* moment */
					if (e_codes[Centroid])
						centroid.AddScaled(ip_w, fShapes->IPCoords());

					/* cohesive energy */
					if (e_codes[CohesiveEnergy])
					  {  
						/* surface potential */
						state.Copy(pstate);
						double potential = surfpot->FractureEnergy(state);
	
						/* integrate */
						phi += potential*ip_w;
					}
				}
				
				/* element values */
				if (e_codes[Centroid]) centroid /= area;
			}		
		}

		/* copy in the cols (in sequence of output) */
		int colcount = 0;
		nodal_all.BlockColumnCopyAt(disp  , colcount); colcount += disp.MinorDim();
		nodal_all.BlockColumnCopyAt(coords, colcount); colcount += coords.MinorDim();
		nodal_all.BlockColumnCopyAt(jump  , colcount); colcount += jump.MinorDim();
		nodal_all.BlockColumnCopyAt(T     , colcount); colcount += T.MinorDim();
		nodal_all.BlockColumnCopyAt(matdat, colcount);

		/* accumulate - extrapolation done from ip's to corners => X nodes */
		ElementSupport().AssembleAverage(element.NodesX(), nodal_all);
		
		/* store results */
		e_values.SetRow(CurrElementNumber(), element_values);	      
	}

	/* get nodally averaged values */
	ElementSupport().OutputUsedAverage(n_values);
}

void CSEAnisoT::GenerateOutputLabels(const iArrayT& n_codes, ArrayT<StringT>& n_labels,
	const iArrayT& e_codes, ArrayT<StringT>& e_labels) const
{
	/* inherited */
	CSEBaseT::GenerateOutputLabels(n_codes, n_labels, e_codes, e_labels);

	/* overwrite nodal labels */
	n_labels.Dimension(n_codes.Sum());
	int count = 0;
	if (n_codes[NodalDisp])
	{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
		/* labels from the field */
		const ArrayT<StringT>& labels = Field().Labels();
		for (int i = 0; i < labels.Length(); i++)
			n_labels[count++] = labels[i];
#else
		const char* labels[] = {"D_1", "D_2", "D_3"};
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = labels[i];
#endif
	}

	if (n_codes[NodalCoord])
	{
		const char* xlabels[3] = {"x1", "x2", "x3"};
		for (int i = 0; i < NumSD(); i++)
			n_labels[count++] = xlabels[i];
	}

	if (n_codes[NodalDispJump])
	{
		const char* d_2D[2] = {"d_t", "d_n"};
		const char* d_3D[3] = {"d_t1", "d_t2", "d_n"};
		const char** dlabels;
		if (NumDOF() == 2)
			dlabels = d_2D;
		else if (NumDOF() == 3)
			dlabels = d_3D;
		else
			throw ExceptionT::kGeneralFail;

		for (int i = 0; i < NumDOF(); i++)
			n_labels[count++] = dlabels[i];
	}

	if (n_codes[NodalTraction])
	{
		const char* t_2D[2] = {"T_t", "T_n"};
		const char* t_3D[3] = {"T_t1", "T_t2", "T_n"};
		const char** tlabels;
		if (NumDOF() == 2)
			tlabels = t_2D;
		else if (NumDOF() == 3)
			tlabels = t_3D;
		else
			throw ExceptionT::kGeneralFail;

		for (int i = 0; i < NumDOF(); i++)
			n_labels[count++] = tlabels[i];
	}

	/* material output labels */
	if (n_codes[MaterialData])
	{
		ArrayT<StringT> matlabels;
		fSurfPots[0]->OutputLabels(matlabels);
		for (int i = 0; i < n_codes[MaterialData]; i++)
			n_labels[count++] = matlabels[i];
	}
	
	/* allocate nodal output labels */
	e_labels.Dimension(e_codes.Sum());
	count = 0;
	if (e_codes[Centroid])
	{
		const char* xlabels[] = {"xc_1", "xc_2", "xc_3"};
		for (int i = 0; i < NumSD(); i++)
			e_labels[count++] = xlabels[i];
	}
	if (e_codes[CohesiveEnergy]) e_labels[count++] = "phi";
	if (e_codes[Traction])
	{
		const char* t_2D[2] = {"T_t", "T_n"};
		const char* t_3D[3] = {"T_t1", "T_t2", "T_n"};
		const char** tlabels;
		if (NumDOF() == 2)
			tlabels = t_2D;
		else if (NumDOF() == 3)
			tlabels = t_3D;
		else
			throw ExceptionT::kGeneralFail;

		for (int i = 0; i < NumDOF(); i++)
			e_labels[count++] = tlabels[i];
	}
}

/* write all current element information to the stream */
void CSEAnisoT::CurrElementInfo(ostream& out) const
{
#pragma unused(out)
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* inherited */
	CSEBaseT::CurrElementInfo(out);
	
	/* current element configuration */
	out << " current integration point:" << fCurrShapes->CurrIP() << '\n';
	try
	{
		dMatrixT Qtemp(fQ);
		double j = fCurrShapes->Jacobian(Qtemp);
		out << " surface jacobian = " << j << '\n';
		out << " coordinate transformation:\n";
		out << fQ << '\n';
		out << " gap vector (global frame):\n";
		const dArrayT& delta = fShapes->InterpolateJumpU(fLocCurrCoords);
		out << delta << '\n';
		out << " gap vector (local frame):\n";
		dArrayT delta_temp(fdelta);
		fQ.MultTx(delta, delta_temp);
		out << delta_temp << '\n';	
	}
	
	catch (ExceptionT::CodeT error)
	{
		out << " CSEAnisoT::CurrElementInfo: error on surface jacobian\n";
	}
#else
	throw ExceptionT::kGeneralFail;
#endif
}

/***********************************************************************
* Private
***********************************************************************/

/* operations with pseudo rank 3 (list in j) matrices */
void CSEAnisoT::u_i__Q_ijk(const dArrayT& u, const ArrayT<dMatrixT>& Q,
	dMatrixT& Qu)
{
	for (int i = 0; i < u.Length(); i++)
	{	
		Q[i].MultTx(u, fNEEvec);
		Qu.SetRow(i, fNEEvec);
	}
}

void CSEAnisoT::Q_ijk__u_j(const ArrayT<dMatrixT>& Q, const dArrayT& u,
	dMatrixT& Qu)
{
	if (Q.Length() == 2)
		Qu.SetToCombination(u[0], Q[0], u[1], Q[1]);
	else if (Q.Length() == 3)
		Qu.SetToCombination(u[0], Q[0], u[1], Q[1], u[2], Q[2]);
	else
		throw ExceptionT::kGeneralFail;
}

/* Auxiliary routines to interface with cohesive models requiring more input
 * than the gap vector.
 */
void CSEAnisoT::ComputeFreeNodesForOutput(void)
{
	if (!freeNodeQ.IsAllocated())
		ExceptionT::GeneralFail("CSEAnisoT::ComputeFreeNodesForOutput","No TiedNodes Data!");

	ElementSupport().ResetAverage(1);
	
	int nen = NumElementNodes();
	dArray2DT oneElement(nen,1);
	Top();
	while (NextElement())
	{
		oneElement.Set(nen,1,freeNodeQ(CurrElementNumber()));
		ElementSupport().AssembleAverage(CurrentElement().NodesX(),oneElement);
	}
}

void CSEAnisoT::StoreBulkOutput(void)
{
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	ElementBaseT& surroundingGroup = ElementSupport().ElementGroup(iBulkGroups[0]);
	surroundingGroup.SendOutput(fNodalInfoCode);
	if (fNodalQuantities.Length() > 0) 
	{
		fNodalQuantities.Free();
	}
	fNodalQuantities = ElementSupport().OutputAverage();
#endif
}

void CSEAnisoT::SurfaceValuesFromBulk(const ElementCardT& element, iArrayT& ndIndices, 
									  dArray2DT& elementVals, LocalArrayT& nodal_values)
{	
	ndIndices = element.NodesX();
  	int numElemNodes = ndIndices.Length();
	elementVals.Dimension(numElemNodes,fNodalQuantities.MinorDim()); 	
  	nodal_values.Dimension(numElemNodes,fNodalQuantities.MinorDim());
  	for (int iIndex = 0; iIndex < numElemNodes; iIndex++) 
	{
		elementVals.SetRow(iIndex,0.);
	 	elementVals.AddToRowScaled(iIndex,.5,fNodalQuantities(ndIndices[iIndex]));
	 	elementVals.AddToRowScaled(iIndex,.5,fNodalQuantities(ndIndices[otherInds[iIndex]]));
	}
  	nodal_values.SetGlobal(elementVals);
  	ndIndices.SetValueToPosition();
  	nodal_values.SetLocal(ndIndices);
}

void CSEAnisoT::FromNodesToIPs(bool rotate, dArrayT& localFrameIP, LocalArrayT& nodal_values)
{
	dArrayT tensorIP(nodal_values.MinorDim());
	localFrameIP.Dimension(nodal_values.MinorDim());
    fShapes->Interpolate(nodal_values, tensorIP);
    if (rotate) {
		dSymMatrixT s_rot(NumSD(), localFrameIP.Pointer());
		s_rot.MultQBQT(fQ, dSymMatrixT(NumSD(), tensorIP.Pointer()));
	}
	else
		localFrameIP = tensorIP;
}

void CSEAnisoT::UntieOrRetieNodes(int elNum, int nnd, const TiedPotentialBaseT* tiedpot, 
								ArrayT<double>& state, dArrayT& localFrameIP)
{
	if (state[iTiedFlagIndex] == TiedPotentialBaseT::kTiedNode && 
		tiedpot->InitiationQ(localFrameIP))
	{
		for (int i = 0; i < nnd; i++) 
			freeNodeQ(elNum,i) = 1.;
		state[iTiedFlagIndex] = TiedPotentialBaseT::kReleaseNextStep;
	}
	else
		/* see if nodes need to be retied */
		if (qRetieNodes && state[iTiedFlagIndex] == TiedPotentialBaseT::kFreeNode && 
			tiedpot->RetieQ(localFrameIP, state, fdelta))
		{
			state[iTiedFlagIndex] = TiedPotentialBaseT::kTieNextStep;
			for (int i = 0; i < nnd; i++)
				freeNodeQ(elNum,i) = 0.;
		}
}
					

/* Auxiliary routines for heat flow */
void CSEAnisoT::InitializeTemperature(const FieldT* temperature)
{
	if (fIncrementalHeat.Length() == 0) {

		/* initialize heat source arrays */
		fIncrementalHeat.Dimension(fBlockData.Length());
		for (int i = 0; i < fIncrementalHeat.Length(); i++)
		{
			/* dimension */
			fIncrementalHeat[i].Dimension(fBlockData[i].Dimension(), fShapes->NumIP());

			/* register */
			temperature->RegisterSource(fBlockData[i].ID(), fIncrementalHeat[i]);
		}
	}
		
	/* clear sources */
	for (int i = 0; i < fIncrementalHeat.Length(); i++)
		fIncrementalHeat[i] = 0.0;
}
