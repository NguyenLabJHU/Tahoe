/* $Id: LocalCrystalPlast2D.cpp,v 1.6.30.3 2004-03-04 06:45:35 paklein Exp $ */
#include "LocalCrystalPlast2D.h"
#include "ElementCardT.h"
#include "ifstreamT.h"

using namespace Tahoe;

/* spatial dimensions of the problem */
const int kNSD = 2;

LocalCrystalPlast2D::LocalCrystalPlast2D(ifstreamT& in, const FSMatSupportT& support) :
	ParameterInterfaceT("local_crystal_plasticity_2D"),
  LocalCrystalPlast (in, support),  
  f2Dsavg_ij   (kNSD),
  f2Dcavg_ijkl (dSymMatrixT::NumValues(kNSD))
{
 
}

const dSymMatrixT& LocalCrystalPlast2D::s_ij()
{
  // inherited
  const dSymMatrixT& savg_ij = LocalCrystalPlast::s_ij();

  // reduce savg_ij: 3D -> 2D
  f2Dsavg_ij.ReduceFrom3D(savg_ij);

  return f2Dsavg_ij;
}

const dMatrixT& LocalCrystalPlast2D::c_ijkl()
{
  // inherited
  const dMatrixT& cavg_ijkl = LocalCrystalPlast::c_ijkl();

  // reduce cavg_ijkl: 3D -> 2D
  f2Dcavg_ijkl.Rank4ReduceFrom3D(cavg_ijkl);

  return f2Dcavg_ijkl;
}

void LocalCrystalPlast2D::PrintName(ostream& out) const
{
  // inherited
  LocalCrystalPlast::PrintName(out);

  // output 2D case name
  out << "    Plane Strain\n";
}

/* describe the parameters needed by the interface */
void LocalCrystalPlast2D::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	LocalCrystalPlast::DefineParameters(list);
	
	/* 2D option must be plain stress */
	ParameterT& constraint = list.GetParameter("constraint_2D");
	constraint.SetDefault(kPlaneStrain);
}