/* $Id: ContactT.h,v 1.8 2003-03-02 18:55:11 paklein Exp $ */
/* created: paklein (12/11/1997) */
#ifndef _CONTACT_T_H_
#define _CONTACT_T_H_

/* base class */
#include "ElementBaseT.h"

/* direct members */
#include "pArrayT.h"
#include "LocalArrayT.h"
#include "dArray2DT.h"
#include "nVariArray2DT.h"

namespace Tahoe {

/** base class for contact elements */
class ContactT: public ElementBaseT
{
public:

	/** constructor */
	ContactT(const ElementSupportT& support, const FieldT& field, int numfacetnodes);

	/** destructor */
	virtual ~ContactT(void);

	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** element level reconfiguration for the current solution */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** initialization after constructor */
	virtual void Initialize(void);

	/** initialize current time increment. Reset the contact tracking data. */
	virtual void InitStep(void);

	/** solution calls */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force); //not implemented

	/** Returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void); // not implemented
	
	/** writing output */
	virtual void RegisterOutput(void);
	virtual void WriteOutput(void);

	/** compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);  // not implemented

	/** \name append connectivities */
	/*@{*/
	virtual void ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
		AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;

	/** ContactT returns no (NULL) geometry connectivies */
	virtual void ConnectsX(AutoArrayT<const iArray2DT*>& connects) const;
	/*@}*/

protected:

	/** surface specification modes */
	enum SurfaceSpecModeT {kNodesOnFacet = 0 /**< explicitly list nodes on each face */,
                               kSideSets = 1 /**< surface from side set */,
                           kBodyBoundary = 2 /**< surface from body boundaries */};

	/** striker node specification */
	enum StrikerSpecModeT { kNodeSetList = 0 /**< collect from node sets */,
                           kSurfaceNodes = 1 /**< collect from contact surfaces */,
                            kAllStrikers = 2 /**< all nodes as strikers */,
                          kContactBodies = 3,
                            kSideSetList = 4 /**< collect from side sets */};

	/** print element group data */
	virtual void PrintControlData(ostream& out) const;
	
	/** \name initialization steps */
	/*@{*/
	virtual void EchoConnectivityData(ifstreamT& in, ostream& out);
	virtual void SetWorkSpace(void);
	/*@}*/

	/** generate contact element data. Returns true if configuration has
	 * changed since the last call */
	bool SetContactConfiguration(void);

	/** \name steps in setting contact configuration */
	/*@{*/
	/** set "internal" data. Configure information used internally by the class. */
	virtual bool SetActiveInteractions(void) = 0; 

	/** set "external" data. Configure information passed to the FEManagerT,
	 * such as connectivities. */
	virtual void SetConnectivities(void) = 0; 

	/** \name surface input methods */
	/*@{*/
	/** specify facets as lists of nodes */
	void InputNodesOnFacet(ifstreamT& in, iArray2DT& facets);

	/** specify facets as side sets */
	void InputSideSets(ifstreamT& in, iArray2DT& facets);

	/** specify facets automatically from body boundaries */
	void InputBodyBoundary(ifstreamT& in, ArrayT<iArray2DT>& surfaces, int& surface);
	/*@}*/

	/** \name collecting striker nodes */
	/*@{*/
	/** generate striker list from surfaces */
	void StrikersFromSurfaces(void);

	/** collect strikers from nodes sets */
	void ReadStrikers(ifstreamT& in, ostream& out);

	/** collect strikers from sides sets */
	void StrikersFromSideSets(ifstreamT& in, ostream& out);
	/*@}*/

	/** set the tracking data */
	void SetTrackingData(int num_contact, double max_depth);

protected:

	/** control parameters */
	int fNumFacetNodes;

	/** tags on contact surfaces */
	ArrayT<iArray2DT> fSurfaces;

	/* database info */
	iArrayT	  fStrikerTags; // should be variable
	dArray2DT fStrikerCoords; // should be variable
		// only used for search grid

	/** \name by-striker data */
	/*@{*/
	// could also map the strikers onto the variable sized data
	// for the active strikers.
	iArrayT fActiveMap;              /**< map to active strikers                  */
	AutoArrayT<int> fActiveStrikers; /**< global numbers of active strikers       */
	AutoArrayT<int> fHitSurface;     /**< contact surface for each active striker */
	AutoArrayT<int> fHitFacets;      /**< facet of contact surface                */
	/*@}*/

	/** link surfaces in ConnectsU needed for graph */
	iArray2DT fSurfaceLinks;
	
private:

	/** \name dynamic work space managers for element arrays */
	/*@{*/
	nVariArray2DT<int> fConnectivities_man;		
	nVariArray2DT<int> fEqnos_man;
	/*@}*/

	/** \name tracking data. Parameters should be set using ContactT::SetTrackingData
	 * and are written during ContactT::WriteOutput. */
	/*@{*/
	int    fnum_contact;
	double fh_max;
	/*@}*/
	
//NOTE: need to generate overall list of facets (all same "geometry"?)
//      to return as ConnectsX for domain decompositition, and what to
//      due with strikers? For now do not return decomposition connects
};

inline void ContactT::SetTrackingData(int num_contact, double max_depth)
{
	fnum_contact = num_contact;
	fh_max = max_depth;
}

} // namespace Tahoe 
#endif /* _CONTACT_T_H_ */
