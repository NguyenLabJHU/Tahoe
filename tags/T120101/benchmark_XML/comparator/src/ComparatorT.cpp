/* $Id: ComparatorT.cpp,v 1.9 2001-10-11 16:32:25 paklein Exp $ */

#include "ComparatorT.h"

#include <iostream.h>
#include <iomanip.h>
#include <time.h>
#include <strstream.h>

#include "fstreamT.h"
#include "Constants.h"
#include "ExceptionCodes.h"
#include "StringT.h"
#include "dArray2DT.h"

const char kBenchmarkDirectory[] = "benchmark";

/* default tolerances */
double abs_tol = 1.0e-10;
double rel_tol = 1.0e-08;

/* constructor */
ComparatorT::ComparatorT(int argc, char* argv[], char job_char, char batch_char):
	FileCrawlerT(argc, argv, job_char, batch_char),
	fAbsTol(abs_tol),
	fRelTol(rel_tol),
	fIsRoot(true)
{
	/* tolerances reset on command line */
	int index;
	if (CommandLineOption("-abs_tol", index))
	{
		istrstream in((const char *) fCommandLineOptions[index + 1]);
		in >> abs_tol;
	}
	if (CommandLineOption("-rel_tol", index))
	{
		istrstream in((const char *) fCommandLineOptions[index + 1]);
		in >> rel_tol;
	}
}

/* prompt input files until "quit" */
void ComparatorT::Run(void)
{
	/* check for command line specification of the file names */
	int index;
	if (CommandLineOption("-f1", index))
	{
		/* first file */
		StringT file_1(fCommandLineOptions[index + 1]);
		if (CommandLineOption("-f2", index))
		{
			/* second file */
			StringT file_2(fCommandLineOptions[index + 1]);

			/* (re-)set default tolerances */
			fAbsTol = abs_tol;	
			fRelTol = rel_tol;

			/* compare */
			PassOrFail(file_1, file_2, false, true);
		}
		else
		{
			cout << "\n ComparatorT::Run: expecting command line option \"-f2\"" << endl;
			throw eGeneralFail;
		}
	}
	else /* inherited */
		FileCrawlerT::Run();
}

/**********************************************************************
* Protected
**********************************************************************/

void ComparatorT::RunJob(ifstreamT& in, ostream& status)
{
#pragma unused(status)

	/* append to list of files */
	fFiles.Append(in.filename());

	/* append to results */
	cout << "\nSTART: " << in.filename() << '\n';
	fPassFail.Append(PassOrFail(in));
	cout << "\nEND: " << in.filename() << '\n';
}

/* batch file processing */
void ComparatorT::RunBatch(ifstreamT& in, ostream& status)
{
	/* keep path to the root batch file */
	bool is_root = false;
	if (fIsRoot) 
	{
		fIsRoot = false;
		is_root = true;
	}

	/* inherited */
	FileCrawlerT::RunBatch(in, status);
	
	/* write summary */
	if (is_root)
	{
		/* check */
		if (fFiles.Length() != fPassFail.Length()) throw eGeneralFail;

		/* relative path */
		StringT path;
		path.FilePath(in.filename());

		/* open output stream */
		StringT file_name = "compare.summary";
		file_name.ToNativePathName();
		file_name.Prepend(path);
		ofstreamT summary;
		summary.open(file_name);
		if (!summary.is_open())
		{
			cout << "\n ComparatorT::RunBatch: ERROR: could not open file: " 
			     << file_name << endl;
			throw eGeneralFail;
		}

		/* count pass/fail */
		int pass_count = 0;
		for (int j = 0; j < fPassFail.Length(); j++)
			if (fPassFail[j]) pass_count++;

		/* write summary results */
		const char* result[]= {"FAIL", "PASS"};
		summary << "number of tests: " << fFiles.Length() << '\n';
		summary << "         passed: " << pass_count << '\n';
		summary << "         failed: " << fFiles.Length() - pass_count << '\n';
		for (int i = 0; i < fFiles.Length(); i++)
			summary << result[fPassFail[i]] << ": " << fFiles[i] << endl;
			
		/* empty lists */
		fPassFail.Resize(0);
		fFiles.Resize(0);
		
		/* reset root */
		fIsRoot = true;		
	}
}

/**********************************************************************
* Private
**********************************************************************/

/* compare results against benchmarks */
bool ComparatorT::PassOrFail(ifstreamT& in) //const
{
	/* path to the current directory */
	StringT path;
	path.FilePath(in.filename());

	/* reset default tolerances */
	fAbsTol = abs_tol;	
	fRelTol = rel_tol;

	/* look for local tolerance file */
	StringT tol_file = "compare.tol";
	tol_file.ToNativePathName();
	tol_file.Prepend(path);
	ifstreamT tol_in(in.comment_marker(), tol_file);

	/* clear skip list */
	fSkipLabels.Resize(0);

	/* set comparison method */
	bool do_relative = true;
	bool do_absolute = false;
	if (tol_in.is_open())
	{
		cout << "found local tolerance file: " << tol_file << '\n';
		StringT key;
		tol_in >> key;
		while (tol_in.good())
		{
			
			if (key == "abs_tol")
			{
				tol_in >> fAbsTol;
			
				/* for now, disable relative tolerance if found */	
				do_relative = false;
				do_absolute = true;
			}
			else if (key == "rel_tol")
			{
				tol_in >> fRelTol;

				/* for now, disable relative tolerance if found */	
				do_relative = true;
				do_absolute = false;
			}
			else if (key == "skip")
			{
				StringT skip_label;
				tol_in >> skip_label;
				fSkipLabels.Append(skip_label);
			}
			tol_in >> key;
		}
	}
	else
		cout << "did not find local tolerance file: " << tol_file << '\n';
	cout << "using tolerances:\n";
	if (do_absolute) cout << "    abs_tol = " << fAbsTol << '\n';
	if (do_relative) cout << "    rel_tol = " << fRelTol << '\n';
	cout.flush();
	
	/* job file name */
	StringT file_root = in.filename();
	file_root.Drop(path.StringLength());
	file_root.Root();
	
	/* look for results file in current directory */
	StringT current = path;
	current.Append(file_root, ".run");

	/* look for results file in benchmark directory */
	StringT benchmark(kBenchmarkDirectory);
	benchmark.ToNativePathName();
	benchmark.Prepend(path);
	if (file_root[0] != StringT::DirectorySeparator())
	{
		const char sep[2] = {StringT::DirectorySeparator(), '\0'};
		benchmark.Append(sep, file_root, ".run");
	}
	else
		benchmark.Append(file_root, ".run");

	/* do compare */
	 return PassOrFail(current, benchmark, do_relative, do_absolute);
}

/* compare results */
bool ComparatorT::PassOrFail(const StringT& file_1, const StringT& file_2,
	bool do_rel, bool do_abs)
{
	/* open file_1 -> "current" */
	ifstreamT current_in(file_1);
	cout << "looking for current results: " << file_1 << ": "
	     << ((current_in.is_open()) ? "found" : "not found") << '\n';
	if (!current_in.is_open()) return false;

	/* open file_2 -> "benchmark" */
	ifstreamT bench_in(file_2);
	cout << "looking for benchmark results: " << file_2 << ": "
	     << ((bench_in.is_open()) ? "found" : "not found") << '\n';
	if (!bench_in.is_open()) return false;

	/* init */
	StringT b_str, c_str;
	if (!bench_in.FindString("O U T P U T", b_str)) return false;
	if (!current_in.FindString("O U T P U T", c_str)) return false;
	
	/* block info */
	int b_group, c_group;
	double b_time, c_time;
	bool b_OK = ReadDataBlockInfo(bench_in, b_group, b_time);
	bool c_OK = ReadDataBlockInfo(current_in, c_group, c_time);

	/* compare blocks */
	while (b_OK && c_OK)
	{
		/* verify block info */
		if (b_group != c_group) {
			cout << "group number mismatch: " << b_group << " != " << c_group << '\n';
			return false;
		}
		else
			cout << "group: " << b_group << '\n';		
		if (fabs(b_time - c_time) > kSmall) {
			cout << "time mismatch: " << b_time << " != " << c_time << '\n';
			return false;
		}
		else
			cout << "time: " << b_time << '\n';
	
		/* read nodal data */
		ArrayT<StringT> b_node_labels;
		dArray2DT b_node_data;
		if (!ReadNodalData(bench_in, b_node_labels, b_node_data)) {
			cout << "error reading node data from: " << bench_in.filename() << '\n';
			return false;
		}

		ArrayT<StringT> c_node_labels;
		dArray2DT c_node_data;
		if (!ReadNodalData(current_in, c_node_labels, c_node_data)) {
			cout << "error reading node data from: " << current_in.filename() << '\n';
			return false;
		}

		/* compare nodal blocks */
		if (!CompareDataBlocks(b_node_labels, b_node_data, c_node_labels, c_node_data, 
			do_rel, do_abs)) {
			cout << "nodal data check: FAIL" << '\n';
			return false;
		}
		else cout << "nodal data check: PASS" << '\n';

		/* read element data */
		ArrayT<StringT> b_element_labels;
		dArray2DT b_element_data;
		if (!ReadElementData(bench_in, b_element_labels, b_element_data)) {
			cout << "error reading element data from: " << bench_in.filename() << '\n';
			return false;
		}

		ArrayT<StringT> c_element_labels;
		dArray2DT c_element_data;
		if (!ReadElementData(current_in, c_element_labels, c_element_data)) {
			cout << "error reading element data from: " << current_in.filename() << '\n';
			return false;
		}

		/* compare element blocks */
		if (!CompareDataBlocks(b_element_labels, b_element_data, c_element_labels, c_element_data, 
			do_rel, do_abs)) {
			cout << "element data check: FAIL" << '\n';
			return false;
		}
		else cout << "element data check: PASS" << '\n';

		/* next block */
		b_OK = ReadDataBlockInfo(bench_in, b_group, b_time);
		c_OK = ReadDataBlockInfo(current_in, c_group, c_time);
	}

	/* termination */
	if (b_OK == c_OK) 
		return true;
	else 
		return false;
}

/* read data block header */
bool ComparatorT::ReadDataBlockInfo(ifstreamT& in, int& group, double& time) const
{
	StringT str;

	/* group number */
	if (!in.FindString("Group number", str)) return false;
	if (!str.Tail('=', group)) return false;
	
	/* time */
	if (!in.FindString("Time", str)) return false;
	if (!str.Tail('=', time)) return false;
	
	return true;
}

/* read block of nodal data */
bool ComparatorT::ReadNodalData(ifstreamT& in, ArrayT<StringT>& labels, dArray2DT& data) const
{
	/* advance nodal data */
	StringT str;
	if (!in.FindString("Nodal data:", str)) return false;

	/* get dimensions */
	int num_nodes, num_values;
	if (!in.FindString("Number of nodal points", str)) return false;
	str.Tail('=', num_nodes);
	if (!in.FindString("Number of values", str)) return false;
	str.Tail('=', num_values);
	
	if (num_values > 0)
	{
		/* read labels */
		labels.Allocate(num_values + 1); // +1 for node number
		for (int i = 0; i < labels.Length(); i++)
			in >> labels[i];
		if (!in.good()) return false;
	
		/* read values */
		data.Allocate(num_nodes, num_values + 1); // +1 for node number
		in >> data;
	}
	return true;
}

/* read block of element data */
bool ComparatorT::ReadElementData(ifstreamT& in, ArrayT<StringT>& labels, dArray2DT& data) const
{
	/* advance nodal data */
	StringT str;
	if (!in.FindString("Element data:", str)) return false;

	/* get dimensions */
	int num_nodes, num_values;
	if (!in.FindString("Number of elements", str)) return false;
	str.Tail('=', num_nodes);
	if (!in.FindString("Number of values", str)) return false;
	str.Tail('=', num_values);
	
	if (num_values > 0)
	{
		/* read labels */
		labels.Allocate(num_values + 1); // +1 for node number
		for (int i = 0; i < labels.Length(); i++)
			in >> labels[i];
		if (!in.good()) return false;
	
		/* read values */
		data.Allocate(num_nodes, num_values + 1); // +1 for node number
		in >> data;
	}
	return true;
}

/* compare blocks - normalized by data set 1 */
bool ComparatorT::CompareDataBlocks(const ArrayT<StringT>& labels_1, const dArray2DT& data_1,
	const ArrayT<StringT>& labels_2, const dArray2DT& data_2, bool do_rel, bool do_abs) const
{
	/* compare labels */
	if (labels_1.Length() != labels_2.Length()) {
		cout << "label length mismatch: " << labels_1.Length() << " != " << labels_2.Length() << '\n';
		return false;
	}
	for (int i = 0; i < labels_1.Length(); i++)
		if (labels_1[i] != labels_2[i]) {
			cout << "mismatch with label " << i+1 << ": " << labels_1[i] << " != " << labels_2[i] << '\n';
			return false;
		}

	/* compare data */
	if (data_1.MajorDim() != data_2.MajorDim() ||
	    data_1.MinorDim() != data_2.MinorDim()) {
	    cout << "data dimension mismatch" << '\n';
	    return false;
	}
	
	/* echo dimensions */
	cout << "  number of rows: " <<  data_1.MajorDim() << '\n';
	cout << "number of values: " <<  data_1.MinorDim() << '\n';

	double max_abs_error = 0.0, max_rel_error = 0.0;
	for (int j = 0; j < data_1.MinorDim(); j++)
	{
		cout << "comparing: \"" << labels_1[j] << "\"\n";

		if (fSkipLabels.HasValue(labels_1[j]))
			cout << "comparing: \"" << labels_1[j] << "\": SKIPPED\n";
		else
		{
			double error_norm = 0.0, bench_norm = 0.0;
			int max_rel_error_index = -1, max_abs_error_index = -1;
			double max_rel_error_var = 0.0, max_abs_error_var = 0.0;
			for (int i = 0; i < data_1.MajorDim(); i++)
			{
				/* error */
				double abs_error = data_2(i,j) - data_1(i,j);
				double rel_error = (fabs(data_1(i,j)) > kSmall) ? abs_error/data_1(i,j) : 0.0;

				/* norms */
				bench_norm += data_1(i,j)*data_1(i,j);
				error_norm += abs_error*abs_error;
			
				/* track maximums */
				if (fabs(abs_error) > fabs(max_abs_error_var)) {
					max_abs_error_var = abs_error;
					max_abs_error_index = i;
				}
				if (fabs(rel_error) > fabs(max_rel_error_var)) {
					max_rel_error_var = rel_error;
					max_rel_error_index = i;
				}		
			}
		
			/* compute norms */
			bench_norm = sqrt(bench_norm);
			error_norm = sqrt(error_norm);
		
			cout << "abs error norm: " << error_norm << '\n';
			cout << "rel error norm: ";
			if (bench_norm > kSmall)
				cout << error_norm/bench_norm << '\n';
			else
				cout << "0.0" << '\n';
			
			/* maximum errors */
			cout << "         max abs error: " << max_abs_error_var << '\n';
			cout << "index of max abs error: " << max_abs_error_index + 1 << '\n';
			cout << "         max rel error: " << max_rel_error_var << '\n';
			cout << "index of max rel error: " << max_rel_error_index + 1 << '\n';
		
			/* running max */
			max_abs_error = (fabs(max_abs_error_var) > fabs(max_abs_error)) ? max_abs_error_var : max_abs_error;
			max_rel_error = (fabs(max_rel_error_var) > fabs(max_rel_error)) ? max_rel_error_var : max_rel_error;
		}
	}
	
	/* results strings */
	const char* judge[] = {"PASS", "FAIL", "IGNORE", "OVERRIDE"};
	
	/* absolute tolerance test */
	const char* abs_judge = judge[0];
	if (do_abs)
	{
		if (fabs(max_abs_error) > fAbsTol)
		{
			if (fabs(max_rel_error) > fRelTol)
				abs_judge = judge[1];
			else
				abs_judge = judge[3];
		}
	}
	else
		abs_judge = judge[2];

	/* relative tolerance test */
	const char* rel_judge = judge[0];
	if (do_rel)
	{
		if (fabs(max_rel_error) > fRelTol)
		{
			if (fabs(max_abs_error) > fAbsTol)
				rel_judge = judge[1];
			else
				rel_judge = judge[3];
		}
	}
	else
		rel_judge = judge[2];
	
	/* report */
	cout << "absolute tolerance result: " << fAbsTol << " : " << max_abs_error << " : " << abs_judge << '\n';
	cout << "relative tolerance result: " << fRelTol << " : " << max_rel_error << " : " << rel_judge << '\n';

	if (abs_judge == judge[1] || rel_judge == judge[1])
		return false;
	else
		return true;
}