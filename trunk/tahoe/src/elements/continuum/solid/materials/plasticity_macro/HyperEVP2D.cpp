/*
  File: HyperEVP2D.cpp
*/

#include "HyperEVP2D.h"
#include "ifstreamT.h"
#include "Utils.h"

#include "ElasticT.h"

/* spatial dimension of problem */
const int kNSD = 2;

HyperEVP2D::HyperEVP2D(ifstreamT& in, const ElasticT& element) :
  HyperEVP3D  (in, element),  
  Material2DT (in, Material2DT::kPlaneStrain),
  f2Ds_ij   (kNSD),
  f2Dc_ijkl (dSymMatrixT::NumValues(kNSD))
{

}

HyperEVP2D::~HyperEVP2D() {} 

const dSymMatrixT& HyperEVP2D::s_ij()
{
  // inherited
  const dSymMatrixT& sij = HyperEVP3D::s_ij();

  // reduce stress: 3D -> 2D
  f2Ds_ij.ReduceFrom3D(sij);
  f2Ds_ij *= fThickness;

  return f2Ds_ij;
}

const dMatrixT& HyperEVP2D::c_ijkl()
{
  // inherited
  const dMatrixT& cijkl = HyperEVP3D::c_ijkl();

  // reduce cijkl: 3D -> 2D
  f2Dc_ijkl.Rank4ReduceFrom3D(cijkl);
  f2Dc_ijkl *= fThickness;

  return f2Dc_ijkl;
}

void HyperEVP2D::Print(ostream& out) const
{
  // inherited
  HyperEVP3D::Print(out);
  Material2DT::Print(out);
}

void HyperEVP2D::PrintName(ostream& out) const
{
  // inherited
  HyperEVP3D::PrintName(out);

  // output model name
  out << "    Plane Strain\n";
}

const dMatrixT& HyperEVP2D::DeformationGradient(const LocalArrayT& disp)
{
  // expand total deformation gradient: 2D -> 3D (plane strain)
  fmatx1.Rank2ExpandFrom2D(F(disp));    // fFtot or fFtot_n
  fmatx1(2, 2) = 1.;

  return fmatx1;
}
