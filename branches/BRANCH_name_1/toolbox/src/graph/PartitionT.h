/* $Id: PartitionT.h,v 1.6.4.1 2002-06-27 18:01:09 cjkimme Exp $ */
/* created: paklein (11/16/1999) */

#ifndef _PARTITION_T_H_
#define _PARTITION_T_H_

/* direct members */
#include "iArrayT.h"
#include "StringT.h"

/* forward declarations */

namespace Tahoe {

class GraphT;
class iArray2DT;
class dArray2DT;
class ifstreamT;
template <class TYPE> class RaggedArray2DT;

/** graph partition information (following NEMESIS data model)
 * class generates complete decomposition information using
 * a node-to-partition map and a graph. initialization is in
 * 3 stages:\n
 * (1) Set(,,) to set internal nodal data\n
 * (2) [cross-link] to complete communication maps\n
 * (3) [tolocal] to convert to partition-local numbering */
class PartitionT
{
public:

	enum NumberScopeT {kUnSet = 0,
	                   kLocal = 1,
	                  kGlobal = 2};

	/* constructor */
	PartitionT(void);

	/* returns true if version if current */
	static bool CheckVersion(const StringT& version);
	
	/** return the comment character used for partition files */
	static char CommentMarker(void) { return '#'; };
	
	/* accessors */
	int ID(void) const;
	NumberScopeT NumberScope(void) const;

	const iArrayT& Nodes_Internal(void) const;
	const iArrayT& Nodes_Border(void) const;
	const iArrayT& Nodes_External(void) const;
	void PartitionNodes(iArrayT& nodes, NumberScopeT scope) const;

	const iArrayT& CommID(void) const;
	const iArrayT* NodesIn(int commID) const;  // NULL if ID not found
	const iArrayT* NodesOut(int commID) const; // NULL if ID not found

	/* set partition data */
	void Set(int num_parts, int id, const iArrayT& part_map, const GraphT& graph);
	void Set(int num_parts, int id, const iArrayT& part_map, const ArrayT<const iArray2DT*>& connects_1,
		const ArrayT<const RaggedArray2DT<int>*>& connects_2);
	void SetOutgoing(const ArrayT<iArrayT>& nodes_in); // in sequence of CommID
	void SetScope(NumberScopeT scope);

	int NumElementBlocks(void) const;
	const ArrayT<StringT>& BlockID(void) const { return fElementBlockID; };
	void InitElementBlocks(const ArrayT<StringT>& blockID);	
	void SetElements(const StringT& blockID, const iArray2DT& connects);

	/* check cross-references - returns 1 if OK */
	int CrossCheck(const PartitionT& that) const;

	/* I/O */
	friend ifstreamT& operator>>(ifstreamT& in, PartitionT& partition) { return partition.Read(in); };
	friend ostream& operator<<(ostream& out, const PartitionT& partition);

	/* operator support */
	ifstreamT& Read(ifstreamT& in);

	/* maps */
	const iArrayT& NodeMap(void) const;
	const iArrayT& InverseNodeMap(int& index_shift) const;
	const iArrayT& ElementMap(const StringT& blockID) const;

	/* returns indeces of global nodes that lie within the partition */
	void ReturnPartitionNodes(const iArrayT& global_nodes,
		iArrayT& partition_indices) const;

	/* returns indeces of (block) global elements that lie within
	 * the partition */
	void ReturnPartitionElements(const StringT& blockID, const iArrayT& global_elements,
		iArrayT& partition_indices) const;

	/* mapping functions (assumes scope is currently the opposite) */
	void SetNodeScope(NumberScopeT scope, ArrayT<int>& nodes) const;
	void SetElementScope(NumberScopeT scope, const StringT& blockID, ArrayT<int>& elements) const;

	/* input operator for scope */
	friend istream& operator>>(istream& in, PartitionT::NumberScopeT& scope);

private:

	/* node and element classifications */
	enum StatusT {   kUnset =-1,
	              kInternal = 0,
	                kBorder = 1,
	              kExternal = 2};
	//there are actually 4 types -> internal,
	//                              border-internal
	//                              border-external
	//                              external

	/* resolve element block ID to index */
	int ElementBlockIndex(const StringT& blockID, const char* caller = NULL) const;

	/* number transformations */
	void MapValues(const iArrayT& map, int shift, ArrayT<int>& values) const;

	/* make inverse map (filled with -1) */
	void MakeInverseMap(const iArrayT& map, iArrayT& inv_map, int& shift) const;

	/* classify set nodes as label nodes as internal, external, or border */
	void ClassifyNodes(const iArrayT& part_map, const GraphT& graph);
	void ClassifyNodes(const iArrayT& part_map, const ArrayT<const iArray2DT*>& connects_1,
		const ArrayT<const RaggedArray2DT<int>*>& connects_2);

	/* set receiving nodes/partition information */
	void SetReceive(const iArrayT& part_map);

	/* map status of (in range) parts into status_map */
	void MapStatus(StatusT status, const iArrayT& part, ArrayT<StatusT>& status_map,
		int offset);

	/* set numbering maps */
	void SetNodeMap(NumberScopeT scope, iArrayT& map, int& shift) const;
	void SetElementMap(NumberScopeT scope, const StringT& blockID, iArrayT& map, int& shift) const;

private:
	
	int fNumPartitions; // total number of partitions
	int fID;            // partition number
	NumberScopeT fScope; // local or global numbering
	
	// nodal information
	iArrayT fNodes_i; // internal nodes	
	iArrayT fNodes_b; // border nodes	
	iArrayT fNodes_e; // external nodes
	
	// receive/send information
	iArrayT fCommID; // ID's of communicating partitions (only)
	ArrayT<iArrayT> fNodes_in;  // nodes per comm part
	ArrayT<iArrayT> fNodes_out; // nodes per comm part
	
	// element information
	ArrayT<StringT> fElementBlockID;
	ArrayT<iArrayT> fElements_i; // internal elements per block
	ArrayT<iArrayT> fElements_b; // border elements per block

	// global node map
	iArrayT fNodeMap; // global[local] for all _i, _b, _e nodes
	int fNodeMapShift;
	iArrayT fInvNodeMap;
	
	// block global element numbering (number within global blocks)
	ArrayT<iArrayT> fElementMap; // block_global[block_local]
	iArrayT fElementMapShift;
	ArrayT<iArrayT> fInvElementMap;	
};

/* inlines */
inline int PartitionT::ID(void) const { return fID; }
inline PartitionT::NumberScopeT PartitionT::NumberScope(void) const { return fScope; }

inline const iArrayT& PartitionT::Nodes_Internal(void) const { return fNodes_i; }
inline const iArrayT& PartitionT::Nodes_Border(void) const { return fNodes_b; }
inline const iArrayT& PartitionT::Nodes_External(void) const { return fNodes_e; }

inline 	const iArrayT& PartitionT::CommID(void) const { return fCommID; }

inline int PartitionT::NumElementBlocks(void) const { return fElementMap.Length(); }

/* maps */
inline const iArrayT& PartitionT::NodeMap(void) const { return fNodeMap; }
inline const iArrayT& PartitionT::ElementMap(const StringT& blockID) const
{
	return fElementMap[ElementBlockIndex(blockID, "ElementMap")];
}

} // namespace Tahoe 
#endif /* _PARTITION_T_H_ */