/* $Id: Tensor4DT.h,v 1.5.2.1 2002-10-17 01:51:27 paklein Exp $ */
/* created paklein (12/19/96) */
#ifndef _TENSOR4D_T_H_
#define _TENSOR4D_T_H_

/* base class */
#include "TensorT.h"

namespace Tahoe {

/** templated base class for fourth order tensors */
template <class MATHTYPE>
class Tensor4DT: public TensorT<MATHTYPE>
{
  public:

	/** \name constructors */
	/*@{*/
	Tensor4DT(void);
	Tensor4DT(int dim0, int dim1, int dim2, int dim3);
	Tensor4DT(const Tensor4DT& source);
	/*@}*/

	/** dimensioning */
	void Dimension(int dim0, int dim1, int dim2, int dim3);

	/** \deprecated replaced by Tensor4DT::Dimension on 02/13/2002 */
	void Allocate(int dim0, int dim1, int dim2, int dim3) { Dimension(dim0, dim1, dim2, dim3); };

	/** \name element and subdimension accessors */
	/*@{*/
	MATHTYPE& operator()(int dim0, int dim1, int dim2, int dim3) const;
	MATHTYPE* operator()(int dim0, int dim1, int dim2) const;
	MATHTYPE* operator()(int dim0, int dim1) const;
	MATHTYPE* operator()(int dim0) const;
	/*@}*/

  	/** \name assignment operators */
	/*@{*/
  	Tensor4DT<MATHTYPE>& operator=(const Tensor4DT& RHS);
  	Tensor4DT<MATHTYPE>& operator=(const MATHTYPE& value);
	/*@}*/
		
  protected:

	/** \name offsets */
	/*@{*/
	int fOffset0;
	int fOffset1;
	int fOffset2;
	/*@}*/
};

/*************************************************************************
 * Implementation
 *************************************************************************/

template <class MATHTYPE> 
inline Tensor4DT<MATHTYPE>::Tensor4DT(void): fOffset0(0), fOffset1(0), fOffset2(0) { }

template <class MATHTYPE> 
inline Tensor4DT<MATHTYPE>::Tensor4DT(int dim0, int dim1, int dim2, int dim3): 
	TensorT<MATHTYPE>(dim0*dim1*dim2*dim3, 4)
{
	Dimension(dim0, dim1, dim2, dim3);
}

template <class MATHTYPE> 
inline Tensor4DT<MATHTYPE>::Tensor4DT(const Tensor4DT& source): 
	TensorT<MATHTYPE>(source)
{

}

template <class MATHTYPE>
void Tensor4DT<MATHTYPE>::Dimension(int dim0, int dim1, int dim2, int dim3)
{
	/* base class allocate */
	TensorT<MATHTYPE>::Dimension(dim0*dim1*dim2*dim3, 4);

	/* dimensions */
	fDim[0] = dim0;
	fDim[1] = dim1;
	fDim[2] = dim2;
	fDim[3] = dim3;

	/* sanity check */
	if (fDim.Min() < 1) throw ExceptionT::kGeneralFail;

	/* offsets */
	fOffset0 = fDim[1]*fDim[2]*fDim[3];
	fOffset1 = fDim[2]*fDim[3];
	fOffset2 = fDim[3];
}

template <class MATHTYPE>
inline MATHTYPE& Tensor4DT<MATHTYPE>::
	operator()(int dim0, int dim1, int dim2, int dim3) const
{
/* range checking */
#if __option (extended_errorcheck)
	if (dim0 < 0 || dim0 >= fDim[0] ||
        dim1 < 0 || dim1 >= fDim[1] ||
        dim2 < 0 || dim2 >= fDim[2] ||
        dim3 < 0 || dim3 >= fDim[3]) throw ExceptionT::kGeneralFail;
#endif

	return (fArray[dim0*fOffset0 + dim1*fOffset1 + dim2*fOffset2 + dim3]);
}

template <class MATHTYPE>
inline MATHTYPE* Tensor4DT<MATHTYPE>::
	operator()(int dim0, int dim1, int dim2) const
{
/* range checking */
#if __option (extended_errorcheck)
	if (dim0 < 0 || dim0 >= fDim[0] ||
        dim1 < 0 || dim1 >= fDim[1] ||
        dim2 < 0 || dim2 >= fDim[2]) throw ExceptionT::kGeneralFail;
#endif

	return fArray + dim0*fOffset0 + dim1*fOffset1 + dim2*fOffset2;
}

template<class MATHTYPE>
inline MATHTYPE* Tensor4DT<MATHTYPE>::operator()(int dim0, int dim1) const
{
/* range checking */
#if __option (extended_errorcheck)
	if (dim0 < 0 || dim0 >= fDim[0] ||
        dim1 < 0 || dim1 >= fDim[1]) throw ExceptionT::kGeneralFail;
#endif

	return fArray + dim0*fOffset0 + dim1*fOffset1;
}

template <class MATHTYPE>
inline MATHTYPE* Tensor4DT<MATHTYPE>::operator()(int dim0) const
{
/* range checking */
#if __option (extended_errorcheck)
	if (dim0 < 0 || dim0 >= fDim[0]) throw ExceptionT::kGeneralFail;
#endif

	return fArray + dim0*fOffset0;
}

template <class MATHTYPE> 
inline Tensor4DT<MATHTYPE>& Tensor4DT<MATHTYPE>::operator=(const Tensor4DT& RHS)
{
	/* inherited */
	TensorT<MATHTYPE>::operator=(RHS);
	return *this;
}

template <class MATHTYPE> 
inline Tensor4DT<MATHTYPE>& Tensor4DT<MATHTYPE>::operator=(const MATHTYPE& value)
{
	/* inherited */
	TensorT<MATHTYPE>::operator=(value);
	return *this;
}

} // namespace Tahoe 
#endif /* _TENSOR4D_T_H_ */
