/* $Id: FCCLatticeT.cpp,v 1.2.42.2 2004-06-16 07:13:38 paklein Exp $ */
#include "FCCLatticeT.h"
#include "ParameterContainerT.h"

using namespace Tahoe;

/* number of atoms per shell */
const int atoms_per_shell[] = {6, 3, 12, 6, 12};
const int atoms_in_shells[] = {6, 9, 21, 27, 39};
static int AtomsInShells(int nshells) {
	if (nshells < 0 || nshells > 5) ExceptionT::OutOfRange();
	return atoms_in_shells[nshells-1];
};
const double sqrt2 = sqrt(2.0);
const double sqrt3 = sqrt(3.0);

/* constructor */
FCCLatticeT::FCCLatticeT(int nshells):
	ParameterInterfaceT("CB_lattice_FCC"),
	fNumShells(nshells)
{

}

/* information about subordinate parameter lists */
void FCCLatticeT::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	ParameterInterfaceT::DefineSubs(sub_list);

	sub_list.AddSub("FCC_lattice_orientation", ParameterListT::Once, true);
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* FCCLatticeT::NewSub(const StringT& list_name) const
{
	if (list_name == "FCC_lattice_orientation")
	{
		ParameterContainerT* orientation = new ParameterContainerT(list_name);
		orientation->SetListOrder(ParameterListT::Choice);
	
		ParameterContainerT natural("FCC_natural");
		orientation->AddSub(natural);
		
		ParameterContainerT FCC110("FCC_110");
		FCC110.SetDescription("xy-plane into [110]");
		orientation->AddSub(FCC110);

		ParameterContainerT FCC111("FCC_111");
		ParameterT FCC111_type(ParameterT::Enumeration, "sense");
		FCC111_type.AddEnumeration("[-1 1 0][-1-1 2][ 1 1 1]", 0);
		FCC111_type.AddEnumeration("[ 1-1 0][ 1 1-2][ 1 1 1]", 1);
		FCC111_type.SetDefault(0);
		FCC111.AddParameter(FCC111_type);
		orientation->AddSub(FCC111);

		ParameterContainerT Euler_angles("FCC_Euler_angles");
		Euler_angles.AddParameter(ParameterT::Double, "theta");
		Euler_angles.AddParameter(ParameterT::Double, "phi");
		Euler_angles.AddParameter(ParameterT::Double, "psi");
		orientation->AddSub(Euler_angles);
	
		return orientation;
	}
	else /* inherited */
		return ParameterInterfaceT::NewSub(list_name);
}

/* accept parameter list */
void FCCLatticeT::TakeParameterList(const ParameterListT& list)
{
	const char caller[] = "FCCLatticeT::TakeParameterList";

	/* inherited */
	ParameterInterfaceT::TakeParameterList(list);

	/* extract orientation */
	const char orientation_choice[] = "FCC_lattice_orientation";
	const ParameterListT& orientation = list.GetListChoice(*this, orientation_choice);
	OrientationCodeT code = kFCC3Dnatural;
	if (orientation.Name() == "FCC_110")
		code = kFCC3D110;
	else if (orientation.Name() == "FCC_111") {
		int sense = orientation.GetParameter("sense");
		code = (sense == 1) ? kFCC3D111_b : kFCC3D111_a;
	}
	else if (orientation.Name() == "FCC_Euler_angles")
	{
		double theta = orientation.GetParameter("theta");
		double phi = orientation.GetParameter("phi");
		double psi = orientation.GetParameter("psi");
		code = kEulerAngles;
		ExceptionT::GeneralFail(caller, "orientation \"%s\" not implemented", orientation.Name().Pointer());
	}
	else
		ExceptionT::GeneralFail(caller, "unrecognized orientation \"%s\"", orientation.Name().Pointer());
	
	/* set Q */
	dMatrixT Q(3);
	if (code != kEulerAngles)
		SetQ(code, Q);
	
	/* initialize bond table */
	Initialize(&Q);
}

/* set the transformation matrix for the given orientation */
void FCCLatticeT::SetQ(OrientationCodeT orientation, dMatrixT& Q)
{
	/* dimension */
	Q.Dimension(3);
	Q = 0.0;

	switch (orientation)
	{
		case kFCC3Dnatural:
		{
			Q.Identity();
			break;
		}	
		case kFCC3D110:
		{
			double cos45 = 0.5*sqrt2;
			
			/* transform global xy-plane into [110] */
			Q(0,0) = 1.0;
			Q(1,1) = Q(2,2) = cos45;
			Q(1,2) =-cos45;
			Q(2,1) = cos45;
			break;
		}	
		case kFCC3D111_a:
		{
			double rt2b2 = sqrt2/2.0;
			double rt3b3 = sqrt3/3.0;
			double rt6b6 = (sqrt2*sqrt3)/6.0;
			double rt23  = sqrt2/sqrt3;
			
			Q(0,0) =-rt2b2;
			Q(0,1) =-rt6b6;
			Q(0,2) = rt3b3;
			
			Q(1,0) = rt2b2;
			Q(1,1) =-rt6b6;
			Q(1,2) = rt3b3;
			
			Q(2,0) = 0.0;
			Q(2,1) = rt23;
			Q(2,2) = rt3b3;
			break;
		}
		case kFCC3D111_b:
		{
			double rt2b2 = sqrt2/2.0;
			double rt3b3 = sqrt3/3.0;
			double rt6b6 = (sqrt2*sqrt3)/6.0;
			double rt23  = sqrt2/sqrt3;
			
			Q(0,0) = rt2b2;
			Q(0,1) = rt6b6;
			Q(0,2) = rt3b3;
			
			Q(1,0) =-rt2b2;
			Q(1,1) = rt6b6;
			Q(1,2) = rt3b3;
			
			Q(2,0) = 0.0;
			Q(2,1) =-rt23;
			Q(2,2) = rt3b3;
			break;
		}
		default:
			ExceptionT::BadInputValue("FCCLatticeT::SetQ", "unrecognized orientation %d", orientation);
	}
}

/*************************************************************************
 * Protected
 *************************************************************************/

/* initialize bond table values */
void FCCLatticeT::LoadBondTable(void)
{
	/* dimension work space */
	int num_bonds = AtomsInShells(fNumShells);
	fBondCounts.Dimension(num_bonds);
	fDefLength.Dimension(num_bonds);
	fBonds.Dimension(num_bonds, 3);

	/* initialize */
  	fBondCounts = 1;
  	fDefLength = 0.0; 

  	double bonddata1[6*3] =
	{ 1.0/sqrt2, 1.0/sqrt2,       0.0,
	 -1.0/sqrt2, 1.0/sqrt2,       0.0,
	  1.0/sqrt2,       0.0, 1.0/sqrt2,
	 -1.0/sqrt2,       0.0, 1.0/sqrt2,
	        0.0, 1.0/sqrt2, 1.0/sqrt2,
	        0.0,-1.0/sqrt2, 1.0/sqrt2};

  	double bonddata2[3*3] =
	{sqrt2,   0.0,   0.0,
	   0.0, sqrt2,   0.0,
	   0.0,   0.0, sqrt2};

  	double bonddata3[12*3] =
	{     sqrt2, 1.0/sqrt2, 1.0/sqrt2,
      1.0/sqrt2,     sqrt2, 1.0/sqrt2,
      1.0/sqrt2, 1.0/sqrt2,     sqrt2,
         -sqrt2, 1.0/sqrt2, 1.0/sqrt2,
     -1.0/sqrt2,     sqrt2, 1.0/sqrt2,
     -1.0/sqrt2, 1.0/sqrt2,     sqrt2,
          sqrt2,-1.0/sqrt2, 1.0/sqrt2,
      1.0/sqrt2,    -sqrt2, 1.0/sqrt2,
      1.0/sqrt2,-1.0/sqrt2,     sqrt2,
         -sqrt2,-1.0/sqrt2, 1.0/sqrt2,
     -1.0/sqrt2,    -sqrt2, 1.0/sqrt2,
     -1.0/sqrt2,-1.0/sqrt2,     sqrt2};

  	double bonddata4[6*3] =
	{ sqrt2, sqrt2,  0.0,
     -sqrt2, sqrt2,  0.0,
      sqrt2,   0.0, sqrt2,
     -sqrt2,   0.0, sqrt2,
        0.0, sqrt2, sqrt2,
        0.0,-sqrt2, sqrt2};

  	double bonddata5[12*3] =
	{1.0/sqrt2, 3.0/sqrt2,      0.0,
    -1.0/sqrt2, 3.0/sqrt2,      0.0,
     3.0/sqrt2, 1.0/sqrt2,      0.0,
     3.0/sqrt2,-1.0/sqrt2,      0.0,
     1.0/sqrt2,       0.0, 3.0/sqrt2,
    -1.0/sqrt2,       0.0, 3.0/sqrt2,
     3.0/sqrt2,       0.0, 1.0/sqrt2,
    -3.0/sqrt2,       0.0, 1.0/sqrt2,
           0.0, 3.0/sqrt2, 1.0/sqrt2,
           0.0,-3.0/sqrt2, 1.0/sqrt2,
           0.0, 1.0/sqrt2, 3.0/sqrt2,
           0.0,-1.0/sqrt2, 3.0/sqrt2};

	double* shells[5];
	shells[0] = bonddata1;
	shells[1] = bonddata2;
	shells[2] = bonddata3;
	shells[3] = bonddata4;
	shells[4] = bonddata5;

	int bond = 0;
	for (int i = 0; i < fNumShells; i++)
	{
		dArray2DT bonds(atoms_per_shell[i], 3, shells[i]);
		for (int j = 0; j < bonds.MajorDim(); j++)
		{
			fBonds(bond,0) = bonds(j,0);
			fBonds(bond,1) = bonds(j,1);
			fBonds(bond,2) = bonds(j,2);
			bond++;
		}
	}
}