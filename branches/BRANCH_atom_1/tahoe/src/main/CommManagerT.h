/* $Id: CommManagerT.h,v 1.1.2.2 2002-12-10 17:13:02 paklein Exp $ */
#ifndef _COMM_MANAGER_T_H_
#define _COMM_MANAGER_T_H_

/* direct members */
#include "dArray2DT.h"
#include "iArrayT.h";

namespace Tahoe {

/* forward declarations */
class PartitionT;
class CommunicatorT;
class ModelManagerT;

/** manage processor to processor transactions. Manages partition information.
 * Creates ghosts nodes. Manages communication lists. Manipulates the
 * ModelManagerT to create the environment for the local processor. */
class CommManagerT
{
public:

	/** constructor */
	CommManagerT(CommunicatorT& comm, ModelManagerT& model_manager);

	/** set partition information */
	void SetPartition(PartitionT* partition) { fPartition = partition; };

	/** \name setting periodic boundaries */
	/*@{*/
	void SetPeriodicBoundaries(int i, double x_i_min, double x_i_max);
	void ClearPeriodicBoundaries(int i);
	/*@}*/

	/** configure the current local coordinate list and register it with the
	 * model manager. The first time this method is called, it will call
	 * CommManagerT::FirstConfigure before performing the usual operations. */
	void Configure(void);

	/** the local node to home processor map */
	const iArrayT& ProcessorMap(void) const { return fProcessor; };

private:

	/** perform actions needed the first time CommManagerT::Configure is called. */
	void FirstConfigure(void);

	/** determine the local coordinate bounds 
	 * \param coords coordinate list
	 * \param local rows of coords to use in determining bounds
	 * \param bounds returns with the bounds of the given points */
	void GetBounds(const dArray2DT& coords, const iArrayT& local, dArray2DT& bounds) const;

private:

	/** communicator */
	CommunicatorT& fComm;

	/** the model manager */
	ModelManagerT& fModelManager;

	/** \name periodic boundaries */
	/*@{*/
	/** flags to indicate if periodic boundary conditions are imposed */
	ArrayT<bool> fIsPeriodic;
	
	/** rows give the lower and upper periodic bounds for that coordinate */
	dArray2DT fPeriodicBoundaries;
	/*@}*/
	
	/** processor bounds */
	dArray2DT fBounds;

	/** partition information */
	PartitionT* fPartition;
	
	/** true if CommManagerT::Configure has not been called yet */
	bool fFirstConfigure;

	/** native processor per node */
	iArrayT fProcessor;
};

} /* namespace Tahoe */

#endif /* _COMM_MANAGER_T_H_ */
