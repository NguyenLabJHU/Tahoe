/* $Id: ContactT.cpp,v 1.7.2.1 2002-06-27 18:02:41 cjkimme Exp $ */
/* created: paklein (12/11/1997) */

#include "ContactT.h"

#include <math.h>
#include <iostream.h>
#include <iomanip.h>

#include "ModelManagerT.h"
#include "fstreamT.h"
#include "IOBaseT.h"
#include "iGridManager2DT.h"
#include "ContinuumElementT.h" // For conversion of side sets to facets.
// Do directly or add call to FEManagerT?

/* constructor */

using namespace Tahoe;

ContactT::ContactT(const ElementSupportT& support, const FieldT& field, int numfacetnodes):
	ElementBaseT(support, field),
	fNumFacetNodes(numfacetnodes)
{

}

/* destructor */
ContactT::~ContactT(void) {	}

/* form of tangent matrix */
GlobalT::SystemTypeT ContactT::TangentType(void) const
{
	return GlobalT::kSymmetric;
}

/* element level reconfiguration for the current solution */
GlobalT::RelaxCodeT ContactT::RelaxSystem(void)
{
	/* inherited */
	GlobalT::RelaxCodeT relax = ElementBaseT::RelaxSystem();

	/* generate contact element data */
	bool contact_changed = SetContactConfiguration();

	/* minimal test of new-ness */
	if (!contact_changed)
		return relax;
	else
		return GlobalT::MaxPrecedence(relax, GlobalT::kReEQ);
}

/* initialization after constructor */
void ContactT::Initialize(void)
{
	/* inherited */
	ElementBaseT::Initialize();

	/* set up work space */
	SetWorkSpace();
	
	/* set initial contact configuration */
	SetContactConfiguration();	
}

/* solution calls */
void ContactT::AddNodalForce(const FieldT& field, int node, dArrayT& force)
{
#pragma unused(field)
#pragma unused(node)
#pragma unused(force)
//not implemented
}

/* Returns the energy as defined by the derived class types */
double ContactT::InternalEnergy(void)
{
//not implemented
	return 0.0;
}

/* writing output - nothing to write */
void ContactT::RegisterOutput(void) {}
void ContactT::WriteOutput(IOBaseT::OutputModeT mode)
{
#pragma unused(mode)
	/* contact statistics */
	ostream& out = ElementSupport().Output();
	out << "\n Contact tracking: group " << ElementSupport().ElementGroupNumber(this) + 1 << '\n';
	out << " Time                           = " << ElementSupport().Time() << '\n';
	out << " Active strikers                = " << fActiveStrikers.Length() << '\n';
	if (fActiveStrikers.Length() > 0)
	{
		out << setw(kIntWidth) << "striker";
		out << setw(kIntWidth) << "surface";
		out << setw(kIntWidth) << "facet";
		out << setw(fNumFacetNodes*kIntWidth) << "facet nodes" << '\n';
		for (int i = 0; i < fActiveStrikers.Length(); i++)
		{
			out << setw(kIntWidth) << fActiveStrikers[i] + 1;
			out << setw(kIntWidth) << fHitSurface[i] + 1; int hit_facet = fHitFacets[i];
			out << setw(kIntWidth) << hit_facet + 1;
			for (int j = 0; j < fNumFacetNodes; j++)
				out << setw(kIntWidth) << fSurfaces[fHitSurface[i]](hit_facet, j) + 1;
			out << '\n';
		}
		out << endl;
	}
}

/* compute specified output parameter and send for smoothing */
void ContactT::SendOutput(int kincode)
{
#pragma unused(kincode)
//not implemented: contact tractions/forces
}

/* appends group connectivities to the array */
void ContactT::ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const
{
	/* inherited */
	ElementBaseT::ConnectsU(connects_1, connects_2);

	/* add surface links */
	connects_1.AppendUnique(&fSurfaceLinks);
}

/* returns no (NULL) geometry connectivies */
void ContactT::ConnectsX(AutoArrayT<const iArray2DT*>& connects) const
{
#pragma unused (connects)
//	connects.Append(NULL);
}

/***********************************************************************
* Protected
***********************************************************************/

/* print element group data */
void ContactT::PrintControlData(ostream& out) const
{
	/* inherited */
	ElementBaseT::PrintControlData(out);

	out << " Number of facet nodes . . . . . . . . . . . . . = "
	    << fNumFacetNodes << '\n';
}

/* echo contact bodies and striker nodes */
void ContactT::EchoConnectivityData(ifstreamT& in, ostream& out)
{
	int num_surfaces;
	in >> num_surfaces;
	out << " Number of contact surfaces. . . . . . . . . . . = "
	    << num_surfaces << '\n';
	if (num_surfaces < 1) throw eBadInputValue;

	/* read contact bodies */
	fSurfaces.Allocate(num_surfaces);
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
				InputSideSets(in, out, fSurfaces[i]);
				break;
			
			case kBodyBoundary:
				/* may resize the surfaces array */
				InputBodyBoundary(in, fSurfaces, i);
				num_surfaces = fSurfaces.Length();
				break;
		
			default:
				cout << "\n ContactT::EchoConnectivityData: unknown surface specification\n";
				cout <<   "     mode " << spec_mode << " for surface " << i+1 << '\n';
				throw eBadInputValue;
		}
	}
	
	// after the read section, should have valid nodes/facet connectivities
	// for the local database

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
		case kListStrikers: /* read strikers */
			ReadStrikers(in, out);
			break;
		
		case kSurfaceNodes: /* collect nodes from contact surfaces */
			StrikersFromSurfaces();
			break;
	
		case kAllStrikers:  /* shallow striker coords */
			fStrikerCoords.Alias(ElementSupport().CurrentCoordinates());
			out << "\n Striker nodes: ALL\n";	
			break;
	
		default:
			cout << "\n ContactT::EchoConnectivityData: unknown striker specification\n";
			cout <<   "     mode " << striker_spec_mode << '\n';
			throw eBadInputValue;
	}

	/* echo */
	if (ElementSupport().PrintInput())
	{
		out << "\n Striker nodes:\n";
		fStrikerTags++;
		out << fStrikerTags.wrap(8) << '\n';
		fStrikerTags--;	
	}
	
	/* allocate striker coords */
	fStrikerCoords.Allocate(fStrikerTags.Length(), NumSD());
	
	/* set connectivity name */
	ModelManagerT& model = ElementSupport().Model();
	StringT name ("Contact");
	name.Append (ElementSupport().ElementGroupNumber(this) + 1);

	/* register with the model manager and let it set the ward */
	int nen = fNumFacetNodes + 1; /* facet nodes + 1 striker */
	if (!model.RegisterVariElements (name, fConnectivities_man, GeometryT::kLine, nen, 0)) 
		throw eGeneralFail;

	/* set up fConnectivities */
	fConnectivities.Allocate(1);
	fConnectivities[0] = model.ElementGroupPointer(name);

	/* set up fBlockData to store block ID */
	fBlockData.Allocate(1);
	fBlockData[0].Set(name, 0, fConnectivities[0]->MajorDim(), -1);

	/* set managed equation numbers array */
	fEqnos.Allocate(1);
	fEqnos_man.SetWard(0, fEqnos[0], nen*NumDOF());
}

void ContactT::SetWorkSpace(void)
{
	/* allocate map to active strikers data */
	fActiveMap.Allocate(fStrikerCoords.MajorDim());
	fActiveMap = -1;

	/* make pseudo-element list to link surfaces in case
	 * bodies are not otherwise interacting (for the bandwidth
	 * reduction) */
	int num_surfaces = fSurfaces.Length();
	if (num_surfaces > 1)
	{
		fSurfaceLinks.Allocate(num_surfaces - 1, 2);
		for (int i = 0; i < num_surfaces - 1; i++)
		{
			fSurfaceLinks(i,0) = (fSurfaces[i  ])[0];
			fSurfaceLinks(i,1) = (fSurfaces[i+1])[0];
		}
	}
}

/* generate contact element data - return true if configuration has
* changed since the last call */
bool ContactT::SetContactConfiguration(void)
{
	bool contact_changed = SetActiveInteractions();
	if (contact_changed)
	{
		/* resize */
		int nel = fActiveStrikers.Length();
		fConnectivities_man.SetMajorDimension(nel, false);
		fEqnos_man.SetMajorDimension(nel, false);

		/* generate connectivities */
		SetConnectivities();	

		/* update dimensions */
		ElementBlockDataT& block = fBlockData[0];
		block.Set(block.ID(), block.StartNumber(), fConnectivities[0]->MinorDim(), block.MaterialID());
	}
	
	return contact_changed;
}

/***********************************************************************
* Private
***********************************************************************/

/* surface input functions */
void ContactT::InputNodesOnFacet(ifstreamT& in, iArray2DT& facets)
{
	int num_facets;
	in >> num_facets;
	if (num_facets < 0) throw eBadInputValue;
	
	/* dimension */
	facets.Allocate(num_facets, fNumFacetNodes);
	
	/* read */
	in >> facets;
	
	/* correct numbering */
	facets--;
}

void ContactT::InputSideSets(ifstreamT& in, ostream& out, iArray2DT& facets)
{
#pragma unused(out)
#ifdef __NO_RTTI__
	cout << "\n ContactT::InputSideSets: RTTI required, but not available.\n";
	cout <<   "     Use different surface specification mode." << endl;
	throw;
#endif

	int elem_group;
	in >> elem_group;
	elem_group--;
	const ContinuumElementT* pelem_group =
		dynamic_cast<const ContinuumElementT*>(&ElementSupport().ElementGroup(elem_group));

	/* checks */
	if (!pelem_group)
	{
		cout << "\n ContactT::InputSideSets: element group " << elem_group;
		cout << " must be of type\n" <<   "     ContinuumElementT" << endl;
		throw eBadInputValue;
	}
	
	/* read data from parameter file */
	ArrayT<StringT> ss_ID;
	bool multidatabasesets = false; /* change to positive and the parameter file format changes */
	ModelManagerT& model = ElementSupport().Model();
	model.SideSetList (in, ss_ID, multidatabasesets);

	if (ss_ID.Length () != 1) 
	  {
	    cout << "\n\nContactT::InputSideSets: Model Manager read more than one side set, not programmed for this.\n\n";
	    throw eBadInputValue;
	  }

	/* read side set */
	StringT elem_ID;
	iArray2DT side_set = model.SideSet(ss_ID[0]);
	if (side_set.MajorDim() > 0 && model.IsSideSetLocal(ss_ID[0]))
	    elem_ID = model.SideSetGroupID(ss_ID[0]);
	else {
		iArray2DT temp = side_set;
		model.SideSetGlobalToLocal(temp, side_set, elem_ID);
	}

	/* numbers from element group */
	pelem_group->SideSetToFacets(elem_ID, side_set, facets);
}

void ContactT::InputBodyBoundary(ifstreamT& in, ArrayT<iArray2DT>& surfaces,
		int& surface)
{
#ifdef __NO_RTTI__
	cout << "\n ContactT::InputBodyBoundary: RTTI required, but not available.\n";
	cout <<   "     Use different surface specification mode." << endl;
	throw;
#endif

	/* get element group */
	int elem_group;
	in >> elem_group;
	elem_group--;
	
	const ContinuumElementT* pelem_group =
		dynamic_cast<const ContinuumElementT*>(&ElementSupport().ElementGroup(elem_group));

	/* checks */
	if (!pelem_group)
	{
		cout << "\n ContactT::InputBodyBoundary: element group " << elem_group;
		cout << " must be of type\n" <<   "     ContinuumElementT" << endl;
		throw eBadInputValue;
	}

	/* get sets of facet */
	GeometryT::CodeT geometry;
	ArrayT<iArray2DT> surface_facet_sets;
	iArrayT surface_nodes;
	pelem_group->SurfaceFacets(geometry, surface_facet_sets, surface_nodes);

	/* just one surface */
	if (surface_facet_sets.Length() == 1)
		surfaces[surface] = surface_facet_sets[0];
	else if (surface_facet_sets.Length() > 1)
	{
		int num_sets = surface_facet_sets.Length();
		
		/* resize surfaces array */
		surfaces.Resize(surfaces.Length() + (num_sets - 1)); //NOTE: not byte copy!
		for (int i = 0; i < num_sets; i++)
		{
			/* copy */
			surfaces[surface] = surface_facet_sets[i];

			/* next */
			surface++;
		}
	}
}

/* generate striker list from surfaces */
void ContactT::StrikersFromSurfaces(void)
{
	//TEMP just make big for now
	int num_nodes = ElementSupport().NumNodes();
	iArrayT counts(num_nodes);
	counts = 0;

	/* tally occurrences */
	for (int i = 0; i < fSurfaces.Length(); i++)
	{
		iArray2DT& surface = fSurfaces[i];
		int* psurf  = surface.Pointer();
		int  length = surface.Length();
		for (int j = 0; j < length; j++)
		{
			counts[*psurf]++;
			psurf++;
		}
	}

	/* count surface nodes */
	int  node_count = 0;
	int* pcount = counts.Pointer();
	for (int j = 0; j < num_nodes; j++)
		if (*pcount++ > 0)
			node_count++;

	/* collect */
	fStrikerTags.Allocate(node_count);
	pcount = counts.Pointer();
	int* pstrike = fStrikerTags.Pointer();
	for (int k = 0; k < num_nodes; k++)
		if (*pcount++ > 0)
			*pstrike++ = k;
}

void ContactT::ReadStrikers(ifstreamT& in, ostream& out)
{
#pragma unused(out)

  ModelManagerT& model = ElementSupport().Model();

  /* read list of node set id indexes */
  ArrayT<StringT> ns_ID;
  model.NodeSetList(in, ns_ID);

  /* collect nodes from those indexes */
  model.ManyNodeSets(ns_ID, fStrikerTags);
}
