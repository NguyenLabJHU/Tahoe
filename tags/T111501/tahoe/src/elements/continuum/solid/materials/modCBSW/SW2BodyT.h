/* $Id: SW2BodyT.h,v 1.1.1.1 2001-01-29 08:20:26 paklein Exp $ */
/* created: paklein (05/20/1997)                                          */

#ifndef _SW2_BODY_T_H_
#define _SW2_BODY_T_H_

/* base class */
#include "TwoBodyT.h"

/* forward declarations */
class SWDataT;

class SW2BodyT: public TwoBodyT
{
public:

	/* constructor */
	SW2BodyT(const dArrayT& lengths, const ThermalDilatationT* thermal,
		const SWDataT& SW);

	/* set free dof - triggers recomputation */
	virtual void Set(void);

private:

	/* 2 body potential and derivatives */
	double U2body(double r, double a) const;
	double DU2body(double r, double a) const;
	double DDU2body(double r, double a) const;

private:

	/* Stillinger-Weber parameters */
	const SWDataT&	fSW;

};

#endif /* _SW2_BODY_T_H_ */
