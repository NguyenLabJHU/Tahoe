/* $Id: main.cpp,v 1.13 2002-08-16 17:55:54 sawimme Exp $ */

#include "TranslateIOManager.h"
#include "ExtractNode.h"
#include "ExtractQuad.h"
#include "PointPlots.h"
#include "MergeResults.h"
#include "ifstreamT.h"
#include "StringT.h"
#include "AutoArrayT.h"

using namespace Tahoe;

void ReadArgs (char* a, AutoArrayT<StringT>& list);
istream& Open (ifstreamT& tmp, const StringT& f);

int main (int c, char* a [])
{
  try 
    {
      bool write = true;
      AutoArrayT<StringT> filelist;
      ifstreamT tmp;

      /* is a command line arguement provided */
      if (c >= 2)
	ReadArgs (a[1], filelist);
      else
	filelist.Free();

      int numruns = 1;
      if (filelist.Length() > 0) 
	{
	  write = false;
	  numruns = filelist.Length();
	}
      for (int f=0; f < numruns; f++)
	{
	  if (write == true)
	    filelist.Append ("");
	  istream& in = Open (tmp, filelist[f]);

	  int selection;
	  if (write)
	    {
	      cout << "\n1. Datafile Translation \n";
	      cout << "2. Nodal Data Extraction to XY Data \n";
	      cout << "3. Quadrature Data Extraction to XY Data \n";
	      cout << "4. Quadrature Data Extraction for Point Plots \n";
	      cout << "5. Merge Results Files \n";
	      cout << "\n Select type of translation: ";
	    }
	  in >> selection;
	  cout << "\n Type of translation: " << selection << ".";
	  
	  TranslateIOManager *dataio;
	  StringT program, version;
	  switch (selection)
	    {
	    case 1:
	      {
		cout << " Translate data files.\n\n";
		program = "Translate";
		version = "v1.4";
		dataio = new TranslateIOManager (cout, in, write);
		break;
	      }
	    case 2:
	      {
		cout << " Extract nodal data.\n\n";
		program = "Extract";
		version = "v1.1";
		dataio = new ExtractNode (cout, in, write);
		break;
	      }
	    case 3:
	      {
		cout << " Extract quadrature data.\n\n";
		program = "Extract";
		version = "v1.1";
		dataio = new ExtractQuad (cout, in, write);
		break;
	      }
	    case 4:
	      {
		cout << " Extract quadrature data for point plots.\n\n";
		program = "PointPlot";
		version = "v1.0";
		dataio = new PointPlots (cout, in, write);
		break;
	      }
	    case 5:
	      {
		cout << " Merge data from multiple files.\n\n";
		program = "Merge";
		version = "v1.0";
		dataio = new MergeResults (cout, in, write);
		break;
	      }
	    default:
	      throw eGeneralFail;
	    }
	  dataio->Translate (program, version, program);
	}
      cout << "\n\n Progam Complete.\n\n";
    }
  catch (int ErrorCode)
    {
      cout << "\n\n Exiting due to error . . . ";
      switch (ErrorCode)
	{
	case eBadInputValue:
	  cout << " Bad Input Value\n";
	  break;
	case eOutOfRange:
	  cout << " Out of Range\n";
	  break;
	case eSizeMismatch:
	  cout << " Size Mismatch\n";
	  break;
	case eOutOfMemory:
	  cout << " Out of Memory\n";
	  break;
	case eDatabaseFail:
	  cout << " Error with database\n";
	  break;
	}
      cout << "\n\n Game Over\n\n";
    }
  return 1;
}

void ReadArgs (char* a, AutoArrayT<StringT>& list)
{
  ifstreamT tmp (a);
  StringT s;
  s.GetLineFromStream (tmp);
  switch (s[0])
    {
    case '%':
      {
	list.Append (a);
	break;
      }
    case '@':
      {
	while (tmp.good())
	  {
	    s.Clear();
	    s.GetLineFromStream(tmp);
	    if (s.StringLength() > 1)
	      list.Append (s);
	  }
	break;
      }
    default:
      {
	cout << "\n You must put either % or @ at the beginning of your input file.\n\n";
	throw eBadInputValue;
      }
    }
}

istream& Open (ifstreamT& tmp, const StringT& f)
{
  if (f.StringLength() >= 1)
    {
      tmp.open (f);
      cout << "\n Reading answers from: " << f << endl;

      char c;
      tmp >> c;
      if (c != '%')
	{
	  cout << "\nExpecting % at the start of the input file.\n\n";
	  throw eBadInputValue;
	}
      return tmp;
    }

  return cin;
}
