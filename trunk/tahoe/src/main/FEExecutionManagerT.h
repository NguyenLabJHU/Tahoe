/* $Id: FEExecutionManagerT.h,v 1.10 2002-12-02 09:50:13 paklein Exp $ */
/* created: paklein (09/21/1997) */

#ifndef _FE_EXECMAN_T_H_
#define _FE_EXECMAN_T_H_

/* base class */
#include "ExecutionManagerT.h"

/* direct members */
#include "IOBaseT.h"

namespace Tahoe {

/* forward declarations */
template <class TYPE> class ArrayT;
class iArrayT;
class OutputSetT;
class IOManager;
class FEManagerT;
class PartitionT;
class ModelManagerT;

/** class to handle file driven finite element simulations */
class FEExecutionManagerT: public ExecutionManagerT
{
public:

	/** decomposition methods */
	enum DecompTypeT {
        kGraph = 0, /**< partition based on connectivities. See FEExecutionManagerT::Decompose_graph. */
         kAtom = 1, /**< partition based on index. See FEExecutionManagerT::Decompose_atom. */
      kSpatial = 2  /**< partition based on position. See FEExecutionManagerT::Decompose_spatial. */
	};

	/** stream extraction operator */ 
	friend istream& operator>>(istream& in, FEExecutionManagerT::DecompTypeT& t);

	/** constructor */
	FEExecutionManagerT(int argc, char* argv[], char job_char, char batch_char,
		CommunicatorT& comm);

	/** prompt input files until "quit" */
	virtual void Run(void);

protected:

	/** add the command line option to the list. \returns true if the option was
	 * added, false otherwise. */
	virtual bool AddCommandLineOption(const char* str);

	/** remove the command line option to the list. \returns true if the option was
	 * removed, false otherwise. */
	virtual bool RemoveCommandLineOption(const char* str);

	/** overloaded */
	virtual void RunJob(ifstreamT& in, ostream& status);

	/** \name basic MP support */
	/*@{*/
	int Rank(void) const;
	int Size(void) const;
	/*@}*/

private:

	/** standard serial driver */
	void RunJob_serial(ifstreamT& in, ostream& status) const;
	
	/** parallel driver */
	void RunJob_parallel(ifstreamT& in, ostream& status) const;

	/** generate decomposition files */
	void RunDecomp_serial(ifstreamT& in, ostream& status) const;

	/** join parallel results files */
	void RunJoin_serial(ifstreamT& in, ostream& status) const;

	/** print message on exception */
	void Rewind(ifstreamT& in, ostream& status) const;

	/** extract the model file name from the stream */
	void GetModelFile(ifstreamT& in, StringT& model_file,
		IOBaseT::FileTypeT& format) const;

	/** \name generate decomposition data */
	/*@{*/
	/** name calls one of decomposition methods below based on user input */
	void Decompose(ifstreamT& in, int size, int decomp_type, const StringT& model_file,
		IOBaseT::FileTypeT format, const StringT& output_map_file) const;

	/** graph-based decomposition. Partition model based on the connectivites
	 * in the model files and those generated at run time. The actual
	 * decomposition is calculated by a FEManagerT_mpi. */
	void Decompose_graph(ifstreamT& in, int size, const StringT& model_file,
		IOBaseT::FileTypeT format, const StringT& output_map_file) const;

	/** "atom" decomposition. Partition model by dividing global list
	 * of coordinates into sequential, nearly equal length lists. The
	 * number of atoms per partition is \f$ \frac{N}{n_p} \f$ for
	 * all partitions except the last, which also includes any remainder.
	 * \f$ N \f$ is the total number nodes and \f$ n_p \f$ is the number
	 * of partitions. The partition for a given node is then given by
	 \f[
	 	p_i = floor \left( \frac{i n_p}{N} \right).
	 \f]
	 */
	void Decompose_atom(ifstreamT& in, int size, const StringT& model_file,
		IOBaseT::FileTypeT format, const StringT& output_map_file) const;

	/** spatial decomposition. Partition model based on a grid. */
	void Decompose_spatial(ifstreamT& in, int size, const StringT& model_file,
		IOBaseT::FileTypeT format, const StringT& output_map_file) const;
	/*@}*/

	/** returns true if a new decomposition is needed */
	bool NeedDecomposition(ifstreamT& in, const StringT& model_file,
		int size) const;

	/** returns true if the global output model file is not found */
	bool NeedModelFile(const StringT& model_file, IOBaseT::FileTypeT format) const;

	/** returns true if a new decomposition is needed */
	bool NeedOutputMap(ifstreamT& in, const StringT& map_file,
		int size) const;

	/** read the map of I/O ID to processor. Used only is the output is
	 * joined at run time. */
	void ReadOutputMap(ifstreamT& in, const StringT& map_file,
		iArrayT& map) const;

	/** set output map based on length of map. The map defines the output prcoessor
	 * for each OutputSetT.
	 * \param output_sets list of OutputSetT's
	 * \param output_map returns with the output processor for each OutputSetT
	 * \param size number of output processors. */
	void SetOutputMap(const ArrayT<OutputSetT*>& output_sets,
		iArrayT& output_map, int size) const;

	/** construct and return the local IOManager */
	IOManager* NewLocalIOManager(const FEManagerT* global_FEman,
		const iArrayT& output_map) const;
		
	/** \name write partial geometry files 
	 * \param partition partition information for the part to be written
	 * \param model_ALL ModelManagerT accessing the total model database
	 * \param partial_file path to the partial geometry file
	 */
	/*@{*/
	/** write partial file for the given format */
	void EchoPartialGeometry(const PartitionT& partition, ModelManagerT& model_ALL,
		const StringT& partial_file, IOBaseT::FileTypeT format) const;

	/** write partial model file in ExodusII format */
	void EchoPartialGeometry_ExodusII(const PartitionT& partition,
		ModelManagerT& model_ALL, const StringT& partial_file) const;

	/** write partial model file in TahoeII format */
	void EchoPartialGeometry_TahoeII(const PartitionT& partition,
		ModelManagerT& model_ALL, const StringT& partial_file) const;
	/*@}*/
};

} // namespace Tahoe 
#endif /* _FE_EXECMAN_T_H_ */
