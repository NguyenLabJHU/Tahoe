/* $Id: LocalCrystalPlast2D.h,v 1.2 2001-07-03 01:35:35 paklein Exp $ */
/*
  File: LocalCrystalPlast2D.h
*/

#ifndef _LOCAL_CRYSTAL_PLAST_2D_H_
#define _LOCAL_CRYSTAL_PLAST_2D_H_

#include "LocalCrystalPlast.h"
#include "Material2DT.h"

#include <iostream.h>
#include "dMatrixT.h"
#include "dSymMatrixT.h"

class ifstreamT;
class ElasticT;

class LocalCrystalPlast2D : public LocalCrystalPlast, public Material2DT
{
 public:
  // constructor
  LocalCrystalPlast2D(ifstreamT& in, const FiniteStrainT& element);

  // destructor
  ~LocalCrystalPlast2D();

  // Cauchy stress - Taylor average    
  virtual const dSymMatrixT& s_ij();   

  // modulus - Taylor average 
  virtual const dMatrixT& c_ijkl();

  // print data and model name
  virtual void Print(ostream& out) const;
  virtual void PrintName(ostream& out) const;

 protected:
 
  // crystal Cauchy stress in 2D
  dSymMatrixT f2Dsavg_ij;
  
  // crystal tangent moduli in 2D
  dMatrixT f2Dcavg_ijkl;
};

#endif /* _LOCAL_CRYSTAL_PLAST_2D_H_ */
