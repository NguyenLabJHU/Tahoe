/* $Id: DPSSKStV2D.h,v 1.6.6.1 2002-06-27 18:03:34 cjkimme Exp $ */
/* created: myip (06/01/1999)                                    */

#ifndef _DP_SS_KSTV_2D_H_
#define _DP_SS_KSTV_2D_H_

/* base class */
#include "Material2DT.h"
#include "DPSSKStV.h"


namespace Tahoe {

class DPSSKStV2D: public DPSSKStV, public Material2DT
{
  public:

	/* constructor */
	DPSSKStV2D(ifstreamT& in, const SmallStrainT& element);

	/* initialization */
	virtual void Initialize(void);

	/* returns elastic strain (3D) */
	virtual const dSymMatrixT& ElasticStrain(
                const dSymMatrixT& totalstrain, 
		const ElementCardT& element, int ip);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	
	/* modulus */
	virtual const dMatrixT& c_ijkl(void);
	virtual const dMatrixT& cdisc_ijkl(void);
  	
	/* stress */
	virtual const dSymMatrixT& s_ij(void);

	/* returns the strain energy density for the specified strain */
	virtual double StrainEnergyDensity(void);

  private:
  
  	/* return values */
  	dSymMatrixT	fStress2D;
  	dMatrixT	fModulus2D;

	/* work space */
	dSymMatrixT	fTotalStrain3D;
};

} // namespace Tahoe 
#endif /* _DP_SS_KSTV_2D_H_ */
