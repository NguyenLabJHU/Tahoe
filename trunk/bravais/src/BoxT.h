/* $Id: BoxT.h,v 1.3 2002-07-25 23:48:07 saubry Exp $ */

#ifndef _BOX_T_H_
#define _BOX_T_H_

#include <iostream>
#include "iArrayT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "VolumeT.h"
#include "CrystalLatticeT.h"
#include "ifstreamT.h"

using namespace Tahoe;

class BoxT : public VolumeT 
{
  
 protected:
  
  iArrayT ncells;
  dArrayT length;
  
 public:
  
  //Constructor
  BoxT(int dim, int whichunit, dArrayT len_cel, 
       dArrayT lattice_parameter);
  
  //Destructor
  ~BoxT(){};
  
	// Copy constructor
  BoxT(const BoxT& source);
  
  void CalculateVolume();
  void CreateLattice(CrystalLatticeT* pcl);
  
  iArrayT GetNCells();
  dArrayT GetLength();
  
};

inline iArrayT BoxT::GetNCells(){return ncells;};
inline dArrayT BoxT::GetLength(){return length;};

#endif
