/* $Id: zTensor3DT.h,v 1.2.4.1 2002-06-27 18:00:47 cjkimme Exp $ */
/* created : PAK (05/19/97) */

#ifndef _Z_TENSOR3D_T_H_
#define _Z_TENSOR3D_T_H_

/* base class */
#include "Tensor3DT.h"

/* direct members */
#include "ComplexT.h"

/* forward declarations */

namespace Tahoe {

class dTensor3DT;

class zTensor3DT: public Tensor3DT<ComplexT>
{
  public:

	/* constructor */
	zTensor3DT(void);
	zTensor3DT(int dim0, int dim1, int dim2);
	zTensor3DT(const dTensor3DT& re, const dTensor3DT& im);
	zTensor3DT(const zTensor3DT& source);

 	/* assigment operators */
	zTensor3DT& operator=(const zTensor3DT& RHS);
	zTensor3DT& operator=(const ComplexT& value);
			
	/*
  	 * Returning the Real and Imaginary parts
  	 */
  	void toRe(dTensor3DT& re) const;
  	void toIm(dTensor3DT& im) const;
  	zTensor3DT& toZ(const dTensor3DT& re, const dTensor3DT& im);	
};

/*
 * Assigment operators
 */
inline zTensor3DT& zTensor3DT::operator=(const zTensor3DT& RHS)
{
	Tensor3DT<ComplexT>::operator=(RHS);
	return(*this);
}

inline zTensor3DT& zTensor3DT::operator=(const ComplexT& value)
{
	Tensor3DT<ComplexT>::operator=(value);
	return(*this);
}

} // namespace Tahoe 
#endif /* _Z_TENSOR3D_T_H_ */