/*
  File: BCJHypoIsoDamageKE2D.h
*/

#ifndef _BCJ_HYPO_ISO_DAMAGE_KE_2D_H_
#define _BCJ_HYPO_ISO_DAMAGE_KE_2D_H_

#include "BCJHypoIsoDamageKE3D.h"
#include "Material2DT.h"

#include <iostream.h>
#include "dMatrixT.h"
#include "dSymMatrixT.h"

class ifstreamT;
class ElasticT;

class BCJHypoIsoDamageKE2D : public BCJHypoIsoDamageKE3D, public Material2DT
{
 public:
  // constructor
  BCJHypoIsoDamageKE2D(ifstreamT& in, const FiniteStrainT& element);

  // destructor
  ~BCJHypoIsoDamageKE2D();

  // Cauchy stress
  virtual const dSymMatrixT& s_ij();   

  // tangent modulus
  virtual const dMatrixT& c_ijkl();

  // print data and model name
  virtual void Print(ostream& out) const;
  virtual void PrintName(ostream& out) const;

 protected:

  // Cauchy stress in 2D
  dSymMatrixT f2Ds_ij;

  // tangent moduli in 2D
  dMatrixT f2Dc_ijkl; 
};

#endif /* _BCJ_HYPO_ISO_DAMAGE_KE_2D_ */
