/*
 * File: GlobalEdgeFinderT.h
 *
 * Class to determine element neighbors based on the connectivies.
 */

/*
 * created      : PAK (02/14/98)
 * last modified: 
 */

#ifndef _GLOBAL_EDGE_FINDER_T_H_
#define _GLOBAL_EDGE_FINDER_T_H_

/* direct members */
#include "iArray2DT.h"
#include "iArrayT.h"
#include "iAutoArrayT.h"

class FEManager;
class ElementBaseT;
class NodeManagerPrimitive;

class GlobalEdgeFinderT
{
  public:

	enum EdgeType { kNoNeighbor = -1, kExteriorFacet = -5 };

	/* constructor */
	GlobalEdgeFinderT (ostream& out);
	~GlobalEdgeFinderT (void);

	void Initialize (FEManager& theBoss, int num_nodes);

	// element group data
	int ElementGroup (int groupid) const;
	int WhichGroup (int elem) const;

	// neighbor data
	void NeighborFacet (int elem, int face, int& neighbor, int& neighborfacet);
	void SetNeighbor (int elem, int face, int neighbor, int neighborfacet);

	// update changing node numbering
	void ResetInvConnects (int numnodes);

	// element data
	void AddElements (int numelems, int numfaces, int group);
	int TotalElements (void) const;
	void LocalElement (int global, int& local, int& group) const;
	int LocalElement (int global, int group) const;
	int GlobalElement (int local, int group) const;

	// node and facet data
	void ZoneFacets (int groupid, const iArrayT& zonegroupids, iArray2DT& sideset, iAutoArrayT& boundarynodes);
	void BoundaryFacets (int groupid, const iArrayT& bordergroupids, iArray2DT& sideset);

	bool HasNode (int node, const ArrayT<int>& facets);
	bool AnotherNode (int node, const ArrayT<int>& facets, const iArrayT& nodes);

	void HitElements (int node, AutoArrayT<int>& hit_elems);

  private:
	void SetInverseConnects (void);
	void HitElements (iArrayT& facenodes, AutoArrayT<int>& hit_elems);

  private:
	ostream& log;
	ArrayT<ElementBaseT*> theElements;

	/* element maps */
	iArrayT fElementID;
	ArrayT<iArrayT> fElementMap;
	iArray2DT fRevElementMap;     // group, local element

  	/* range of node numbers */
  	int fNumNodes;
	int fNumElements;
	int fNumRegular; // number of non CSE groups

  	/* data flags */
  	bool fCurrent;
  	
  	/* data */
  	ArrayT<iArrayT> fNeighbors; 
	ArrayT<iArrayT> fNeighborFacets;
  	ArrayT<iAutoArrayT> fInvConnects; // 1: elements(node)
};

inline int GlobalEdgeFinderT::TotalElements (void) const { return fNumElements; }
inline void GlobalEdgeFinderT::ResetInvConnects (int numnodes) 
{ 
  fNumNodes = numnodes; 
  fCurrent = false; 
  SetInverseConnects ();
}

#endif /* _GLBOAL_EDGE_FINDER_T_H_ */