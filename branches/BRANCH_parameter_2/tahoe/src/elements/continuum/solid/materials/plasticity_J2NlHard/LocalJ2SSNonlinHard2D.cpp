/* $Id: LocalJ2SSNonlinHard2D.cpp,v 1.3.32.3 2004-03-04 06:45:34 paklein Exp $ */
#include "LocalJ2SSNonlinHard2D.h"
#include "ElementCardT.h"
#include "StringT.h"

using namespace Tahoe;

/* constructor */
LocalJ2SSNonlinHard2D::LocalJ2SSNonlinHard2D(ifstreamT& in, const SSMatSupportT& support) :
	ParameterInterfaceT("small_strain_J2_local_2D"),
  LocalJ2SSNonlinHard(in, support),  
  fStress2D(2),
  fModulus2D(dSymMatrixT::NumValues(2)),
  fTotalStrain3D(3)
{

}

/* returns elastic strain (3D) */
const dSymMatrixT& LocalJ2SSNonlinHard2D::ElasticStrain(const dSymMatrixT& totalstrain,
	const ElementCardT& element, int ip)
{
	/* 2D -> 3D (plane strain) */
	fTotalStrain3D.ExpandFrom2D(totalstrain);

	/* inherited */
	return LocalJ2SSNonlinHard::ElasticStrain(fTotalStrain3D, element, ip);
}

/* moduli */
const dMatrixT& LocalJ2SSNonlinHard2D::c_ijkl()
{
	/* 3D -> 2D */
	fModulus2D.Rank4ReduceFrom3D(LocalJ2SSNonlinHard::c_ijkl());
	return fModulus2D;
}

/* stress */
const dSymMatrixT& LocalJ2SSNonlinHard2D::s_ij()
{
	/* 3D -> 2D */
	fStress2D.ReduceFrom3D(LocalJ2SSNonlinHard::s_ij());
	return fStress2D;
}

/* describe the parameters needed by the interface */
void LocalJ2SSNonlinHard2D::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	LocalJ2SSNonlinHard::DefineParameters(list);
	
	/* 2D option must be plain stress */
	ParameterT& constraint = list.GetParameter("constraint_2D");
	constraint.SetDefault(kPlaneStrain);
}

/***********************************************************************
* Protected
***********************************************************************/

/* print name */
void LocalJ2SSNonlinHard2D::PrintName(ostream& out) const
{
  // inherited
  LocalJ2SSNonlinHard::PrintName(out);

  // output model name
  out << "    Plane Strain\n";
}