/* $Id: PatranT.cpp,v 1.12.4.1 2002-06-27 18:00:59 cjkimme Exp $ */
/* created sawimme (05/17/2001) */

#include "PatranT.h"
#include "ifstreamT.h"
#include "iArrayT.h"
#include "iArray2DT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "ExceptionCodes.h"
#include "iAutoArrayT.h"
#include <time.h>


using namespace Tahoe;

PatranT::PatranT (ostream &message_out) :
  fMessage (message_out)
{
}

PatranT::~PatranT (void) 
{
}

bool PatranT::OpenRead (const StringT& filename)
{
  ifstreamT tmp (filename);
  if (!tmp.is_open()) 
    {
      fMessage << "\n PatranT::OpenRead, unable to open file: "
	   << filename << endl;
      return false;
    }

  file_name = filename;
  ScanFile ();
  return true;
}

void PatranT::VersionNotes (ArrayT<StringT>& records) const
{
  int ID, IV, KC;
  ifstream in (file_name);
  if (!AdvanceTo (in, kSummary, ID, IV, KC)) return;
  ClearPackets (in, 1);
  records.Allocate (4);
  records[0] = "PATRAN";
  in >> records[2] >> records[3] >> records[1];
}

int PatranT::NumNodes (void) const
{
  int ID, IV, KC, num_nodes;
  ifstream in (file_name);
  if (!AdvanceTo (in, kSummary, ID, IV, KC)) 
    {
      fMessage << "\n PatranT::NumNodes, no nodes found" << endl;
      return -1;
    }
  in >> num_nodes;
  return num_nodes;
}

int PatranT::NumElements (void) const
{
  int ID, IV, KC, num_nodes, num_elements;
  ifstream in (file_name);
  if (!AdvanceTo (in, kSummary, ID, IV, KC))  
    {
      fMessage << "PatranT::NumElements, no elements found\n";
      return -1;
    }
  in >> num_nodes >> num_elements;
  return num_elements;
}

int PatranT::NumNamedComponents (void) const
{
  return fNamedComponents.Length();
}

int PatranT::NumDimensions (void) const
{
  /*int ID, IV, KC;
    ifstream in (file_name);
    if (!AdvanceTo (in, kElement, ID, IV, KC))   
    {
    fMessage << "PatranT::NumDimensions, no elements found\n";
    return -1;
    }
    switch (IV)
    {
    case kBarShape:
    case kTriShape:
    case kQuadShape:
    return 2;
    break;
    case kTetShape:
    case kWedgeShape:
    case kHexShape:
    return 3;
    break;
    }
    fMessage << "\n PatranT::NumDimensions: Unknown element shape ID=" << ID
    << " shape= " << IV << "\n";
    return -1;*/
  return 3;
}

bool PatranT::NamedComponents (ArrayT<StringT>& names) const
{
  if (names.Length() != fNamedComponents.Length()) throw eSizeMismatch;
  for (int i=0; i < names.Length(); i++)
    names[i] = fNamedComponents[i];
  return true;
}

bool PatranT::ReadGlobalNodeMap (iArrayT& map) const
{
  int numnodes = NumNodes ();
  int ID, IV, KC, num = 0;
  map.Allocate (numnodes);
  ifstream in (file_name);
  while (AdvanceTo (in, kNode, ID, IV, KC))
    {
      if (num >= map.Length())
	{
	  fMessage << "PatranT::ReadGlobalNodeMap, incorrect allocation\n";
	  return false;
	}
      map [num++] = ID;
      ClearPackets (in, KC+1);
    } 
  if (num != numnodes)  
    {
      fMessage << "PatranT::ReadGlobalNodeMap num != numnodes\n"
	   << num << " " << numnodes << "\n";
      return false;
    }
  return true;
}

bool PatranT::ReadGlobalElementMap (iArrayT& map) const
{
  int numelems = NumElements ();
  int ID, IV, KC, num = 0;
  ifstream in (file_name);
  map.Allocate (numelems);
  while (AdvanceTo (in, kElement, ID, IV, KC))
    {
      if (num >= map.Length())
	{
	  fMessage << "PatranT::ReadGlobalElementMap, incorrect allocation\n";
	  return false;
	}
      map [num++] = ID;
      ClearPackets (in, KC+1);
    } 
  if (num != numelems) 
    {
      fMessage << "PatranT::ReadGlobalElementMap num != numelems\n"
	   << num << " " << numelems << "\n";
      return false;
    }
  return true;
}

bool PatranT::NumNodesInSet (const StringT& title, int& num) const
{
  num = -1;
  int index = LocateNamedComponent (title);
  if (index < 0) 
    {
      fMessage << "PatranT::NumNodesInSet, unable to read named components\n";
      return false;
    }

  /* pull node IDs from component list */
  num = 0;
  int length = fNamedComponentsData[index].MajorDim();
  int *it = fNamedComponentsData[index].Pointer();
  int *il = fNamedComponentsData[index].Pointer() + 1;
  for (int i=0; i < length; i++, it += 2, il += 2)
    if (*it == kNodeType)
      num++;

  return true;
}

bool PatranT::ReadCoordinates (dArray2DT& coords, int dof) const
{
  if (dof != coords.MinorDim())
    {
      fMessage << "PatranT::ReadCoordinates, incorrect minor allocation\n";
      return false;
    }
  int ID, IV, KC;
  ifstream in (file_name);
  int count = 0;
  dArrayT temp (dof);
  while (count < coords.MajorDim())
    {
      if (!AdvanceTo (in, kNode, ID, IV, KC)) 
	{
	  fMessage << "PatranT::ReadCoordinates, unable to find node, count = "
	       << count << "\n";
	  return false;
	}
      ClearPackets (in, 1);
      in >> temp;
      coords.SetRow (count, temp);
      ClearPackets (in, KC);
      count ++;
    }
  return true;
}

bool PatranT::ReadElementBlockDims (const StringT& title, int& num_elems, int& num_elem_nodes) const
{
  int ID, IV, KC, code;
  iArrayT elems;
  if (!ReadElementSet (title, code, elems))
    {
      fMessage << "PatranT::ReadElementBlockDims, unable to read set\n";
      return false;
    }
  num_elems = elems.Length();
  if (num_elems == 0) 
    {
      num_elem_nodes = 0;
      return true;
    }
  ifstream in (file_name);
  while (AdvanceTo (in, kElement, ID, IV, KC))
    {
      if (elems.HasValue (ID))
	{
	  ClearPackets (in, 1);
	  in >> num_elem_nodes;
	  return true;
	}
      else
	ClearPackets (in, KC + 1);
    }
  fMessage << "\n PatranT::ReadElementBlockDims, unable to find element for set "
       << title << "\n";
  return false;
}

bool PatranT::ReadConnectivity (const StringT& title, int& namedtype, iArray2DT& connects) const
{
  iArrayT elems;
  if (!ReadElementSet (title, namedtype, elems))
    {
      fMessage << "PatranT::ReadConnectivity, unable to read set";
      return false;
    }
  
  int count = 0, num_nodes;
  int *pc = connects.Pointer();
  int ID, IV, KC;
  ifstream in (file_name);
  iArrayT temp (connects.MinorDim());
  while (count < connects.MajorDim())
    {
      if (!AdvanceTo (in, kElement, ID, IV, KC)) 
	{
	  fMessage << "PatranT::ReadConnectivity, unable to find element\n";
	  fMessage << "num elems " << elems.Length() << " " << connects.MajorDim() << endl;
	  fMessage << "count " << count << endl;
	  elems.WriteWithFormat (fMessage, 6, 0, 6);
	  throw eDatabaseFail;
	  return false;
	}

      if (elems.HasValue (ID))
	{
	  ClearPackets (in, 1);

	  in >> num_nodes;
	  if (num_nodes != connects.MinorDim()) 
	    {
	      fMessage << "\n PatranT::ReadConnectivity: Warning num_nodes_per_element mismatch for "
		   << title << " ID=" << ID << " has " << num_nodes 
		   << ", which doesn't match " << connects.MinorDim() << "\n";
	    }
	  ClearPackets (in, 1);

	  in >> temp;
	  connects.SetRow (count, temp);
	  KC -= (num_nodes + 9)/10;
	  ClearPackets (in, KC);

	  count ++;
	}
      else
	ClearPackets (in, KC + 1);
    }
  return true;
}

bool PatranT::ReadAllElements (ArrayT<iArrayT>& connects, iArrayT& elementtypes) const
{
  int ID, IV, KC, num_nodes, count = 0;
  ifstream in (file_name);
  while (AdvanceTo (in, kElement, ID, IV, KC)) 
    {
      if (IV < kLine || IV > kHexahedron)
	{
	  fMessage << "\nPatranT::ReadAllElements, IV not valid\n";
	  fMessage << "ID " << ID << " IV " << IV << " KC " << KC << endl;
	  throw eDatabaseFail;
	}

      if (count >= connects.Length() ||
	  count >= elementtypes.Length())
	{
	  fMessage << "\nPatranT::ReadAllElements, incorrect allocation\n";
	  throw eSizeMismatch;
	}

      ClearPackets (in, 1);

      in >> num_nodes;
      connects[count].Allocate (num_nodes);
      ClearPackets (in, 1);

      in >> connects[count];
      KC -= (num_nodes + 9)/10;
      ClearPackets (in, KC);

      elementtypes[count] = IV;
      count ++;
    }

  if (count != elementtypes.Length())
    {
      fMessage << "\nPatranT::ReadAllElements, incorrect number read or allocated\n";
      fMessage << "count " << count << " " << " elementtypes.length " << elementtypes.Length() << endl;
      throw eSizeMismatch;    
    }

  return true;
}

bool PatranT::ReadElementSet (const StringT& title, int& namedtype, iArrayT& elems) const
{
  int index = LocateNamedComponent (title);
  if (index < 0) 
    {
      fMessage << "PatranT::ReadElementSet, unable to read named component\n";
      return false;
    }

  /* pull element IDs from component list */
  iAutoArrayT set;
  namedtype = -1;
  int length = fNamedComponentsData[index].MajorDim();
  int *it = fNamedComponentsData[index].Pointer();
  int *il = fNamedComponentsData[index].Pointer() + 1;
  for (int i=0; i < length; i++, it += 2, il += 2)
    if ((*it >   5 && *it < 19) ||
	(*it > 105 && *it < 119) ||
	(*it > 205 && *it < 219))
      {
	set.Append (*il);
	if (namedtype == -1)
	  namedtype = *it;
      }

  elems.Allocate (set.Length());
  elems.CopyPart (0, set, 0, set.Length());
  return true;
}

bool PatranT::ReadElementSetMixed (const StringT& title, iArrayT& namedtype, iArrayT& elems) const
{
  int index = LocateNamedComponent (title);
  if (index < 0) 
    {
      fMessage << "PatranT::ReadElementSet, unable to read named component\n";
      return false;
    }

  /* pull element IDs from component list */
  iAutoArrayT set;
  iAutoArrayT nt;
  int length = fNamedComponentsData[index].MajorDim();
  int *it = fNamedComponentsData[index].Pointer();
  int *il = fNamedComponentsData[index].Pointer() + 1;
  for (int i=0; i < length; i++, it += 2, il += 2)
    if ((*it >   5 && *it < 19) ||
	(*it > 105 && *it < 119) ||
	(*it > 205 && *it < 219))
      {
	set.Append (*il);
	nt.Append (*it);
      }

  elems.Allocate (set.Length());
  elems.CopyPart (0, set, 0, set.Length());
  namedtype.Allocate (nt.Length());
  namedtype.CopyPart (0, nt, 0, nt.Length());
  return true;
}

bool PatranT::ReadDistLoadSetDims (int setID, int& num_elems) const
{
  int ID, IV, KC;
  ifstream in (file_name);
  num_elems = 0;
  while (AdvanceTo (in, kDistLoads, ID, IV, KC))
    {
      if (setID == IV) num_elems++;
      ClearPackets (in, KC + 1);
    }
  return true;
}

bool PatranT::ReadDistLoadSet (int setID, iArray2DT& facets) const
{
  int ID, IV, KC;
  ifstream in (file_name);
  int num_elems = 0;
  while (num_elems < facets.MajorDim())
    {
      if (!AdvanceTo (in, kDistLoads, ID, IV, KC)) 
	{
	  fMessage << "PatranT::ReadDistLoadSet, unable to find dist load\n";
	  return false;
	}

      if (setID == IV) 
	{
	  int itemp;
	  StringT temp;
	  ClearPackets (in, 1);
	  in >> temp >> itemp;
	  facets (num_elems, 0) = ID;
	  facets (num_elems, 1) = itemp;
	  num_elems++;
	  ClearPackets (in, KC);
	}
      else
	ClearPackets (in, KC + 1);
    }
  return true;
}

bool PatranT::ReadNodeSet (const StringT& title, iArrayT& nodes) const
{
  int index = LocateNamedComponent (title);
  if (index < 0) 
    {
      fMessage << "PatranT::ReadNodeSet, unable to read named components\n";
      return false;
    }

  /* pull node IDs from component list */
  iAutoArrayT set;
  int length = fNamedComponentsData[index].MajorDim();
  int *it = fNamedComponentsData[index].Pointer();
  int *il = fNamedComponentsData[index].Pointer() + 1;
  for (int i=0; i < length; i++, it += 2, il += 2)
    if (*it == kNodeType)
      set.Append (*il);
    
  nodes.Allocate (set.Length());
  nodes.CopyPart (0, set, 0, set.Length());
  return true;
}

bool PatranT::ReadNodeSets (const ArrayT<StringT>& title, iArrayT& nodes) const
{
  iArrayT count (title.Length());
  for (int i=0; i < title.Length(); i++)
    if (!NumNodesInSet (title[i], count[i])) 
      {
	fMessage << "PatranT::ReadNodeSets, unable to read num nodes in set\n";
	return false;
      }

  nodes.Allocate (count.Sum());
  iArrayT nt;
  for (int j=0, k=0; j < title.Length(); j++)
    {
      nt.Allocate (count[j]);
      if (!ReadNodeSet (title[j], nt)) 
	{
	  fMessage << "PatranT::ReadNodeSets, unable to read node set "
	       << title[j] << "\n";
	  return false;
	}
      nodes.CopyPart (k, nt, 0, count[j]);
      k += count[j];
    }
    return true;
}

bool PatranT::WriteHeader (ostream& out, int numnodes, int numelems, StringT& title) const
{
  iArrayT n (5);
  n = 0;
  if (!WritePacketHeader (out, kTitle, 0, 0, 1, n))
    return false;

  for (int i=0; i < 80 && i < title.Length()-1; i++)
    out << title[i];
  out << "\n";

  n[0] = numnodes;
  n[1] = numelems;
  /*
    n[2] = nummats;
    n[3] = numelemprops;
    n[4] = numcoordframes;
  */
  if (!WritePacketHeader (out, kSummary, 0, 0, 1, n))
    return false;
  time_t now;
  time (&now);
  char date[10], time[10];
  strftime (date, 10, "%d-%b-%y", localtime (&now));
  strftime (time, 10, "%H:%M:%S", localtime (&now));
  for (int d=0; d < 9; d++)
    out << date[d];
  for (int d2=9; d2 < 12; d2++)
    out << " ";
  for (int t=0; t < 8; t++)
    out << time[t];
  out << "   3.0\n";
  
  return true;
}

bool PatranT::WriteCoordinates (ostream& out, dArray2DT& coords, int firstnodeID) const
{
  iArrayT n (5);
  n = 0;
  int p = out.precision ();
  out.precision (prec);
  out.setf (ios::scientific);
  for (int k=0; k < coords.MajorDim(); k++)
    {
      if (!WritePacketHeader(out, kNode, firstnodeID + k, 0, 2, n)) 
	return false;
      for (int i=0; i < coords.MinorDim(); i++)
	out << setw (cwidth) << coords (k, i);
      for (int j=coords.MinorDim(); j < 3; j++)
	out << setw (cwidth) << 0.0;
      out << "\n";

      int ICF = 1;
      char GTYPE = 'G';
      int dof = 6;
      int config = 0;
      int CID = 0;
      iArrayT PSPC (6);
      PSPC = 0;
      out << setw (1) << ICF;
      out << setw (1) << GTYPE;
      out << setw (hwidth) << dof;
      out << setw (hwidth) << config;
      out << setw (hwidth) << CID;
      out << setw (2) << "  ";
      for (int ii=0; ii < 6; ii++)
	out << setw(1) << PSPC[ii];
      out << "\n";
    }
  out.precision (p);
  return true;
}

bool PatranT::WriteElements (ostream& out, iArray2DT& elems, iArrayT& elemtypes, int firstelemID) const
{
  iArrayT n (5);
  n = 0;
  int KC = 1 + (elems.MinorDim() + 9) / 10;
  int p = out.precision ();
  out.precision (prec);
  for (int e=0; e < elems.MajorDim(); e++)
    {
      if (!WritePacketHeader (out, kElement, firstelemID + e, elemtypes[e], KC, n))
	return false;
      
      int config = 0;
      int PID = 0;
      int CEID = 0;
      dArrayT theta (3);
      theta = 0;
      out << setw (hwidth) << elems.MinorDim();
      out << setw (hwidth) << config;
      out << setw (hwidth) << PID;
      out << setw (hwidth) << CEID;
      for (int t=0; t < theta.Length(); t++)
	out << setw (16) << theta[t];
      out << "\n";

      for (int i=0; i < elems.MinorDim(); i++)
	{
	  out << setw (hwidth) << elems (e, i);
	  if ( (i+1) % 10 || i == elems.MinorDim() - 1) 
	    out << "\n";
	}

      // if (n[0] > 0) write associated data
    }
  out.precision (p);
  return true;
}

bool PatranT::WriteNamedComponent (ostream& out, StringT& name, int ID, iArray2DT& comps) const
{
  iArrayT n (5);
  n = 0;
  if (comps.MinorDim() != 2) return false;
  int IV = comps.Length();
  int KC = 1 + (IV + 9) / 10;
  if (!WritePacketHeader (out, kNamedComponents, ID, IV, KC, n))
    return false;
  for (int g=0; g < 12 && g < name.Length()-1; g++)
    out << name[g];
  out << "\n";
  int *pc = comps.Pointer();
  int w = 1;
  for (int i=0, w=0; i < IV; i++, w++)
    {
      out << setw (hwidth) << *pc++;
      if (w == 10) 
	{
	  out << "\n";
	  w = 0;
	}
    }

  // filler
  for (int j=w; j < 11; j++)
    out << setw (hwidth) << 0;
  out << "\n";

  return true;
}

bool PatranT::WriteGeometryPoints (ostream& out, dArray2DT& points, int firstptID) const
{
  iArrayT n (5);
  n = 0;
  int p = out.precision ();
  out.precision (prec);
  out.setf (ios::scientific);
  for (int k=0; k < points.MajorDim(); k++)
    {
      if (!WritePacketHeader (out, kGridData, firstptID + k, 0, 1, n))
	return false;
      for (int i=0; i < points.MinorDim(); i++)
	out << setw (cwidth) << points (k, i);
      for (int j=points.MinorDim(); j < 3; j++)
	out << setw (cwidth) << 0.0;
      out << "\n";
    }
  out.precision (p);
  return true;
}

bool PatranT::WriteClosure (ostream& out) const
{
  int tag = 99;
  int ID = 0, IV = 0, KC = 1;
  iArrayT n (5);
  n = 0;
  if (!WritePacketHeader (out, tag, ID, IV, KC, n)) return false;
  return true;
}

/**************************************************************************
* Private
**************************************************************************/

void PatranT::ScanFile (void)
{
  int ID, IV, KC, num=0;
  ifstream in (file_name);
  StringT name;
  // count number of named components 
  while (AdvanceTo (in, kNamedComponents, ID, IV, KC))
    {
      ClearPackets (in, KC + 1);
      num++;
    }

  in.close ();
  fNamedComponents.Allocate (num);
  fNamedComponentsData.Allocate (num);

  in.open (file_name);
  // save data
  int count = 0;
  while (AdvanceTo (in, kNamedComponents, ID, IV, KC))
    {
      ClearPackets (in, 1);
      in >> fNamedComponents[count];
      fNamedComponentsData[count].Allocate (IV/2, 2);
      in >> fNamedComponentsData[count];
      count ++;
      ClearPackets (in, 1);
    }
}

int PatranT::LocateNamedComponent (const StringT &title) const
{
  for (int i=0; i < fNamedComponents.Length(); i++)
    if (strncmp (title.Pointer(), fNamedComponents[i].Pointer(), title.StringLength()) == 0)
      return i;
  return -1;
}

bool PatranT::AdvanceTo (ifstream &in, int target, int& ID, int &IV, int &KC) const
{
  int IT = 0;
  while (in >> IT)
    {
      in >> ID >> IV >> KC;
      
      if (IT == target) return true;

      /* skip to next entry */
      ClearPackets (in, KC + 1);
    }

  //cout << target << " " << IT << " " << ID << endl;
  //fMessage << "PatranT::AdvanceTo: Cannot find: " << target << '\n';
  return false;
}

void PatranT::ClearPackets (ifstream &in, int KC) const
{
  char line[255];
  for (int i=0; i < KC; i++)
    in.getline (line, 254);
}

bool PatranT::WritePacketHeader (ostream& out, int tag, int ID, int IV, int KC, iArrayT n) const
{
  if (n.Length () != 5)
    {
      fMessage << "\n PatranT::WritePacketHeader, wrong length for N\n"
	       << tag << " " << ID << " " << n.Length() << endl;
      return false;
    }
  out << setw (2) << tag;
  out << setw (hwidth) << ID;
  out << setw (hwidth) << IV;
  out << setw (hwidth) << KC;
  for (int i=0; i < n.Length(); i++)
    out << setw (hwidth) << n[i];
  out << "\n";
  return true;
}
