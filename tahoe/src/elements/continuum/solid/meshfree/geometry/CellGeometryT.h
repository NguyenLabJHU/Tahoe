/* $Id: CellGeometryT.h,v 1.5 2005-01-27 17:50:34 paklein Exp $ */
#ifndef _CELL_GEOMETRY_T_
#define _CELL_GEOMETRY_T_

#include "SCNIMFT.h"
#include "ParameterInterfaceT.h"
#include "ElementSupportT.h"
#include "iArrayT.h"
#include "dArray2DT.h"
#include "RaggedArray2DT.h"
#include "MeshFreeNodalShapeFunctionT.h"
#include "LinkedListT.h"

namespace Tahoe {

class dArrayT;

class CellGeometryT: public ParameterInterfaceT
{
public:

	/** constructor */
	CellGeometryT(const ElementSupportT& support, bool isAxisymmetric);
	CellGeometryT(void);

	/** collect-geometry specific mesh, node, element, or body information */
	virtual void DefineElements(const ArrayT<StringT>& block_ID, const ArrayT<int>& mat_index);
	
	/** set the nodal coordinates and shape functions
	* \param nodes global ID's of the nodes associated with the rows in the coordinate array */
	void SetNodesAndShapes(const iArrayT* nodes, const dArray2DT* nodal_coordinates, 
		MeshFreeNodalShapeFunctionT* nodalShapeFunctions);
	
	/** generate data structures for integration over the body boundary */
	virtual void BoundaryShapeFunctions(RaggedArray2DT<double>& phis, RaggedArray2DT<int>& supports, dArray2DT& normals) = 0;
	
	void SetNodalElements(SCNIMFT* scnimft_group) { fscnimft = scnimft_group; };
	
	/** compute B matrices for strain smoothing/nodal integration */
	virtual void ComputeBMatrices(RaggedArray2DT<int>& nodalCellSupports, RaggedArray2DT<dArrayT>& bVectorArray,
									dArrayT& cellVolumes, dArray2DT& cellCentroids, RaggedArray2DT<double>& circumferential_B) = 0;

	/** accessor to the element support class */
	const ElementSupportT& ElementSupport(void) const;
	
	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;

	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& name, ParameterListT::ListOrderT& order, 
		SubListT& sub_lists) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& name) const;
	
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/
	
protected: /* for derived classes only */

	void MergeFacetIntegral(int node_num, double weight, dArrayT& facetNormal, const dArrayT& phiValues,
						iArrayT& ip_cover, iArrayT& ip_cover_key, ArrayT< LinkedListT<int> >& nodeWorkSpace, 
						ArrayT< LinkedListT<dArrayT> >& facetWorkSpace,
						ArrayT< LinkedListT<double> >& circumferentialWorkSpace);
	
	/** number of integration points per facet for cell volume boundary integration */
	int fNumIP; 

	const ElementSupportT* fElementSupport;

	/** global ID of the nodes in CellGeometryT::fNodalCoordinates */
	const iArrayT* fNodes;
	const dArray2DT* fNodalCoordinates;
	
	/** shape functions */
	MeshFreeNodalShapeFunctionT* fNodalShapes;
	
	/** helper monkey class -- has global and local numbering scheme */
	SCNIMFT* fscnimft;
	
	/** axisymmetric? -- false by default */
	bool qIsAxisymmetric;
	
};

/* accessor to the element support class */
inline const ElementSupportT& CellGeometryT::ElementSupport(void) const
{
#if __option(extended_errorcheck)
	if (!fElementSupport) ExceptionT::GeneralFail("CellGeometryT::ElementSupport", "pointer not set");
#endif
	return *fElementSupport;
}

} /* namespace Tahoe */

#endif /* _CELL_GEOMETRY_T */

