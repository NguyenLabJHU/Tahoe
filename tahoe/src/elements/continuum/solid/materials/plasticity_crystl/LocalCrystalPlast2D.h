/* $Id: LocalCrystalPlast2D.h,v 1.5.54.1 2004-07-06 06:54:00 paklein Exp $ */
#ifndef _LOCAL_CRYSTAL_PLAST_2D_H_
#define _LOCAL_CRYSTAL_PLAST_2D_H_

#include "LocalCrystalPlast.h"

#include <iostream.h>
#include "dMatrixT.h"
#include "dSymMatrixT.h"

namespace Tahoe {

class ifstreamT;
class SolidElementT;

class LocalCrystalPlast2D : public LocalCrystalPlast
{
 public:
  // constructor
  LocalCrystalPlast2D(ifstreamT& in, const FSMatSupportT& support);

  // Cauchy stress - Taylor average    
  virtual const dSymMatrixT& s_ij();   

  // modulus - Taylor average 
  virtual const dMatrixT& c_ijkl();

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;
	/*@}*/

 protected:
 
  // crystal Cauchy stress in 2D
  dSymMatrixT f2Dsavg_ij;
  
  // crystal tangent moduli in 2D
  dMatrixT f2Dcavg_ijkl;
};

} // namespace Tahoe 
#endif /* _LOCAL_CRYSTAL_PLAST_2D_H_ */
