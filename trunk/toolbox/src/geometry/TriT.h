/* $Id: TriT.h,v 1.5 2004-05-12 17:51:27 paklein Exp $ */
/* created: paklein (07/03/1996) */
#ifndef _TRI_T_H_
#define _TRI_T_H_

/* base class */
#include "GeometryBaseT.h"

namespace Tahoe {

/** 2D triangular parent domain */
class TriT: public GeometryBaseT
{
public:

	/** constructor */
	TriT(int numnodes);  	

	/** return the geometry code */
	virtual GeometryT::CodeT Geometry(void) const { return kTriangle; };

	/** evaluate the shape functions. See GeometryBaseT::EvaluateShapeFunctions 
	 * for documentation */
	virtual void EvaluateShapeFunctions(const dArrayT& coords, dArrayT& Na) const;

	/** evaluate the shape functions and gradients. See 
	 * GeometryBaseT::EvaluateShapeFunctions for documentation */
	virtual void EvaluateShapeFunctions(const dArrayT& coords, dArrayT& Na, 
		dArray2DT& DNa) const;

	/** evaluate the shape functions and gradients. See 
	 * GeometryBaseT::SetLocalShape for documentation */
	virtual void SetLocalShape(dArray2DT& Na, ArrayT<dArray2DT>& Na_x,
		dArrayT& weights) const;

	/* set the values of the nodal extrapolation matrix */
	virtual void SetExtrapolation(dMatrixT& extrap) const;

	/** integration point gradient matrix */
	virtual void IPGradientTransform(int ip, dMatrixT& transform) const;

	/* return the local node numbers for each facet of the element
	 * numbered to produce at outward normal in the order: vertex
	 * nodes, mid-edge nodes, mid-face nodes */
	virtual void NodesOnFacet(int facet, iArrayT& facetnodes) const;
	virtual void NumNodesOnFacets(iArrayT& num_nodes) const;

	/* returns the nodes on each facet needed to determine neighbors
	 * across facets */
	virtual void NeighborNodeMap(iArray2DT& facetnodes) const;
	
	/* return geometry and number of nodes on each facet */
	virtual void FacetGeometry(ArrayT<CodeT>& facet_geom,
		iArrayT& facet_nodes) const;		

	/** return true if the given point is within the domain defined by
	 * the list of coordinates.
	 * \param coords list of coordinates defining the domain
	 * \param point test point coordinates */
	virtual bool PointInDomain(const LocalArrayT& coords, const dArrayT& point) const;
};

} // namespace Tahoe 
#endif /* _TRI_T_H_ */