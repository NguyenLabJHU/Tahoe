/* $Id: JoinOutputT.cpp,v 1.2 2002-01-09 12:39:20 paklein Exp $ */
/* created: paklein (03/24/2000) */

#include "JoinOutputT.h"

#include "fstreamT.h"
#include "IOManager.h"
#include "ModelManagerT.h"
#include "OutputSetT.h"
#include "StringT.h"
#include "ExodusT.h"
#include "iArray2DT.h"
#include "nVariArray2DT.h"
#include "VariArrayT.h"
#include "dArrayT.h"

/* constructor */
JoinOutputT::JoinOutputT(const StringT& param_file, const StringT& model_file,
	IOBaseT::FileTypeT model_file_type, IOBaseT::FileTypeT results_file_type, int size):
	fJobFile(param_file),
	fResultsFileType(results_file_type),
	fPartitions(size),
	fIO(NULL)
{
	/* set model database manager */
	fModel = new ModelManagerT(cout);
	if (!fModel->Initialize(model_file_type, model_file)) {
		cout << "\n JoinOutputT::JoinOutputT: error opening geometry file: " 
		     << fModel->DatabaseName() << endl;
		throw eDatabaseFail;
	}

	/* read partition data */
	for (int i = 0; i < fPartitions.Length(); i++)
	{
		/* file name */
		StringT file;
		file.Root(model_file);
		file.Append(".n", fPartitions.Length());
		file.Append(".part", i);

		/* open stream */
		ifstreamT part_in(file);
		if (!part_in.is_open())
		{
			cout << "\n JoinOutputT::JoinOutputT: could not open decomposition file: "
			     << part_in.filename() << endl;
			throw eGeneralFail;
		}
		
		/* read data */
		part_in >> fPartitions[i];
		
		/* set numbering scope */
		fPartitions[i].SetScope(PartitionT::kLocal);
	}
	
	/* set output */
	SetOutput();
	
	/* set assembly maps */
	SetMaps();
}
	
/* destructor */
JoinOutputT::~JoinOutputT(void)
{
	delete fModel;
	delete fIO;
	
	fIO = NULL;
	fModel = NULL;
}

/* do join */
void JoinOutputT::Join(void)
{
	/* assembly work space */
	dArray2DT part_n_values;
	dArray2DT part_e_values;
	nVariArray2DT<double> part_n_man(0, part_n_values, 0);
	nVariArray2DT<double> part_e_man(0, part_e_values, 0);	

	/* output sets data */
	const ArrayT<OutputSetT*>& element_sets = fIO->ElementSets();
	for (int i = 0; i < element_sets.Length(); i++)
	{
		cout << "\n JoinOutputT: output set: " << i+1 << endl;
	
		/* set data */
		const OutputSetT& output_set = *(element_sets[i]);
		const MapSetT& map_set = fMapSets[i];
	
		/* assembled values */
		dArray2DT all_n_values(output_set.NumNodes(), output_set.NumNodeValues());
		dArray2DT all_e_values;
		if (output_set.BlockID().Length() == 0 && output_set.NumElementValues() > 0) //outdated - this should not occur
			cout << "\n JoinOutputT::Join: skipping element output\n" << endl;
		else
			all_e_values.Allocate(output_set.NumElements(), output_set.NumElementValues());

		/* non-empty output */
		if (all_n_values.Length() > 0 || all_e_values.Length() > 0)
		{
			/* number of output steps */
			int num_steps = NumOutputSteps(i);
			cout << " JoinOutputT:      steps: " << num_steps << endl;
			int d_width = cout.precision() + kDoubleExtra;
			cout << setw(kIntWidth) << "step"
			     << setw(d_width)   << "time" << '\n';
			
			/* loop over steps */
			for (int j = 0; j < num_steps; j++)
			{
				/* initialize */
				all_n_values = 0.0;
				all_e_values = 0.0;
			
				/* loop over partitions */
				double time;
				bool found_time = false;
				for (int k = 0; k < fPartitions.Length(); k++)
				{
					/* file name */
					StringT filename;
					ResultFileName(k, i, filename);
					
					/* check if file is present */
					if (fstreamT::Exists(filename))
					{
						/* open the database file */
						ModelManagerT results(cout);
						if (!results.Initialize(fResultsFileType, filename)) {
							cout << "\n JoinOutputT::Join: error opening partial results file \""
							     << results.DatabaseName() << '\"' << endl;
							throw eDatabaseFail;
						}

						/* get time */
						if (!found_time)
						{
							found_time = true;
							dArrayT steps;
							results.TimeSteps(steps);
							time = steps[j];
							cout << setw(kIntWidth) << j+1
							     << setw(d_width)   << time << endl;
						}
					
						/* assemble nodal values */
						if (all_n_values.Length() > 0)
						{
							/* dimensions */
							int num_nodes = results.NumNodes();
						
							/* assembly map: output_set_ID[partition_ID] */
							const iArrayT& node_map = map_set.NodeMap(k);

							/* very weak consistency check */
							if (node_map.Length() > num_nodes)
							{
								cout << "\n JoinOutputT::Join: assembly map of nodal values (" << node_map.Length() 
								     << ") is longer than\n" 
								     <<   "     is longer than the number of nodes (" << num_nodes
								     << ") in partial results file:\n"
								     <<   "     " << results.DatabaseName() << endl;
								throw eSizeMismatch;
							}
							
							/* set work space */
							part_n_man.Dimension(num_nodes, all_n_values.MinorDim());

							/* read data */
							results.AllNodeVariables (j, part_n_values);

							/* assemble */
							all_n_values.Assemble(node_map, part_n_values);
						}					

						/* assemble element values */
						if (all_e_values.Length() > 0)
						{
							/* element map: global_output_block_ID[partition_output_block_ID] */
							const iArrayT& element_map = map_set.ElementMap(k);

							/* dimension check */
							if (output_set.NumElements() != element_map.Length())
							{
								cout << "\n JoinOutputT::Join: number of elements (" << output_set.NumElements() 
								     << ") in the partial results\n"
								     << "     file does not match the number of elements (" << element_map.Length() 
								     << ") expected for output\n"
								     << "     set " << i << " in partition " << k << endl;
								throw eSizeMismatch;
							}

							/* set work space */
							part_e_man.Dimension(element_map.Length(), all_e_values.MinorDim());

							/* block ID's in the set */
							const iArrayT& block_ID = output_set.BlockID();
							
							/* read data by block - one block after the next */
							int row_offset = 0;
							for (int l = 0; l < block_ID.Length(); l++)
							{
								/* block name */
								StringT block_name;
								block_name.Append(block_ID[l]);
								
								/* block index */
								int block_index = results.ElementGroupIndex(block_name);
								
								/* block dimensions */
								int nel, nen;
								results.ElementGroupDimensions(block_index, nel, nen);
							
								/* alias */
								dArray2DT block_values(nel, all_e_values.MinorDim(), all_e_values(row_offset));
								row_offset += nel;
								
								/* read variables */
								results.ElementVariables(j, block_name, block_values);
							}
							
							/* assemble */
							all_e_values.Assemble(element_map, part_e_values);
						}
					}
				}

				/* write assembled data */
				fIO->SetOutputTime(time);
				fIO->WriteOutput(i, all_n_values, all_e_values);
			}	
		}
	}
}

/*************************************************************************
* Private
*************************************************************************/

/* set output */
void JoinOutputT::SetOutput(void)
{
	/* construct I/O */
	StringT program_name("tahoe");
	StringT nothing("none");
	fIO = new IOManager(cout, program_name, nothing, nothing, fJobFile, fResultsFileType);
	if (!fIO) throw eOutOfMemory;
	
	/* set coordinates */
	fIO->SetCoordinates(fModel->Coordinates(), NULL);
	
	/* block ID's in io groups */
	StringT io_file;
	io_file.Root(fJobFile);
	io_file.Append(".io.ID");
	ifstreamT io('#', io_file);
	if (!io.is_open()) {
		cout << "\n JoinOutputT::SetOutput: error opening: \"" << io_file << "\"\n"
		     <<   "     file lists the element block ID's in each output ID:\n"
		     <<   "        [output ID] [num blocks] [list of block ID's]" << endl;
		throw eGeneralFail;
	}

	/* construct output sets for each ID */
	int ID, count = 0;
	io >> ID;
	while (io.good())
	{
		int num_ID = -99;
		io >> num_ID;
		iArrayT block_ID(num_ID);
		io >> block_ID;
	
		/* get output labels */
		ArrayT<StringT> n_labels;
		ArrayT<StringT> e_labels;
		OutputLabels(ID, n_labels, e_labels);
		
		/* collect element blocks */
		GeometryT::CodeT geometry_code;
		ArrayT<const iArray2DT*> connects_list(block_ID.Length());
		for (int i = 0; i < block_ID.Length(); i++)
		{
			/* block ID as string */
			StringT block_name;
			block_name.Append(block_ID[i]);
			
			/* element group index */
			int index = fModel->ElementGroupIndex(block_name);
			if (index < 0) {
				cout << "\n JoinOutputT::SetOutput: error trying to read element block ID " 
				     << block_ID[i] << endl;
				throw eDatabaseFail;
			}
			
			/* geometry code */
			geometry_code = fModel->ElementGroupGeometry(index);
			
			/* load element group */
			 const iArray2DT& connects = fModel->ElementGroup(index);
			 connects_list[i] = &connects;
		}

		/* construct output set */
		bool changing = false; // changing geometry not supported
		OutputSetT output_set(++count, geometry_code, block_ID, connects_list, n_labels, e_labels, changing);
	
		/* register */
		fIO->AddElementSet(output_set);
	
		/* next block */
		io >> ID;
	}
}

/* set assembly maps
 *    nodes: map partition local number -> output set local number 
 * elements: ??? */
void JoinOutputT::SetMaps(void)
{
	/* check */
	if (!fIO) throw eGeneralFail;

	/* output sets data */
	const ArrayT<OutputSetT*>& element_sets = fIO->ElementSets();

	/* dimensions */
	int num_parts = fPartitions.Length();
	int num_sets  = element_sets.Length();

	/* global to set maps */
	fMapSets.Allocate(num_sets);
	iArrayT shift(num_sets);
	ArrayT<iArrayT> inv_global(num_sets);
	for (int i = 0; i < num_sets; i++)
	{
		/* set data */
		OutputSetT& output_set = *(element_sets[i]);

		/* global nodes used by the set */
		const iArrayT& global_nodes_used = output_set.NodesUsed();

		/* global to set map */
		SetInverseMap(global_nodes_used, inv_global[i], shift[i], -1);
		
		/* allocate maps set */
		MapSetT& map_set = fMapSets[i];
		int n_sets = (output_set.NumNodeValues()    > 0) ? num_parts : 0;
		int e_sets = (output_set.NumElementValues() > 0) ? num_parts : 0;
		map_set.Allocate(n_sets, e_sets);
	}

	/* resident partition for each node */
	iArrayT node_part_map;
	SetNodePartitionMap(node_part_map);

	/* construct nodal assembly maps (loops reversed) */
	for (int j = 0; j < num_parts; j++)
	{
		for (int i = 0; i < num_sets; i++)
		{
			/* set data */
			OutputSetT& output_set = *(element_sets[i]);
			MapSetT& map_set = fMapSets[i];
	
			/* non-empty nodal output */
			if (map_set.NumNodeMaps() > 0)
			{
				/* resident partition nodes in set */
				iArrayT nodes;
				PartitionSetNodes(j, node_part_map, output_set.NodesUsed(), nodes);

				/* set output assembly map */
				SetAssemblyMap(inv_global[i], shift[i], nodes, map_set.NodeMap(j));
			}			
		}
	}
	
	/* element assembly maps */
	for (int i = 0; i < num_sets; i++)
	{
		/* output set data */
		const OutputSetT& output_set = *(element_sets[i]);
		MapSetT& map_set = fMapSets[i];
		if (map_set.NumElementMaps() > 0)
		{
			const iArrayT& block_ID = output_set.BlockID();
			if (block_ID.Length() > 0)
			{
				/* get block sizes from max element number 
				 * element may be duplicated across partitions */
				iArrayT block_size(block_ID.Length());
				block_size = 0;
				for (int j = 0; j < num_parts; j++)
					for (int k = 0; k < block_ID.Length(); k++)
					{
						const iArrayT& element_map = fPartitions[j].ElementMap(block_ID[k]);
						if (element_map.Length() > 0)
						{
							int max_element = element_map.Max();
						
							/* find max across parts */
							block_size[k] = (max_element > block_size[k]) ?  
								max_element : block_size[k];
						}
					}
				
				/* max number is one less than size */
				block_size += 1;

				/* check total against output size */
				if (block_size.Sum() != output_set.NumElements())
				{
					cout << "\n JoinOutputT::SetMaps: expecting " << output_set.NumElements() 
					     << " elements for output ID " << i << ",\n" 
					     << "     found " << block_size.Sum() 
					     << " counting by block in partitions" << endl;
					throw eSizeMismatch;
				}
				
				/* loop over partitions */
				for (int n = 0; n < num_parts; n++)
				{
					/* element assembly map */
					iArrayT& element_map = map_set.ElementMap(n);
				
					/* map size - no duplicated elements within
					 * a partition */
					int num_elems = 0;
					for (int j = 0; j < block_ID.Length(); j++)
						num_elems += fPartitions[n].ElementMap(block_ID[j]).Length();
						
					/* allocate map */
					element_map.Allocate(num_elems);
					element_map = -1;
					
					/* fill map */
					int offset = 0;
					int dex = 0;
					for (int k = 0; k < block_ID.Length(); k++)
					{
						const iArrayT& part_element_map = fPartitions[n].ElementMap(block_ID[k]);
						for (int j = 0; j < part_element_map.Length(); j++)
							element_map[dex++] = part_element_map[j] + offset;
					
						/* numbering offset in next block */
						offset += block_size[k];
					}
				}
			}
		}
	}

	/* check maps (checks node maps only) */
	CheckAssemblyMaps();
}

/* resident partition for each node */
void JoinOutputT::SetNodePartitionMap(iArrayT& node_partition)
{
	/* initialize */
	node_partition.Allocate(fModel->NumNodes());
	node_partition = -1;
	
	for (int i = 0; i < fPartitions.Length(); i++)
	{
		/* nodes in the partition */
		iArrayT partition_nodes;
		fPartitions[i].PartitionNodes(partition_nodes, PartitionT::kGlobal);
		
		/* write to map */
		for (int j = 0; j < partition_nodes.Length(); j++)
		{
			int node = partition_nodes[j];
			if (node_partition[node] != -1)
			{
				cout << "\n JoinOutputT::SetNodePartitionMap: node already assigned "
				     << node << endl;
				throw eGeneralFail;
			}
			else
				node_partition[node] = i;
		}
	}
	
	/* check map is complete */
	int count = node_partition.Count(-1);
	if (count != 0)
	{
		cout << "\n JoinOutputT::SetNodePartitionMap: " << count
		     << " nodes are unassigned" << endl;
		throw eGeneralFail;
	}
}

/* determine map from local nodes into global array, such that:
*
*             global[lg_map[i]] = local[i]
*/
void JoinOutputT::SetInverseMap(const iArrayT& global, iArrayT& inv_global,
	int& shift, int fill) const
{
	/* compressed number range */
	int max;
	global.MinMax(shift, max);
	int range = max - shift + 1;

	/* determine (all) used nodes */
	inv_global.Allocate(range);
	inv_global = fill;
	for (int i = 0; i < global.Length(); i++)
		inv_global[global[i] - shift] = i;
}

/* return the global node numbers of the set nodes residing
* in the partition */
void JoinOutputT::PartitionSetNodes(int partition, const iArrayT& node_part_map,
	const iArrayT& set_nodes, iArrayT& nodes) const
{
	/* count */
	int count = 0;
	for (int i = 0; i < set_nodes.Length(); i++)
		if (node_part_map[set_nodes[i]] == partition) count++;

	/* allocate return space */
	nodes.Allocate(count);
	
	/* copy in */
	count = 0;
	for (int j = 0; j < set_nodes.Length(); j++)
	{
		int node = set_nodes[j];
		if (node_part_map[node] == partition)
			nodes[count++] = node;
	}
	
	/* need nodes in ascending order of local number */
	fPartitions[partition].SetNodeScope(PartitionT::kLocal, nodes);
	nodes.SortAscending();
	fPartitions[partition].SetNodeScope(PartitionT::kGlobal, nodes);
}

/* add partition contribution to set assembly map */
void JoinOutputT::SetAssemblyMap(const iArrayT& inv_global, int shift, const iArrayT& local,
	iArrayT& lg_map) const
{
	/* set map */
	int n_map = local.Length();
	lg_map.Allocate(n_map);
	int dex = 0;
	int*  p = local.Pointer();
	for (int j = 0; j < n_map; j++)
	{
		int dex = inv_global[*p++ - shift];
		if (dex == -1) throw eGeneralFail;
		lg_map[j] = dex;
	}	
}

/* generate output file name */
void JoinOutputT::ResultFileName(int part, int group, StringT& name) const
{
	/* basic name */
	name.Root(fJobFile);
	name.Append(".p", part);
	name.Append(".io", group);
	
	/* file format extension */
	if (fResultsFileType == IOBaseT::kExodusII)
		name.Append(".exo");
	else
		cout << "\n JoinOutputT::ResultFileName: no extension added for file type "
		     << fResultsFileType << endl;
	
	//NOTE: not handling multiple time sequences ".sq" or changing
	//      geometry groups ".ps"
}

/* returns the number of output steps */
int JoinOutputT::NumOutputSteps(int group) const
{
	/* database  */
	int num_steps = -1;
	ExodusT exo(cout);
	StringT filename;
	
	/* some partitions could be missing */
	for (int i = 0; i < fPartitions.Length() && num_steps < 0; i++)
	{
		/* generate file name */
		ResultFileName(i, group, filename);
		if (exo.OpenRead(filename))
			num_steps = exo.NumTimeSteps();
	}
	return num_steps;
}

/* retrieve output labels */
void JoinOutputT::OutputLabels(int group, ArrayT<StringT>& node_labels,
	ArrayT<StringT>& element_labels) const
{	
	/* some partitions could be missing */
	bool found_file = false;
	for (int i = 0; i < fPartitions.Length() && !found_file; i++)
	{
		/* generate file name */
		StringT filename;
		ResultFileName(i, group, filename);
		
		/* check if found */
		if (fstreamT::Exists(filename))
		{
			/* database  */
			ModelManagerT model(cout);
			if (!model.Initialize(fModel->DatabaseFormat(), filename)) {
				cout << "\n JoinOutputT::OutputLabels: error opening database file \""
				     << fModel->DatabaseName() << '\"' << endl;
				throw eDatabaseFail;
			}
		
			/* read labels */
			found_file = true;
			model.NodeLabels(node_labels);
			model.ElementLabels(element_labels);		
		}
	}
}

/* check that assembly maps are compact and complete */
void JoinOutputT::CheckAssemblyMaps(void)
{
	/* global output sets */
	const ArrayT<OutputSetT*>& element_sets = fIO->ElementSets();

	for (int i = 0; i < element_sets.Length(); i++)
	{
		/* output set data */
		OutputSetT& set = *(element_sets[i]);
			
		/* check node maps */
		if (set.NumNodeValues() > 0)
		{			
			const iArrayT& nodes_used = set.NodesUsed();
						
			/* assembly map */
			const MapSetT& map_set = fMapSets[i];
			
			/* check overall length */
			int node_count = 0;
			for (int j = 0; j < map_set.NumNodeMaps(); j++)
				node_count += map_set.NodeMap(j).Length();
			if (node_count != nodes_used.Length())
			{
				cout << "\n JoinOutputT::CheckAssemblyMaps: node maps size error: " << node_count
				     << " should be " << nodes_used.Length() << " for set " << i << endl;
				throw eGeneralFail;
			}

			/* check fill */
			iArrayT fill_check(nodes_used.Length());
			fill_check = 0;
			
			/* check for overlap */
			for (int k = 0; k < map_set.NumNodeMaps(); k++)
			{
				const iArrayT& node_assem_map = map_set.NodeMap(k);
				for (int j = 0; j < node_assem_map.Length(); j++)
				{
					int& check = fill_check[node_assem_map[j]];
					if (check != 0)
					{
						cout << "\n JoinOutputT::CheckAssemblyMaps: duplicated fill for node "
						     << nodes_used[node_assem_map[j]] << "\n"
						     <<   "     in assembly map " << k << " for output set ID "
						     << set.ID() << endl;
						throw eGeneralFail;
					}
					else
						check = 1;
				}
			}
			
			/* redundant check */
			if (fill_check.Count(0) != 0)
			{
				cout << "\n JoinOutputT::CheckAssemblyMaps: node maps error" << endl;
				throw eGeneralFail;
			}
		}
			
		/* check element maps */
		//TEMP - not supported yet
	}
}
