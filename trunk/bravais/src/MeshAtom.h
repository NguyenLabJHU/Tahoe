#ifndef _MESHATOM_H_
#define _MESHATOM_H_

#include <iostream>
#include <fstream.h>

/* direct members */
#include "StringT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "iAutoArrayT.h"
#include "IOBaseT.h"
#include "GeometryT.h"

#include "ifstreamT.h"
#include "iArray2DT.h"
#include "dArray2DT.h"

#include "CrystalLatticeT.h"
#include "FCCT.h"
#include "BCCT.h"
#include "VolumeT.h"
#include "BoxT.h"
#include "OutputSetT.h"
#include "OutPutLatticeT.h"


namespace Tahoe {


/** Class to construct a mesh of atoms defined by
 *  its crytallography and its shape. 
 *  Create an output file in different formats. 
 *
 *  Remark: To use this class, firt create a mesh 
 *  (with CreateMeshAtom), gives you number of atoms.
 *  Call one the defined functions to get mesh data such
 *  as coordinates,connectivities, volume etc.
 **/

class MeshAtom {

 protected:

  CrystalLatticeT* Crystal;
  VolumeT* Shape;
  OutputSetT* Set;
  OutPutLatticeT* IOLattice;

 public:

  // Constructor
  MeshAtom(StringT which_latticetype,int nsd,int nuca,
	   dArrayT latticeparameter,StringT which_shape,
	   int whichunit,dArray2DT len, iArrayT cel,int irot,
	   dArray2DT mat_rot,double angle,iArrayT isort);
  
  // Destructor: not done yet. 
  ~MeshAtom();

  // Create the mesh of atoms and return number of atoms.
  int CreateMeshAtom();

  // Access to mesh
  double Volume_of_Mesh();
  dArray2DT Length();

  iArrayT* ReturnAtomID();
  dArray2DT* ReturnCoordinates();
  iArray2DT* ReturnConnectivities();
  dArray2DT* ReturnBounds();


  //Build IO file
  void BuildIOFile(StringT& program_name,
		   StringT& version, StringT& title, 
		   StringT& input_file,
		   IOBaseT::FileTypeT output_format,
		   iArrayT per);
  

				

};

} // namespace Tahoe 


#endif

    
