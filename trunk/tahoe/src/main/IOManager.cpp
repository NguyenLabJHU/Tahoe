/* $Id: IOManager.cpp,v 1.15 2002-06-25 14:14:12 sawimme Exp $ */
/* created: sawimme (10/12/1999) */

#include "IOManager.h"

#include "fstreamT.h"
#include "ifstreamT.h"
#include "OutputSetT.h"
#include "dArrayT.h"

// output
#include "FE_ASCIIT.h"
#include "ExodusOutputT.h"
#include "EnSightOutputT.h"
#include "AbaqusOutputT.h"
#include "TecPlotOutputT.h"

IOManager::IOManager(ostream& outfile, const StringT& program_name,
	const StringT& version, const StringT& title, const StringT& input_file,
	IOBaseT::FileTypeT output_format):
	fLog(outfile),
	fOutputFormat(output_format),
	fOutput(NULL),
	fEcho (false),
	fOutputTime(0.0),
	fOutput_tmp(NULL)
{
	/* construct output formatter */
	fOutput = NewOutput(program_name, version, title, input_file, fOutputFormat, fLog);
}

IOManager::IOManager(ifstreamT& in, const IOManager& io_man):
	fLog(io_man.fLog),
	fOutputFormat(io_man.fOutputFormat),
	fOutput(NULL),
	fEcho (false),
	fOutputTime(0.0),
	fOutput_tmp(NULL)
{
	/* construct output formatter */
	fOutput = NewOutput((io_man.fOutput)->CodeName(),
				(io_man.fOutput)->Version(),
				(io_man.fOutput)->Title(), in.filename(), fOutputFormat, fLog);
}

#if 0
IOManager::IOManager (ostream& out) :
	fLog (out),
	fOutput(NULL),
	fOutputFormat (IOBaseT::kExodusII),
	fEcho (false)
{

}
#endif

IOManager::~IOManager(void)
{
	/* in case output is diverted */
	RestoreOutput();
	
	delete fOutput;
	fOutput = NULL;
}

void IOManager::EchoData (ostream& o) const
{
  IOBaseT temp (o);
  o << " Output format . . . . . . . . . . . . . . . . . = " << fOutputFormat  << '\n';
  temp.OutputFormats (o);
}

/*********** OUTPUT **************/


void IOManager::NextTimeSequence(int sequence_number)
{
	fOutput->NextTimeSequence(sequence_number);
}

/* set model coordinates */
void IOManager::SetCoordinates(const dArray2DT& coordinates, const iArrayT* node_id)
{
	fOutput->SetCoordinates(coordinates, node_id); 
}
	
/* register the output for an element set. returns the output ID */
int IOManager::AddElementSet(const OutputSetT& output_set)
{
	return fOutput->AddElementSet(output_set);
}

const ArrayT<OutputSetT*>& IOManager::ElementSets(void) const
{
	return fOutput->ElementSets();
}

void IOManager::AddNodeSet(const iArrayT& nodeset, const StringT& setID)
{
	fOutput->AddNodeSet(nodeset, setID);
}

void IOManager::AddSideSet(const iArray2DT& sideset, const StringT& setID, const StringT& group_ID)
{
	fOutput->AddSideSet(sideset, setID, group_ID);
}

/* output functions */
void IOManager::WriteGeometry(void)
{
	fOutput->WriteGeometry();
}

void IOManager::WriteGeometryFile(const StringT& file_name,
	IOBaseT::FileTypeT format) const
{
	if (!fOutput)
	{
		cout << "\n IOManager::WriteGeometryFile: output must be configured" << endl;
		throw eGeneralFail;		
	}

	fOutput->WriteGeometryFile(file_name, format);
}

void IOManager::WriteOutput(int ID, const dArray2DT& n_values, const dArray2DT& e_values)
{
	fOutput->WriteOutput(fOutputTime, ID, n_values, e_values);
}

/* return the list of nodes used by the output set */
const iArrayT& IOManager::NodesUsed(int ID) const { return fOutput->NodesUsed(ID); }

/* (temporarily) re-route output */
void IOManager::DivertOutput(const StringT& outfile)
{
	/* can only divert once */
	if (fOutput_tmp != NULL)
		cout << "\n IOManager::DivertOutput: cannot divert output to \""
		     << outfile << "\".\n"
		     <<   "     Output is already diverted to \"" <<
		     fOutput->OutputRoot() << "\"" << endl;
	else
	{
		/* store main out */
		fOutput_tmp = fOutput;
	
		/* construct temporary output formatter */
		StringT tmp(outfile);
		tmp.Append(".ext"); //OutputBaseT takes root of name passed in
		fOutput = NewOutput(fOutput_tmp->CodeName(), fOutput_tmp->Version(),
			fOutput_tmp->Title(), tmp, fOutputFormat, fLog);
		
		/* add all output sets */
		const ArrayT<OutputSetT*>& element_sets = fOutput_tmp->ElementSets();
		for (int i = 0; i < element_sets.Length(); i++)
			AddElementSet(*(element_sets[i]));
			
		/* set coordinate data */
		fOutput->SetCoordinates(fOutput_tmp->Coordinates(), fOutput_tmp->NodeID());
	}
}

void IOManager::RestoreOutput(void)
{
	if (fOutput_tmp != NULL)
	{
		/* delete temp output formatter */
		delete fOutput;
	
		/* restore main out */
		fOutput = fOutput_tmp;
		fOutput_tmp = NULL;
	}
}

const OutputSetT& IOManager::OutputSet(int ID) const
{
	return fOutput->OutputSet(ID);
}

/* construct and return new output formatter */
OutputBaseT* IOManager::NewOutput(const StringT& program_name,
	const StringT& version, const StringT& title, const StringT& input_file,
	IOBaseT::FileTypeT output_format, ostream& log)
{
	ArrayT<StringT> outstrings (4);
	outstrings[0] = input_file;
	outstrings[1] = title;
	outstrings[2] = program_name;
	outstrings[3] = version;

	const int kdigits = 4;
	OutputBaseT* output = NULL;
	switch (output_format)
	  {
	  case IOBaseT::kExodusII:
	    output = new ExodusOutputT(log, outstrings);
	    break;
	  case IOBaseT::kTahoe:
	  case IOBaseT::kTahoeII:
	  case IOBaseT::kTahoeResults:
	    output = new FE_ASCIIT(log, true, outstrings);
	    break;
	  case IOBaseT::kEnSight:
	    output = new EnSightOutputT(log, outstrings, kdigits, false);
	    break;
	  case IOBaseT::kEnSightBinary:
	    output = new EnSightOutputT(log, outstrings, kdigits, true);
	    break;
	  case IOBaseT::kAbaqus:
	    output = new AbaqusOutputT(log, outstrings, false);
	    break;
	  case IOBaseT::kAbaqusBinary:
	    output = new AbaqusOutputT(log, outstrings, true);
	    break;
	  case IOBaseT::kTecPlot:
	    output = new TecPlotOutputT(log, outstrings, kdigits);
	    break;
	  default:
	    {			
	      cout << "\n IOManager::SetOutput unknown output format:"
		   << output_format << endl;
	      log  << "\n IOManager::SetOutput unknown output format:"
		    << output_format << endl;
	      throw eBadInputValue;
	    }
	  }	
	if (!output) throw eOutOfMemory;
	return output;
}

