/* $Id: InterpolationDataT.h,v 1.2 2004-03-04 08:54:20 paklein Exp $ */
#ifndef _INTERPOLATION_DATA_T_H_
#define _INTERPOLATION_DATA_T_H_

/* direct members */
#include "RaggedArray2DT.h"
#include "InverseMapT.h"

namespace Tahoe {

/** collection of information for interpolating data */
class InterpolationDataT
{
public:

	/** constructor */
	InterpolationDataT(void) { };

	/** \name accessors */
	/*@{*/
	/** neighbors for each entry in InterpolationDataT::Map */
	RaggedArray2DT<int>& Neighbors(void) { return fNeighbors; };
	const RaggedArray2DT<int>& Neighbors(void) const { return fNeighbors; };

	/** interpolation weights */
	RaggedArray2DT<double>& NeighborWeights(void) { return fNeighborWeights; };
	const RaggedArray2DT<double>& NeighborWeights(void) const { return fNeighborWeights; };

	/** map of global number to corresponding row in InterpolationDataT::Neighbors
	 * and InterpolationDataT::NeighborWeights */
	InverseMapT& Map(void) { return fMap; };
	const InverseMapT& Map(void) const { return fMap; };
	/*@}*/

	/** transpose the given InterpolationDataT */
	void Transpose(const InverseMapT& map, RaggedArray2DT<int>& neighbors,
		RaggedArray2DT<double>& neighbor_weights);

private:

	/** map of global number to corresponding row in InterpolationDataT::fNeighbors
	 * and InterpolationDataT::fNeighborWeights */
	InverseMapT fMap;
	
	/** neighbors for each entry in InterpolationDataT::fMap */
	RaggedArray2DT<int> fNeighbors;

	/** interpolation weights */
	RaggedArray2DT<double> fNeighborWeights;	
};

} /* namespace Tahoe */

#endif /* _INTERPOLATION_DATA_T_H_ */
