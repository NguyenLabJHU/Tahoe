/* $Id: ParticleT.h,v 1.9.2.3 2003-02-23 02:40:26 paklein Exp $ */
#ifndef _PARTICLE_T_H_
#define _PARTICLE_T_H_

/* base class */
#include "ElementBaseT.h"

/* direct members */
#include "nVariArray2DT.h"
#include "InverseMapT.h"

namespace Tahoe {

/** forward declarations */
class iGridManagerT;
class CommManagerT;
class ParticlePropertyT;
class dSPMatrixT; //TEMP
class InverseMapT;

/** base class for particle types */
class ParticleT: public ElementBaseT
{
public:

	/** constructor */
	ParticleT(const ElementSupportT& support, const FieldT& field);

	/** destructor */
	~ParticleT(void);
	
	/** initialization */
	virtual void Initialize(void);

	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** NOT implemented. Returns an zero force vector */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);
			
	/** returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void) { return 0.0; };
	
	/** resgiter for writing output. Uses the uses sub-class implementations
	 * of ParticleT::GenerateOutputLabels to register the particle group for
	 * output. Sub-classes also need to implemented the WriteOutput method. */
	virtual void RegisterOutput(void);

	/** write output. ParticleT::WriteOutput only writes search grid statistics.
	 * Sub-classes are responsible for writing data for each particle, given the
	 * variables names returned by ParticleT::GenerateOutputLabels. */
	virtual void WriteOutput(void);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

	/** trigger reconfiguration */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** \name restart functions */
	/*@{*/
	/** write restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::ReadRestart implementation. */
	virtual void WriteRestart(ostream& out) const;

	/** read restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::WriteRestart implementation. */
	virtual void ReadRestart(istream& in);
	/*@}*/

	/** define the particles to skip. This is a list of nodes though "owned" 
	 * by this processor and appearing in the list of particles, should be skipped 
	 * in the calculation of energy, force, etc. This method must be called
	 * whenever the list changes as it is used to reset some internal data. An
	 * empty list should be passed to clear any previous lists. The list may
	 * contain duplicates. Note this method does not trigger recalculation of
	 * the neighborlists. This can be triggered explicitly with a call to
	 * ParticleT::SetConfiguration */
	void SetSkipParticles(const iArrayT& skip);

	/** set neighborlists and any other system configuration information
	 * based on the current information. ParticleT::SetConfiguration
	 * simply stores a list of coordinates in the current configuration
	 * which is used to determine if SetConfiguration needs to be called
	 * again. */
	virtual void SetConfiguration(void);

	/** compute the part of the stiffness matrix */
	virtual void FormStiffness(const InverseMapT& col_to_col_eq_row_map,
		const iArray2DT& col_eq, dSPMatrixT& stiffness) = 0;

protected: /* for derived classes only */

	/** echo element connectivity data. Reads parameters that define
	 * which nodes belong to this ParticleT group. */
	virtual void EchoConnectivityData(ifstreamT& in, ostream& out);
	
	/** generate labels for output data */
	virtual void GenerateOutputLabels(ArrayT<StringT>& labels) const = 0;

	/** return true if connectivities are changing */
	virtual bool ChangingGeometry(void) const;

	/** generate neighborlist
	 * \param particle_tags global tags for which to determine neighhors. If NULL, find
	 *        neighbors for all nodes.
	 * \param distance distance over which to search for neighbors
	 * \param neighbors list of neighbors for every tag in particle_tags as rows in the
	 *        2D array. The list for each tag begins with the tag itself. Therefore, all
	 *        lists are at least length 1. The numbering of the tags in the neighbor list
	 *        is in terms of the tags provided in particle_tags.
	 * \param double_list if true the neighbor lists will contain two references for
	 *        every neighbor interaction. For tags A and B, this means B will appear
	 *        in the neighbor list for A and vice versa. If false, neighbor lists will
	 *        only contain neighbors for which A > B.
	 * \param full_list if double_list is false, passing full_list true will add B to
	 *        the neighbor list of A regardless of which tag is higher in order to
	 *        produce a full list of interactions for A. If double_list is true,
	 *        this flag has no effect. */
	void GenerateNeighborList(const ArrayT<int>* particle_tags, double distance, 
		RaggedArray2DT<int>& neighbors, bool double_list, bool full_list);

	/** construct the list of properties from the given input stream */
	virtual void EchoProperties(ifstreamT& in, ofstreamT& out) = 0;

	/** assemble particle mass matrix into LHS of global equation system
	 * \param mass mass associated with each particle type */
	void AssembleParticleMass(const dArrayT& mass);

	/** return the maximum distance between the current coordinates and those
	 * stored in ParticleT::fReNeighborCoords. The maximum is over local atoms
	 * only. */
	double MaxDisplacement(void) const;

protected:

	/** reference ID for sending output */
	int fOutputID;
	
	/** communications manager */
	const CommManagerT& fCommManager;

	/** \name local to global tag map.
	 * Used for things like neighbor lists */
	/*@{*/
	/** list of node set IDs define particles in this group. If ALL
	 * nodes are particles, this will be length 1 with fID[0] = "ALL" */
	ArrayT<StringT> fID;
	
	/** connectivities used to define the output set. Just an alias to the
	 * ParticleT::fGlobalTag. */
	iArray2DT fPointConnectivities;
	/*@}*/

	/** the neighboring cut-off distance */
	double fNeighborDistance;

	/** maximum distance an atom can displace before the neighbor lists are reset */
	double fReNeighborDisp;

	/** maximum number of steps between reseting neighbor lists */
	int fReNeighborIncr;

	/** \name particle properties */
	/*@{*/
	/** number of types. Read during ParticleT::EchoConnectivityData. */
	int fNumTypes;

	/** particle type for global tag */
	AutoArrayT<int> fType;
	
	/** message id for exchanging the ParticleT::fType array */
	int fTypeMessageID;
	
	/** map of particle types to properties: {type_a, type_b} -> property number */
	nMatrixT<int> fPropertiesMap;

	/** particle properties list */
	ArrayT<ParticlePropertyT*> fParticleProperties;
	
	/** list of active particles. The pointer will be NULL if all nodes in the
	 * CommManagerT::PartitionNodes list are active. */
	AutoArrayT<int>* fActiveParticles;
	/*@}*/

	/** \name cached calculated values */
	/*@{*/
//	dArrayT   fEnergy;
	dArray2DT fForce;
	nVariArray2DT<double> fForce_man;
	/*@{*/
	
	/** \name group running averages.
	 * Values are averages over {n1, n2,...,nN} steps */
	/*@{*/
//	iArrayT fAverages;
//	dArrayT fKE_avg;
//	dArrayT fPE_avg;
	/*@}*/
	
	/** the search grid */
	iGridManagerT* fGrid;

	/** coordinates of partition nodes at the last re-neighbor. The list of coordinares
	 * is collected during ParticleT::SetConfiguration */
	dArray2DT fReNeighborCoords;
	double fDmax;  /**< maximum distance between the current
	                    coordinates and the coordinates in ParticleT::fReNeighborCoords.
	                    This value is computed during ParticleT::RelaxSystem. */

private:

	/** count between resetting neighbor lists */
	int fReNeighborCounter;
};

} /* namespace Tahoe */

#endif /* _PARTICLE_T_H_ */
