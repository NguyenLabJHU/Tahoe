/* $Id: MFPenaltyContact2DT.cpp,v 1.2 2003-11-04 17:37:50 paklein Exp $ */
#include "MFPenaltyContact2DT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "fstreamT.h"
#include "eIntegratorT.h"
#include "InverseMapT.h"
#include "iGridManager2DT.h"

/* meshfree element group types */
#include "MeshFreeSSSolidT.h"
#include "MeshFreeFSSolidT.h"
#include "MeshFreeSupportT.h"

/* parameters (duplicated from Contact2DT) */
const int kNumFacetNodes = 2;
const int kMaxNumGrid    = 75;

using namespace Tahoe;

/* constructor */
MFPenaltyContact2DT::MFPenaltyContact2DT(const ElementSupportT& support, const FieldT& field):
	PenaltyContact2DT(support, field),
	fElementGroup(NULL),
	fMeshFreeSupport(NULL),
	fStrikerCoords_man(0, fStrikerCoords, support.NumSD()),
	fdvT_man(0, true),
	fRHS_man(0, fRHS)
{
	ElementSupport().Input() >> fGroupNumber;
	fGroupNumber--;
	ElementBaseT& element = ElementSupport().ElementGroup(fGroupNumber);
	fElementGroup = &element;
	
	/* cast to meshfree element types */
	const MeshFreeSSSolidT* mf_ss_solid = dynamic_cast<const MeshFreeSSSolidT*>(fElementGroup);
	const MeshFreeFSSolidT* mf_fs_solid = dynamic_cast<const MeshFreeFSSolidT*>(fElementGroup);
	if (mf_ss_solid)
		fMeshFreeSupport = &(mf_ss_solid->MeshFreeSupport());	
	else if (mf_fs_solid)
		fMeshFreeSupport = &(mf_fs_solid->MeshFreeSupport());	
	else
		ExceptionT::GeneralFail("MFPenaltyContact2DT::MFPenaltyContact2DT",
			"element group %d is not meshfree", fGroupNumber+1);

	/* set map of node ID to meshfree point index */
	fNodeToMeshFreePoint.SetMap(fMeshFreeSupport->NodesUsed());
}

/***********************************************************************
 * Protected
 ***********************************************************************/

/* print element group data */
void MFPenaltyContact2DT::PrintControlData(ostream& out) const
{
	/* inherited */
	PenaltyContact2DT::PrintControlData(out);

	out << " Meshfree element group. . . . . . . . . . . . . = " << fGroupNumber+1 << '\n';	
}

/* called by FormRHS and FormLHS */
void MFPenaltyContact2DT::LHSDriver(GlobalT::SystemTypeT)
{
	double constK = 0.0;
	int formK = fIntegrator->FormK(constK);
	if (formK)
		ExceptionT::GeneralFail("MFPenaltyContact2DT::LHSDriver", "not implemented");
}

void MFPenaltyContact2DT::RHSDriver(void)
{
	/* time integration parameters */
	double constKd = 0.0;
	int     formKd = fIntegrator->FormKd(constKd);
	if (!formKd) return;

	/* references to global nodal data */
	const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
	const dArray2DT& disp = Field()[0]; /* displacements */

	/* striker neighbors */
	const RaggedArray2DT<int>& striker_neighbors = fMeshFreeSupport->NodeNeighbors();

	/* compute current configuration of active strikers */
	ComputeStrikerCoordinates(fActiveStrikers);

	/* reset tracking data */
	int num_contact = 0;
	double h_max = 0.0;

	/* loop over active elements */
	dArrayT tangent(NumSD());
	AutoArrayT<int> nodes;
	AutoArrayT<int> eqnos;
	iArray2DT eqnos2D;
	iArrayT neighbors;
	dArrayT Na;
	dArray2DT DNa;
	int* pelem = fConnectivities[0]->Pointer();
	int rowlength = fConnectivities[0]->MinorDim();
	for (int i = 0; i < fConnectivities[0]->MajorDim(); i++, pelem += rowlength)
	{
		/* collect element configuration */
		fElCoord.RowCollect(pelem, init_coords);
		fElDisp.RowCollect(pelem, disp);

		/* current configuration */
		fElCoord.AddScaled(constKd, fElDisp);
	
		/* get facet coords */
		fElCoord.RowAlias(0, fx1);
		fElCoord.RowAlias(1, fx2);

		/* collect information for current meshless striker */
		int striker_node = pelem[2];
		int striker_meshfree_index = fNodeToMeshFreePoint.Map(striker_node);
		int striker_active_index = fNodeToActiveStriker.Map(striker_node);
		fStrikerCoords.RowAlias(striker_active_index, fStriker);

		/* striker neighborhood */
		fMeshFreeSupport->LoadNodalData(striker_node, neighbors, Na, DNa);
		SetDerivativeArrays(Na);

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
		if (h < 0.0)
		{
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
					
			/* get equation numbers */
			nodes.Dimension(kNumFacetNodes + neighbors.Length());
			nodes[0] = pelem[0];
			nodes[1] = pelem[1];
			nodes.CopyPart(kNumFacetNodes, neighbors, 0, neighbors.Length());
			eqnos.Dimension(nodes.Length()*NumDOF());
			eqnos2D.Alias(nodes.Length(), NumDOF(), eqnos.Pointer());
			Field().SetLocalEqnos(nodes, eqnos2D);

			/* assemble */
			ElementSupport().AssembleRHS(Group(), fRHS, eqnos2D);
		}
	}

	/* set tracking */
	SetTrackingData(num_contact, h_max);
}

/* set contact surfaces and strikers */
void MFPenaltyContact2DT::EchoConnectivityData(ifstreamT& in, ostream& out)
{
	const char caller[] = "MFPenaltyContact2DT::EchoConnectivityData";

	int num_surfaces;
	in >> num_surfaces;
	out << " Number of contact surfaces. . . . . . . . . . . = "
	    << num_surfaces << '\n';
	if (num_surfaces < 1) ExceptionT::BadInputValue(caller);

	/* read contact bodies */
	fSurfaces.Dimension(num_surfaces);
	for (int i = 0; i < fSurfaces.Length(); i++)
	{
		int spec_mode;
		in >> spec_mode;
		switch (spec_mode)
		{
			case kNodesOnFacet:	
				InputNodesOnFacet(in, fSurfaces[i]);
				break;
			
			case kSideSets:
				InputSideSets(in, fSurfaces[i]);
				break;
			
			case kBodyBoundary:
				/* may resize the surfaces array */
				InputBodyBoundary(in, fSurfaces, i);
				num_surfaces = fSurfaces.Length();
				break;
		
			default:
				ExceptionT::BadInputValue(caller, "unknown surface specification mode %d for surface %d",
					spec_mode, i+1);
		}
	}

	/* echo data and correct numbering offset */
	out << " Contact surfaces:\n";
	out << setw(kIntWidth) << "surface"
	    << setw(kIntWidth) << "facets"
	    << setw(kIntWidth) << "size" << '\n';
	int surface_count = 0;
	for (int j = 0; j < fSurfaces.Length(); j++)
	{		
	  	iArray2DT& surface = fSurfaces[j];

	  	out << setw(kIntWidth) << j+1
	  	    << setw(kIntWidth) << surface.MajorDim()
	  	    << setw(kIntWidth) << surface.MinorDim() << "\n\n";
	  	
	  	/* set offset for output */
	  	if (ElementSupport().PrintInput())
	  	{
	  		surface++;
	  		surface.WriteNumbered(out);
	  		surface--;
	  		out << '\n';
	  	}
	  	
	  	/* count non-empty */
	  	if (surface.MajorDim() > 0) surface_count++;
	}	
	
	/* remove empty surfaces */
	if (surface_count != fSurfaces.Length())
	{
		out << " Found empty contact surfaces:\n\n";
		ArrayT<iArray2DT> tmp_surfaces(surface_count);
		surface_count = 0;
		for (int i = 0; i < fSurfaces.Length(); i++)
		{
	  		iArray2DT& surface = fSurfaces[i];
			if (surface.MajorDim() == 0)
				out << " removing surface " << i+1 << '\n';
			else
				tmp_surfaces[surface_count++].Swap(surface);
		}
		
		/* exchange */
		fSurfaces.Swap(tmp_surfaces);
	}
	
	/* striker nodes */
	int striker_spec_mode;
	in >> striker_spec_mode;
	switch (striker_spec_mode)
	{
		case kNodeSetList: /* read strikers from node sets */
			ReadStrikers(in, out);
			break;
		
		case kSurfaceNodes: /* collect nodes from contact surfaces */
			StrikersFromSurfaces();
			break;
	
		case kAllStrikers:  /* shallow striker coords */
			fStrikerTags.Alias(fMeshFreeSupport->NodesUsed());
			out << "\n Striker meshfree nodes: ALL\n";
			break;

		case kSideSetList: /* collect strikers from side sets */
			StrikersFromSideSets(in, out);
			break;
	
		default:
			ExceptionT::BadInputValue(caller, "unknown striker specification mode %d",
				striker_spec_mode);
	}
	
	/* check to see that all strikers are meshfree */
	if (striker_spec_mode != kAllStrikers) {
		InverseMapT map;
		map.SetOutOfRange(InverseMapT::MinusOne);
		map.SetMap(fMeshFreeSupport->NodesUsed());
		for (int i = 0; i < fStrikerTags.Length(); i++)
			if (map.Map(fStrikerTags[i]) == -1)
				ExceptionT::GeneralFail(caller, "striker %d is not meshfree", fStrikerTags[i]+1);
	}

	/* echo */
	if (ElementSupport().PrintInput()) {
		out << "\n Striker nodes:\n";
		fStrikerTags++;
		out << fStrikerTags.wrap(8) << '\n';
		fStrikerTags--;	
	}
}

/* generate contact element data */
bool MFPenaltyContact2DT::SetActiveInteractions(void)
{
	int last_num_active = fActiveStrikers.Length();

	/* current coords of all strikers */
	ComputeStrikerCoordinates(fStrikerTags);

	/* construct search grid if needed */
	if (!fGrid2D)
	{
		/* try to get roughly least 10 per grid */
		int ngrid = int(pow(fStrikerCoords.MajorDim()/10.0,
		                    1.0/fStrikerCoords.MinorDim())) + 1;

		ngrid = (ngrid < 2) ? 2 : ngrid;
		ngrid = (ngrid > kMaxNumGrid) ? kMaxNumGrid : ngrid;

		fGrid2D = new iGridManager2DT(ngrid, ngrid, fStrikerCoords, 0);
		if (!fGrid2D) ExceptionT::OutOfMemory("MFPenaltyContact2DT::SetActiveInteractions");

		/* search grid statistics */
		ostream& out = ElementSupport().Output();
		out << "\n Search grid: group " << ElementSupport().ElementGroupNumber(this) + 1 << '\n';
		fGrid2D->WriteStatistics(out);
	}
	
	/* (re-)set grid boundaries */
	fGrid2D->Reset();

	/* update by-body stored data */
	SetSurfacesData();
		
	/* set striker/facet data */
	SetActiveStrikers();
	
	/* set map from global ID to active striker index */
	fNodeToActiveStriker.SetMap(fActiveStrikers);
	
	/* assume changed unless last and current step have no active */
	if (last_num_active == 0 && fActiveStrikers.Length() == 0)
		return false;
	else
		return true;
}	

void MFPenaltyContact2DT::ComputeStrikerCoordinates(const ArrayT<int>& strikers)
{
	/* dimension */
	fStrikerCoords_man.SetMajorDimension(strikers.Length(), false);

	/* current striker coords */
	if (strikers.Length() > 0) {
	
		/* reconstruct displacement field */
		iArrayT tmp;
		tmp.Alias(strikers);
		fElementGroup->NodalDOFs(tmp, fStrikerCoords);

		/* compute current coordinates */
		const dArray2DT& init_coords = ElementSupport().InitialCoordinates();
		for (int i = 0; i < strikers.Length(); i++)
			fStrikerCoords.AddToRowScaled(i, 1.0, init_coords(strikers[i]));
	}
}

/* set derivative arrays given the array of shape functions for the
 * nodes in the neighborhood of the meshfree striker. */
void MFPenaltyContact2DT::SetDerivativeArrays(const dArrayT& mf_shape)
{
	/* dimenions */
	int nsd = NumSD();
	int ndof = NumDOF();
	fdvT_man.Dimension((mf_shape.Length() + kNumFacetNodes)*ndof, nsd);

	/* facet nodes */	
	fdv1T(0,0) =-1.0;
	fdv1T(1,1) =-1.0;
	fdv1T(2,0) = 0.0;
	fdv1T(3,1) = 0.0;

	fdv2T(0,0) = 0.0;
	fdv2T(1,1) = 0.0;
	fdv2T(2,0) =-1.0;
	fdv2T(3,1) =-1.0;

	/* striker node */
	int dex = kNumFacetNodes*ndof;
	for (int i = 0; i < mf_shape.Length(); i++) {
		double shape = mf_shape[i];
		fdv1T(dex  , 0) = shape;
		fdv1T(dex+1, 1) = shape;
		fdv2T(dex  , 0) = shape;
		fdv2T(dex+1, 1) = shape;
		dex += ndof;
	}
}
