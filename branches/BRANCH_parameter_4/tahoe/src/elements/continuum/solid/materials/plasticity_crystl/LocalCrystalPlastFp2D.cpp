/* $Id: LocalCrystalPlastFp2D.cpp,v 1.5.54.2 2004-07-07 15:28:23 paklein Exp $ */
#include "LocalCrystalPlastFp2D.h"
#include "ElementCardT.h"


using namespace Tahoe;

/* spatial dimensions of the problem */
const int kNSD = 2;

LocalCrystalPlastFp2D::LocalCrystalPlastFp2D(ifstreamT& in, const FSMatSupportT& support) :
	ParameterInterfaceT("local_crystal_plasticity_Fp_2D"),
  LocalCrystalPlastFp (in, support),  
  f2Dsavg_ij   (kNSD),
  f2Dcavg_ijkl (dSymMatrixT::NumValues(kNSD))
{
 
}

const dSymMatrixT& LocalCrystalPlastFp2D::s_ij()
{
  // inherited
  const dSymMatrixT& savg_ij = LocalCrystalPlastFp::s_ij();

  // reduce savg_ij: 3D -> 2D
  f2Dsavg_ij.ReduceFrom3D(savg_ij);

  return f2Dsavg_ij;
}

const dMatrixT& LocalCrystalPlastFp2D::c_ijkl()
{
  // inherited
  const dMatrixT& cavg_ijkl = LocalCrystalPlastFp::c_ijkl();

  // reduce cavg_ijkl: 3D -> 2D
  f2Dcavg_ijkl.Rank4ReduceFrom3D(cavg_ijkl);

  return f2Dcavg_ijkl;
}

/* describe the parameters needed by the interface */
void LocalCrystalPlastFp2D::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	LocalCrystalPlastFp::DefineParameters(list);
	
	/* 2D option must be plain stress */
	ParameterT& constraint = list.GetParameter("constraint_2D");
	constraint.SetDefault(kPlaneStrain);
}
