/* $Id: D2VIB2D.cpp,v 1.4 2002-10-20 22:48:58 paklein Exp $ */
/* created: paklein (10/23/1999)                                          */

#include "D2VIB2D.h"

#include <math.h>
#include <iostream.h>

#include "toolboxConstants.h"
#include "fstreamT.h"
#include "D2MeshFreeFDElasticT.h"

/* constructors */

using namespace Tahoe;

D2VIB2D::D2VIB2D(ifstreamT& in, const D2MeshFreeFDElasticT& element):
	VIB2D(in, element),
	fD2MLSShape(element.D2MLSShapeFunction())
{
	/* length scale parameter */
	in >> feps2;
//	if (feps2 < 0.0) throw ExceptionT::kBadInputValue;
		
	/* squared */
	feps2 *= feps2;
}

/* print parameters */
void D2VIB2D::Print(ostream& out) const
{
	/* inherited */
	VIB2D::Print(out);
	
	out << " Length scale parameter (epsilon). . . . . . . . = " << sqrt(feps2) << '\n';
}

#if 0
/* DISABLE */
const dMatrixT& D2VIB2D::c_ijkl(void)
{
	cout << "\n D2VIB2D::c_ijkl: not allowed" << endl;
	throw ExceptionT::kGeneralFail;
	return fModuli; //dummy
}

const dSymMatrixT& D2VIB2D::s_ij(void)
{
	cout << "\n D2VIB2D::s_ij: not allowed" << endl;
	throw ExceptionT::kGeneralFail;
	return fPK2; //dummy
}

const dMatrixT& D2VIB2D::C_IJKL(void)
{
	cout << "\n D2VIB2D::C_IJKL: not allowed" << endl;
	throw ExceptionT::kGeneralFail;
	return fModuli; //dummy
}

const dSymMatrixT& D2VIB2D::S_IJ(void)
{
	cout << "\n D2VIB2D::S_IJ: not allowed" << endl;
	throw ExceptionT::kGeneralFail;
	return fPK2; //dummy
}
#endif

/***********************************************************************
* Protected
***********************************************************************/

/* print name */
void D2VIB2D::PrintName(ostream& out) const
{
	/* inherited */
	VIB2D::PrintName(out);
	out << "    with higher order gradient terms\n";
}