/* $Id: PenaltyContactDrag2DT.cpp,v 1.5.2.1 2004-07-06 06:53:13 paklein Exp $ */
/* created: paklein (12/11/1997) */
#include "PenaltyContactDrag2DT.h"
#include "ifstreamT.h"
#include "eIntegratorT.h"
#include "ModelManagerT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

using namespace Tahoe;

/* constructor */
PenaltyContactDrag2DT::PenaltyContactDrag2DT(const ElementSupportT& support, const FieldT& field):
	PenaltyContact2DT(support, field),
	fDrag(0),
	fGapTolerance(0),
	fSlipTolerance(0)
{
	SetName("contact_drag_2D_penalty");
}

PenaltyContactDrag2DT::PenaltyContactDrag2DT(const ElementSupportT& support):
	PenaltyContact2DT(support),
	fDrag(0),
	fGapTolerance(0),
	fSlipTolerance(0)
{
	SetName("contact_drag_2D_penalty");
}

/* initialization after constructor */
void PenaltyContactDrag2DT::Initialize(void)
{
	/* inherited */
	PenaltyContact2DT::Initialize();

	ifstreamT& in = ElementSupport().Input();

	/* drag parameters */
	in >> fDrag
	   >> fGapTolerance
	   >> fSlipTolerance;

	/* node blocks containing striker nodes */
	int num_blocks = -1;
	in >> num_blocks;
	ArrayT<StringT> element_id(num_blocks);
	for (int i = 0; i < element_id.Length(); i++)
		in >> element_id[i];

	/* compute associated nodal area (using all element blocks) */
	ComputeNodalArea(element_id, fNodalArea, fStrikerLocNumber);
}

/* describe the parameters needed by the interface */
void PenaltyContactDrag2DT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	PenaltyContact2DT::DefineParameters(list);

	list.AddParameter(fDrag, "drag_traction");
	list.AddParameter(fGapTolerance, "gap_tolerance");
	list.AddParameter(fSlipTolerance, "slip_tolerance");
}

/* accept parameter list */
void PenaltyContactDrag2DT::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	PenaltyContact2DT::TakeParameterList(list);

	fDrag = list.GetParameter("drag_traction");
	fGapTolerance = list.GetParameter("gap_tolerance");
	fSlipTolerance = list.GetParameter("slip_tolerance");

	/* compute associated nodal area */
	ArrayT<StringT> element_id;
	ElementSupport().ModelManager().ElementGroupIDsWithNodes(fStrikerTags, element_id);
	ComputeNodalArea(element_id, fNodalArea, fStrikerLocNumber);
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* print element group data */
void PenaltyContactDrag2DT::PrintControlData(ostream& out) const
{
	/* inherited */
	PenaltyContact2DT::PrintControlData(out);

	/* regularization */
	out << " Magnitude of the drag traction. . . . . . . . . = " << fDrag << '\n';	
	out << " Gap tolerance for drag. . . . . . . . . . . . . = " << fGapTolerance << '\n';	
	out << " Minimum incremental slip for drag . . . . . . . = " << fSlipTolerance << '\n';	
}

void PenaltyContactDrag2DT::RHSDriver(void)
{
	/* time integration parameters */
	double constKd = 0.0;
	int     formKd = fIntegrator->FormKd(constKd);
	if (!formKd) return;

	/* references to global nodal data */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
	const dArray2DT& disp = Field()(0,0); /* displacements */
	const dArray2DT& disp_last = Field()(-1,0); /* displacements from last step */

	/* reset tracking data */
	int num_contact = 0;
	double h_max = 0.0;

	/* loop over active elements */
	dArrayT tangent(NumSD());
	iArrayT eqnos;
	dArrayT tangent_ref(NumSD()), drag(NumDOF()); 
	const int* pelem = fConnectivities[0]->Pointer();
	int rowlength = fConnectivities[0]->MinorDim();
	for (int i = 0; i < fConnectivities[0]->MajorDim(); i++, pelem += rowlength)
	{
		/* collect element configuration */
		fElCoord.RowCollect(pelem, init_coords);
		fElDisp.RowCollect(pelem, disp);

		/* current configuration using effective displacement */
		fElCoord.AddScaled(constKd, fElDisp); //EFFECTIVE_DVA
	
		/* get facet and striker coords */
		fElCoord.RowAlias(0, fx1);
		fElCoord.RowAlias(1, fx2);
		fElCoord.RowAlias(2, fStriker);

		/* penetration vectors */
		fv1.DiffOf(fStriker, fx1);
		fv2.DiffOf(fStriker, fx2);

		/* tangent vector */
		tangent.DiffOf(fx2, fx1);

		/* distance to facet (could store some of this) */
		double magtan = tangent.Magnitude();				
		double      h = (fv2[0]*fv1[1] - fv1[0]*fv2[1])/magtan;
//		double  max_d =-magtan/10; //max penetration

		/* contact */
		bool has_contact = false;
		if (h < 0.0)
		{
			has_contact = true;
		
			/* tracking data */
			num_contact++;
			h_max = (h < h_max) ? h : h_max;

			/* penetration force */
			double dphi =-fK*h;
			
			/* initialize */
			fRHS = 0.0;
					
			/* d_tan contribution */
			fdtanT.Multx(tangent, fNEEvec);
			fRHS.AddScaled(-dphi*h/(magtan*magtan), fNEEvec);
						
			/* d_area */
			fColtemp1.Set(fdv1T.Rows(), fdv1T(0));
			fColtemp2.Set(fdv2T.Rows(), fdv2T(1));
			fRHS.AddCombination(-dphi*fv2[1]/magtan, fColtemp1,
				                -dphi*fv1[0]/magtan, fColtemp2);
			
			fColtemp1.Set(fdv1T.Rows(), fdv1T(1));
			fColtemp2.Set(fdv2T.Rows(), fdv2T(0));
			fRHS.AddCombination(dphi*fv2[0]/magtan, fColtemp1,
				                dphi*fv1[1]/magtan, fColtemp2);					
		}
		
		/* drag */
		bool has_drag = false;
		if (h < fGapTolerance)
		{
			/* reference tangent */
			tangent_ref.DiffOf(init_coords(pelem[1]), init_coords(pelem[0]));
			double tangent_ref_mag = tangent_ref.Magnitude();

			/* displacement from the last increment */
			int striker_node = pelem[2];
			drag.DiffOf(disp_last(striker_node), disp(striker_node));
			
			/* striker is "sliding" */
			double mag_slip = dArrayT::Dot(drag, tangent_ref)/tangent_ref_mag;
			if (fabs(mag_slip) > fSlipTolerance)
			{
				has_drag = true;
				if (!has_contact) fRHS = 0.0;
			
				/* drag force */
				int striker_index = fStrikerLocNumber.Map(striker_node);				
				double drag_force = fNodalArea[striker_index]*fDrag;
				double f_x = drag_force*tangent_ref[0]/tangent_ref_mag;
				double f_y = drag_force*tangent_ref[1]/tangent_ref_mag;
			
				/* assemble - equal and opposite force on facet nodes */
				fRHS[0] += -0.5*f_x;
				fRHS[1] += -0.5*f_y;
				fRHS[2] += -0.5*f_x;
				fRHS[3] += -0.5*f_y;
				fRHS[4] += f_x;
				fRHS[5] += f_y;
			}
		}

		/* assemble */
		if (has_contact || has_drag)
		{
			/* get equation numbers */
			fEqnos[0].RowAlias(i, eqnos);

			/* assemble */
			ElementSupport().AssembleRHS(Group(), fRHS, eqnos);
		}
	}

	/* set tracking */
	SetTrackingData(num_contact, h_max);
}
