/* $Id: HarmonicPairT.h,v 1.3 2003-03-31 23:09:14 paklein Exp $ */
#ifndef _HARMONIC_PAIR_T_H_
#define _HARMONIC_PAIR_T_H_

/* base class */
#include "PairPropertyT.h"

namespace Tahoe {

/** harmonic pair interaction */
class HarmonicPairT: public PairPropertyT
{
public:

	/** constructor */
	HarmonicPairT(double mass, double R0, double K);

	/** \name return interaction functions */
	/*@{*/
	/** return a pointer to the energy function */
	virtual EnergyFunction getEnergyFunction(void);

	/** return a pointer to the force function */
	virtual ForceFunction getForceFunction(void);

	/** return a pointer to the stiffness function */
	virtual StiffnessFunction getStiffnessFunction(void);
	/*@}*/

	/** write properties to output */
	virtual void Write(ostream& out) const;

private:

	/** \name interaction functions */
	/*@{*/
	static double Energy(double r_ab, double* data_a, double* data_b);
	static double Force(double r_ab, double* data_a, double* data_b);
	static double Stiffness(double r_ab, double* data_a, double* data_b);
	/*@}*/

private:

	/** equilibrium spacing */
	double fR0;

	/** stiffness */
	double fK;
	
	/** \name static parameters 
	 * There parameters are use during evaluation of the static interaction 
	 * functions. These are copied to static when a function pointer is requested */
	/*@{*/
	static double sR0;
	static double sK;
	/*@}*/
};

} /* namespace Tahoe */

#endif /* _HARMONIC_PAIR_T_H_ */