/* $Id: SSCubic2DT.cpp,v 1.5 2002-10-20 22:48:43 paklein Exp $ */
/* created: paklein (06/11/1997)                                          */

#include "SSCubic2DT.h"
#include "ThermalDilatationT.h"

/* constructor */

using namespace Tahoe;

SSCubic2DT::SSCubic2DT(ifstreamT& in, const SmallStrainT& element):
	SSCubicT(in, element),
	Anisotropic2DT(in),
	Material2DT(in)
{
	/* account for thickness */
	fDensity *= fThickness;
}

/* print parameters */
void SSCubic2DT::Print(ostream& out) const
{
	/* inherited */
	SSCubicT::Print(out);
	Anisotropic2DT::Print(out);
	Material2DT::Print(out);
}

double SSCubic2DT::Pressure(void) const
{
	if (ConstraintOption() == kPlaneStress)
		return SSCubicT::Pressure();
	else {
		cout << "\n SSCubic2DT::Pressure: not implemented for plane strain" << endl;
		throw ExceptionT::kGeneralFail;
	}
}

/*************************************************************************
* Protected
*************************************************************************/

/* set (material) tangent modulus */
void SSCubic2DT::SetModulus(dMatrixT& modulus)
{
	/* compute modulus in crystal coordinates */
	CubicT::ComputeModuli2D(modulus, fConstraintOption);
	modulus *= fThickness;
	
	/* transform modulus into global coords */
	TransformOut(modulus);
}

/*************************************************************************
* Private
*************************************************************************/

/* set the internal thermal strain */
bool SSCubic2DT::SetThermalStrain(dSymMatrixT& thermal_strain)
{
	thermal_strain = 0.0;
	if (fThermal->IsActive())
	{
		double factor = CubicT::DilatationFactor2D(fConstraintOption);
		thermal_strain.PlusIdentity(factor*fThermal->PercentElongation());
		return true;
	}
	else
		return false;
}