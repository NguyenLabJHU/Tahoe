/* $Id: BridgingScaleT.h,v 1.23.6.1 2003-05-12 16:34:24 paklein Exp $ */
#ifndef _BRIDGING_SCALE_T_H_
#define _BRIDGING_SCALE_T_H_

/* base class */
#include "SolidElementT.h"

/* direct members */
#include "RaggedArray2DT.h"
#include "ElementMatrixT.h"
#include "CCSMatrixT.h"

namespace Tahoe {

/* forward declarations */
class PointInCellDataT;

/** class to handle interpolation of data from the nodes to an arbitrary set of
 * points and from a arbitrary set of points into the nodes. */
class BridgingScaleT: public ElementBaseT
{
public:

	/** constructor */
	BridgingScaleT(const ElementSupportT& support, const FieldT& field,
		const SolidElementT& solid);

	/** \name field interpolation */
	/*@{*/
	/** initialize interpolation data. Calculate and store interpolation data
	 * within the given PointInCellDataT. Maps points into the initial or current 
	 * configuration of the mesh. Only one of the two pointers must be passed as NULL.
	 * \param points_used indecies of points in the coordinate lists which should be
	 *        mapped into cells
	 * \param init_coords point to the initial coordinates array if available
	 * \param curr_coords point to the initial coordinates array if available
	 * \param point_in_cell destination for map data
	 */
	void InitInterpolation(const iArrayT& points_used, const dArray2DT* init_coords, 
		const dArray2DT* curr_coords, PointInCellDataT& cell_data);

	/** interpolate field.
	 * \param field name of field to interpolate
	 * \param cell_data interpolation data generated by first mapping points into
	 *        the mesh with BridgingScaleT::MaptoCells and then initializing the
	 *        interpolation data with BridgingScaleT::InitInterpolation. 
	 * \param returns with the interpolated values of the field */
	void InterpolateField(const StringT& field, const PointInCellDataT& cell_data,
		dArray2DT& point_values) const;
	/*@}*/

	/** \name project external field onto mesh */
	/*@{*/
	/** initialize projection data. Maps points into the initial or current 
	 * configuration of the mesh. Only one of the two pointers must be passed as NULL.
	 * \param points_used indecies of points in the coordinate lists which should be
	 *        mapped into cells
	 * \param init_coords point to the initial coordinates array if available
	 * \param curr_coords point to the initial coordinates array if available
	 * \param point_in_cell destination for map data
	 */
	virtual void InitProjection(const iArrayT& points_used, const dArray2DT* init_coords, 
		const dArray2DT* curr_coords, PointInCellDataT& cell_data);

	/** project the point values onto the mesh. Requires a previous call to
	 * BridgingScaleT::InitProjection in order to compute the mass-like
	 * matrix. */
	virtual void ProjectField(const PointInCellDataT& cell_data,
		const dArray2DT& point_values, dArray2DT& projection);
	/*@}*/

	/** register self for output */
	virtual void RegisterOutput(void);

	/** send output */
	virtual void WriteOutput(void);

	/** form of tangent matrix, symmetric by default */
	virtual GlobalT::SystemTypeT TangentType(void) const { return GlobalT::kSymmetric; };

	/** accumulate the residual force on the specified node
	 * \param node test node
	 * \param force array into which to assemble to the residual force */
	virtual void AddNodalForce(const FieldT&, int, dArrayT&) {};

	/** returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void) { return 0; };

	/** compute specified output parameter and send for smoothing */
	virtual void SendOutput(int) {};

protected:

	/** map points into cells in the group's connectivities.
	 * Map points into the initial or current configuration of the mesh. Only
	 * one of the two pointers must be passed as NULL.
	 * \param points_used indecies of points in the coordinate lists which should be
	 *        mapped into cells
	 * \param init_coords point to the initial coordinates array if available
	 * \param curr_coords point to the initial coordinates array if available
	 * \param point_in_cell destination for map data
	 */
	void MaptoCells(const iArrayT& points_used, const dArray2DT* init_coords, 
		const dArray2DT* curr_coords, PointInCellDataT& cell_data);

	/** continuum shape functions */
	const ShapeFunctionT& ShapeFunction(void) const;

	/** element coordinates.
	 * \return initial nodal coordinates of current element: [nen] x [nsd] */
	const LocalArrayT& InitialCoordinates() const;
	
	/** element displacements.
	 * \return nodal displacements of current element: [nen] x [ndof] */
	const LocalArrayT& Displacements() const;

	/** echo element connectivity data. No connectivities need to be read */
	virtual void EchoConnectivityData(ifstreamT&, ostream&) {};

	/* called by FormRHS and FormLHS */
	virtual void LHSDriver(GlobalT::SystemTypeT) {};
	virtual void RHSDriver(void) {};

	/** allocate and initialize local arrays */
	virtual void SetLocalArrays(void);

	/** write element group parameters to out */
	virtual void PrintControlData(ostream& out) const;

	/** write all current element information to the stream. used to generate
	 * debugging information after runtime errors */
	virtual void CurrElementInfo(ostream& out) const;

protected:

	/** continuum group solving displacements */
	const SolidElementT& fSolid;
	iArray2DT fSolidNodesUsed;

	/** list of particles per element: [n_cell] x [n_part_i] */
	RaggedArray2DT<int> fParticlesInCell;
	
	/** take fParticlesInCell, now have list of inverse mappings per element:
	 *  [n_cell] x [n_inversemap_i] */
	RaggedArray2DT<double> fInverseMapInCell;

	/** coarse scale field at the source points. The projected field interpolated
	 * to the source points. */
	dArray2DT fCoarseScale;

	/** fine scale field at the source points. The difference in the source field
	 * and projected field interpolated to the source points. */
	dArray2DT fFineScale;

	/** Nodal degrees of freedom */
	dArrayT fUx, fUy;
	dMatrixT fWtempU;
	dArray2DT fFineScaleU;

	int fTotalNodes;
	iArray2DT fConnect, fAtomConnect;
	ElementMatrixT fElMatU;
	CCSMatrixT fGlobalMass;

	/* output control */
	iArrayT	fNodalOutputCodes;
	iArrayT	fElementOutputCodes;
	
	/* arrays with local ordering */
	LocalArrayT	fLocInitCoords;	/**< initial coords with local ordering */
	LocalArrayT fLocDisp;	    /**< displacements with local ordering  */ 
	
	/* work space */
	dArrayT fNEEvec; /**< work space vector: [element DOF] */
	dArrayT fDOFvec; /**< work space vector: [nodal DOF]   */
//	dArrayT fNSDvec; /**< work space vector: [nodal dim]   */

private:

	/** \name output ID's */
	/*@{*/
	int fParticleOutputID;
	int fSolidOutputID;
	/*@}*/
};

/* inlines */

/* accessors */
inline const ShapeFunctionT& BridgingScaleT::ShapeFunction(void) const
{
	return fSolid.ShapeFunction();
}

inline const LocalArrayT& BridgingScaleT::InitialCoordinates() const
{	
	return fLocInitCoords;
}

inline const LocalArrayT& BridgingScaleT::Displacements() const
{
	return fLocDisp;
}

} // namespace Tahoe 
#endif /* _BRIDGING_SCALE_T_H_ */
