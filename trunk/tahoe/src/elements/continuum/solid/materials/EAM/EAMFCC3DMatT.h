/* $Id: EAMFCC3DMatT.h,v 1.2 2001-07-03 01:35:01 paklein Exp $ */
/* created: paklein (10/25/1998)                                          */
/* Plane strain EAM material                                              */

#ifndef _EAMFCC3DMatT_H_
#define _EAMFCC3DMatT_H_

/* base class */
#include "NL_E_MatT.h"

/* forward declarations */
class EAMFCC3DSym;

class EAMFCC3DMatT: public NL_E_MatT
{
public:

	/* orientation codes - for crystal axes rotated wrt global axes*/
	enum OrientationCodeT {
	kFCC3Dnatural = 0,
	    kFCC3D110 = 1,
	  kFCC3D111_a = 2, // x,y,z = <-1 1 0> <-1-1 2> < 1 1 1>
	  kFCC3D111_b = 3, // x,y,z = < 1-1 0> < 1 1-2> < 1 1 1>
	  kPrescribed = 4};

	/* constructor */
	EAMFCC3DMatT(ifstreamT& in, const FiniteStrainT& element);

	/* destructor */
	virtual ~EAMFCC3DMatT(void);
	
	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;

protected:

	/* compute the symetric Cij reduced index matrix */
	virtual void ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli);	
	
	/* compute the symetric 2nd Piola-Kirchhoff reduced index vector */
	virtual void ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2);

	/* returns the strain energy density for the specified strain */
	virtual double ComputeEnergyDensity(const dSymMatrixT& E);
	
protected:
	
	int fOrientCode;
	int	fEAMCode;
	
	/* EAM solver */
	EAMFCC3DSym* fEAM;
	
};

#endif /* _EAMFCC3DMatT_H_ */
