/* $Id: BCJHypo2D.h,v 1.4 2002-07-05 22:28:26 paklein Exp $ */

#ifndef _BCJ_HYPO_2D_H_
#define _BCJ_HYPO_2D_H_

#include "BCJHypo3D.h"
#include "Material2DT.h"

#include <iostream.h>
#include "dMatrixT.h"
#include "dSymMatrixT.h"


namespace Tahoe {

class ifstreamT;
class ElasticT;

class BCJHypo2D : public BCJHypo3D, public Material2DT
{
 public:
  // constructor
  BCJHypo2D(ifstreamT& in, const FiniteStrainT& element);

  // destructor
  ~BCJHypo2D();

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

} // namespace Tahoe 
#endif /* _BCJ_HYPO_2D_ */