/*
  File: LocalCrystalPlastFp_C.h
*/

#ifndef _LOCAL_CRYSTAL_PLAST_FP_C_H_
#define _LOCAL_CRYSTAL_PLAST_FP_C_H_

#include "LocalCrystalPlastFp.h"

#include <iostream.h>
#include "dArrayT.h"
#include "dMatrixT.h"
#include "dSymMatrixT.h"
#include "dArray2DT.h"
#include "LocalArrayT.h"

class ifstreamT;
class ElasticT;
class ElementCardT;
class StringT;

class LocalCrystalPlastFp_C : public LocalCrystalPlastFp
{
 public:
  // constructor
  LocalCrystalPlastFp_C(ifstreamT& in, const FiniteStrainT& element);

  // destructor
  ~LocalCrystalPlastFp_C();

  // Cauchy stress - Taylor average    
  virtual const dSymMatrixT& s_ij();   

  // modulus - Taylor average 
  virtual const dMatrixT& c_ijkl();

  // update/reset crystal state
  virtual void UpdateHistory();
  virtual void ResetHistory();

  // output related methods
  virtual int NumOutputVariables() const;
  virtual void OutputLabels(ArrayT<StringT>& labels) const;
  virtual void ComputeOutput(dArrayT& output);

  // print data and model name
  virtual void Print(ostream& out) const;
  virtual void PrintName(ostream& out) const;

 protected:
 
  // deformation gradient at center of element
  void DeformationGradient_C(const LocalArrayT& disp, dMatrixT& F_3D);

 private:

  // number of crystal variables to be stored
  virtual int NumVariablesPerElement();
 
   // initial value of crystal variables
  virtual void InitializeCrystalVariables(ElementCardT& element);

 protected:

  // number of nodes/element: 4-node Quad (2D) and 8-node Hexa (3D)
  //const int fNNodes;
  int fNNodes;

  // references to initial coords
  const LocalArrayT& fLocInitX;

  // arrays for shape funtion derivatives at center
  dArray2DT fLNa;
  dArray2DT fLDNa;
  dArray2DT fGDNa;

  // displacement gradient
  dMatrixT fGradU;
};

#endif /* _LOCAL_CRYSTAL_PLAST_FP_C_H_ */
