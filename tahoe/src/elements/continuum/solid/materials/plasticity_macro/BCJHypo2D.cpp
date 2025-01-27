/* $Id: BCJHypo2D.cpp,v 1.7 2004-09-10 22:39:48 paklein Exp $ */
#include "BCJHypo2D.h"
#include "Utils.h"

using namespace Tahoe;

/* spatial dimension of problem */
const int kNSD = 2;

BCJHypo2D::BCJHypo2D(ifstreamT& in, const FSMatSupportT& support) :
	ParameterInterfaceT("BCJHypo_2D"),
	BCJHypo3D   (in, support),  
  f2Ds_ij   (kNSD),
  f2Dc_ijkl (dSymMatrixT::NumValues(kNSD))
{
	/* reset default value */
	fConstraint = kPlaneStrain;
}

const dSymMatrixT& BCJHypo2D::s_ij()
{
  // inherited
  const dSymMatrixT& sij = BCJHypo3D::s_ij();

  // reduce stress: 3D -> 2D
  f2Ds_ij.ReduceFrom3D(sij);

  return f2Ds_ij;
}

const dMatrixT& BCJHypo2D::c_ijkl()
{
  // inherited
  const dMatrixT& cijkl = BCJHypo3D::c_ijkl();

  // reduce cijkl: 3D -> 2D
  f2Dc_ijkl.Rank4ReduceFrom3D(cijkl);

  return f2Dc_ijkl;
}

#if 0
const dMatrixT& BCJHypo2D::DeformationGradient(const LocalArrayT& disp)
{
  // expand total deformation gradient: 2D -> 3D (plane strain)
  fmatx1.Rank2ExpandFrom2D(F(disp));    // fFtot or fFtot_n
  fmatx1(2, 2) = 1.;

  return fmatx1;
}
#endif
