// file: CSEBaseT.cpp

// created: SAW 5/2/2000

#include "CSEBaseT.h"

#include "TriT.h"
#include "QuadT.h"
#include "HexahedronT.h"
#include "TetrahedronT.h"
#include "PentahedronT.h"
#include "FEManager.h"

CSEBaseT::CSEBaseT (ostream& fMainOut, int ID) :
  ElementBaseT (fMainOut, ID),
  fSurfaceFacets (2)
{
  fSurfaceFacets[0] = 0;
  fSurfaceFacets[1] = FEManager::kNotSet;
}

void CSEBaseT::Initialize (GeometryT::GeometryCode code, int numregfacenodes)
{
  CSEType (code, numregfacenodes);
  SetFace ();

  out << "\n  Initializing CS Element Group ID . . . . . . . = " 
      << fGroupID << '\n';
  PrintControlData ();
  out << "\n";
}

void CSEBaseT::SetNodes (int e1local, const iArrayT& regelemnodes)
{
  if (!IsElementValid (e1local)) 
    { 
      cout << "CSEBaseT::SetNodes" << endl;
      throw eSizeMismatch;
    }
  
  int *n = fNodeNums (e1local);
  for (int i=0; i < fNumElemNodes; i++)
    *n++ = regelemnodes [fNodeNumberingMap [i]];
}

// *********** PRIVATE *************

void CSEBaseT::CSEType (GeometryT::GeometryCode code, int numFaceNodes)
{
  switch (code)
    {
    case GeometryT::kTriangle:
    case GeometryT::kQuadrilateral:
      {
	fGeometryCode = GeometryT::kQuadrilateral;
	fNumElemNodes = (numFaceNodes == 2) ? 4 : 8;
	fNodeNumberingMap.Allocate (fNumElemNodes);
	fNodeNumberingMap [0] = 1;
	fNodeNumberingMap [1] = 0;
	fNodeNumberingMap [2] = 0;
	fNodeNumberingMap [3] = 1;
	if (fNumElemNodes == 8)
	  {
	    fNodeNumberingMap [4] = 2;
	    fNodeNumberingMap [5] = 0;
	    fNodeNumberingMap [6] = 2;
	    fNodeNumberingMap [7] = 1;
	  }
	fSurfaceFacets[1] = 2;
	break;
      }
    case GeometryT::kHexahedron:
      {
	fGeometryCode = GeometryT::kHexahedron;
	fNumElemNodes = (numFaceNodes == 4) ? 8 : 20;
	fNodeNumberingMap.Allocate (fNumElemNodes);
	Set3DNodes (4);
	fSurfaceFacets[1] = 1;
	break;
      }
    case GeometryT::kTetrahedron:
      {
	fGeometryCode = GeometryT::kPentahedron;
	fNumElemNodes = (numFaceNodes == 3) ? 6 : 15;
	fNodeNumberingMap.Allocate (fNumElemNodes);
	Set3DNodes (3);
	fSurfaceFacets[1] = 1;
	break;
      }
    default:
      {
	cout << " CSEBaseT::CSEType not programmed for element type " 
	     << code << endl;
	throw eGeneralFail;
      }
    }
}

void CSEBaseT::Set3DNodes (int linearfacenodes)
{
  for (int fe=0; fe < linearfacenodes; fe++)
    {
      // surface 0
      fNodeNumberingMap [fe] = fe;
      // surface 1
      fNodeNumberingMap [fe + linearfacenodes] = fe;
    }

  if (fNodeNumberingMap.Length() > 2*linearfacenodes)
    for (int f=0; f < linearfacenodes; f++)
      {
	// midface nodes on surface 0
	fNodeNumberingMap[2*linearfacenodes + f] = linearfacenodes + f;
	
	// midface nodes on surface 1
	fNodeNumberingMap[3*linearfacenodes + f] = linearfacenodes + f;

	// midface nodes on surfaces unconnected to regular elements
	fNodeNumberingMap[4*linearfacenodes + f] = f;
      }
}
