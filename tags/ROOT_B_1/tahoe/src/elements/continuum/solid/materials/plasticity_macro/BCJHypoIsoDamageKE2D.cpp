/*
  File: BCJHypoIsoDamageKE2D.cpp
*/

#include "BCJHypoIsoDamageKE2D.h"
#include "ifstreamT.h"
#include "Utils.h"

/* spatial dimension of problem */

using namespace Tahoe;

const int kNSD = 2;

BCJHypoIsoDamageKE2D::BCJHypoIsoDamageKE2D(ifstreamT& in, const FiniteStrainT& element) :
  BCJHypoIsoDamageKE3D   (in, element),  
  Material2DT (in, Material2DT::kPlaneStrain),
  f2Ds_ij   (kNSD),
  f2Dc_ijkl (dSymMatrixT::NumValues(kNSD))
{

}

BCJHypoIsoDamageKE2D::~BCJHypoIsoDamageKE2D() {} 

const dSymMatrixT& BCJHypoIsoDamageKE2D::s_ij()
{
  // inherited
  const dSymMatrixT& sij = BCJHypoIsoDamageKE3D::s_ij();

  // reduce stress: 3D -> 2D
  f2Ds_ij.ReduceFrom3D(sij);
  f2Ds_ij *= fThickness;

  return f2Ds_ij;
}

const dMatrixT& BCJHypoIsoDamageKE2D::c_ijkl()
{
  // inherited
  const dMatrixT& cijkl = BCJHypoIsoDamageKE3D::c_ijkl();

  // reduce cijkl: 3D -> 2D
  f2Dc_ijkl.Rank4ReduceFrom3D(cijkl);
  f2Dc_ijkl *= fThickness;

  return f2Dc_ijkl;
}

void BCJHypoIsoDamageKE2D::Print(ostream& out) const
{
  // inherited
  BCJHypoIsoDamageKE3D::Print(out);
  Material2DT::Print(out);
}

void BCJHypoIsoDamageKE2D::PrintName(ostream& out) const
{
  // inherited
  BCJHypoIsoDamageKE3D::PrintName(out);

  // output model name
  out << "    Plane Strain\n";
}
