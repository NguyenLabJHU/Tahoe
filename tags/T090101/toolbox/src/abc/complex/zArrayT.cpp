/* $Id: zArrayT.cpp,v 1.3 2001-02-01 18:22:29 paklein Exp $ */
/* created: PAK/AFLP (05/19/1997)                                         */

#include "zArrayT.h"

#include <iostream.h>
#include <iomanip.h>
#include "Constants.h"
#include "dArrayT.h"

/*
* constructor
*/
zArrayT::zArrayT(void) { }
zArrayT::zArrayT(int length): nArrayT<ComplexT>(length) { }
zArrayT::zArrayT(int length, ComplexT* p): nArrayT<ComplexT>(length,p) { }
zArrayT::zArrayT(const dArrayT& re, const dArrayT& im)
{
	toZ(re,im);
}
zArrayT::zArrayT(const zArrayT& source): nArrayT<ComplexT>(source) { }

/*
* I/O operators
*/
istream& operator>>(istream& in, zArrayT& array)
{
	for (int i = 0; i < array.Length(); i++)
		in >> array[i];

	return (in);
}

ostream& operator<<(ostream& out, const zArrayT& array)
{
	for (int i = 0; i < array.Length(); i++)
		out << array[i];

	return (out);
}

/*
* Returning the Real and Imaginary parts
*/
void zArrayT::toRe(dArrayT& re) const
{
	/* ComplexT function */
	z_to_Re(*this, re);
}

void zArrayT::toIm(dArrayT& im) const
{
	/* ComplexT function */
	z_to_Im(*this, im);
}

zArrayT& zArrayT::toZ(const dArrayT& re, const dArrayT& im)
{
	/* dimension */
	Allocate(re.Length());
	
	/* ComplexT function */
	ReIm_to_z(re,im,*this);

	return (*this);
}

/* conjugate every element in the array */
zArrayT& zArrayT::Conjugate(const zArrayT& array)
{
  /* must have same length */
  if (array.Length() != Length()) throw eSizeMismatch;

  ComplexT* pLHS = Pointer();
  ComplexT* pRHS = array.Pointer();
  int length = Length();
  for(int i = 0; i < length; i++)
	(*pLHS++).Conjugate(*pRHS++);
	
  return *this;
}