
#include "PointPlots.h"
#include "TecPlotT.h"
#include "AVST.h"
#include <stdio.h>

PointPlots::PointPlots (ostream& out, istream& in, bool write) :
  TranslateIOManager (out, in, write)
{
}

void PointPlots::Translate (const StringT& program, const StringT& version, const StringT& title)
{
  fModel.Initialize ();
  SetOutput (program, version, title);
  InitializeQuadVariables ();
  cout << "\n One file will be written per time step.\n";
  InitializeTime();
  if (fNumTS < 1)
    {
      fMessage << "\n No time steps found.";
      return;
    }
  StringT name;
  InitializeElements(fElementGroup, name);
  TranslateVariables ();
}

/**************** PRIVATE **********************/

void PointPlots::SetOutput (const StringT& program, const StringT& version, const StringT& title)
{
  if (fWrite)
    {
      cout << "\n    eq.  " << IOBaseT::kTahoe   << ". Text\n";
      cout << "    eq.  " << IOBaseT::kTecPlot << ". TecPlot 7.5\n";
      cout << "    eq.  " << IOBaseT::kAVS << ". AVS UCD ASCII\n";
      cout << "\n Enter the Output Format: ";
    }
  fIn >> fOutputFormat;
  if (fWrite)
    cout << "\n Enter the root of the output files: ";
  fIn >> fOutputName;
  cout << "\n Output Format: " << fOutputFormat << " File: " << fOutputName << endl;
}

void PointPlots::TranslateVariables (void)
{
  StringT temp, ext;
  temp.Append (fTimeIncs[fNumTS-1]);
  int digits = temp.Length()-1;
  switch (fOutputFormat)
    {
    case IOBaseT::kTahoe: ext = "txt"; break;
    case IOBaseT::kTecPlot: ext = "dat"; break;
    case IOBaseT::kAVS: ext = "inp"; break;
    }

  int numused = fQVUsed.Length();
  ArrayT<StringT> labels (numused);
  for (int i=0; i < numused; i++)
    labels[i] = fQuadratureLabels [fQVUsed[i]];
  
  // keep user informed
  int check = 1;
  if (fNumTS > 100) check = 10;
  else if (fNumTS > 1000) check = 100;
  else if (fNumTS > 10000) check = 1000;

  // read data
  int nume = fModel.NumElementGroups ();
  const ArrayT<StringT>& names = fModel.ElementGroupIDs();
  int egindex = fElementGroup;
  int numelems, numelemnodes;
  fModel.ElementGroupDimensions (names [egindex], numelems, numelemnodes);
  int numquadpts = fModel.NumElementQuadPoints (names [egindex]);
  dArray2DT qv (numelems*numquadpts, fNumQV);
  for (int t=0; t < fNumTS; t++)
    {
      if ((t+1)%check == 0 || t == fNumTS-1)
	cout << "Time Step " << fTimeIncs[t]+1 << ": " << fTimeSteps[t] << endl;
      //if (fElementGroup < 0)
      //fModel.AllQuadratureVariables (fTimeIncs[t], qv);
      //else
      fModel.QuadratureVariables (fTimeIncs[t], names[egindex], qv);

      // write files
      ofstreamT outfile;
      OpenFile (outfile, t, digits, ext);
      bool numbered = false;
      switch (fOutputFormat)
	{
	case IOBaseT::kTahoe: // no headers
	  break;
	case IOBaseT::kTecPlot:
	  {
	    iArrayT ijk (2);
	    ijk[0] = qv.MajorDim();
	    ijk[1] = 1;

	    TecPlotT tec (fMessage, true); // point format
	    tec.WriteHeader (outfile, outfile.filename(), labels);
	    tec.WriteIJKZone (outfile, outfile.filename(), ijk);
	    break;
	  }
	case IOBaseT::kAVS:
	  {
	    numbered = true;
	    // use dummy coorinates, not needed to make point plot
	    dArray2DT coords (qv.MajorDim(), 3);
	    double *pc = coords.Pointer();
	    for (int c=0; c < coords.MajorDim(); c++)
	      for (int c2=0; c2 < coords.MinorDim(); c2++)
		*pc++ = c+1;

	    // use dummy connects, not needed to make point plot
	    iArray2DT conn (1,1);
	    conn = 1;

	    AVST avs (fMessage, false);
	    avs.WriteHeader (outfile, qv.MajorDim(), 1, numused, 0, 0);
	    avs.WriteCoordinates (outfile, coords, 1);
	    avs.WriteCells (outfile, GeometryT::kPoint, conn, 1, 1);
	    avs.WriteDataHeader (outfile, labels);
	    break;
	  }
	default:
	  throw eGeneralFail;
	}

      // so far, all output formats use this format
      for (int e=0; e < qv.MajorDim(); e++)
	{
	  if (numbered)
	    outfile << e+1 << " ";
	  for (int g=0; g < numused; g++)
	    outfile << qv (e, fQVUsed[g]) << " ";
	  outfile << "\n";
	}
      outfile.close();
    }
}

void PointPlots::OpenFile (ofstreamT& o, int index, int digits, StringT& ext) const
{
  StringT filename (fOutputName);
  filename.Append ("_", fTimeIncs[index]+1, digits);
  filename.Append (".", ext);
  remove (filename);
  o.open (filename);
  if (!o.is_open())
    {
      fMessage << "\nPointPlots::OpenFile cannot open file: "
	       << filename << "\n\n";
      throw eGeneralFail;
    }
}