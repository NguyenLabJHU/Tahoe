/* $Id: RodMaterialT.h,v 1.3 2002-06-29 16:17:22 paklein Exp $ */
/* created: paklein (11/20/1996) */

#ifndef _RODMATERIALT_H_
#define _RODMATERIALT_H_

#include "Environment.h"

/* forward declarations */
#include "ios_fwd_decl.h"
class ifstreamT;
class ThermalDilatationT;
class ScheduleT;

/** pair interactions */
class RodMaterialT
{
public:

	/** constructor */
	RodMaterialT(ifstreamT& in);

	/** destructor */
	virtual ~RodMaterialT(void);

	/** I/O functions */
	virtual void Print(ostream& out) const = 0;
	virtual void PrintName(ostream& out) const = 0;

	/** print parameters */
	void PrintParameters(ostream& out) const;
	
	/** return the particle mass */
	double Mass(void) const { return fMass; };
	
	/** potential function and derivatives */
	virtual double Potential(double rmag, double Rmag) const = 0;
	virtual double DPotential(double rmag, double Rmag) const = 0;
	virtual double DDPotential(double rmag, double Rmag) const = 0;

	/** thermal accessors */
	int ThermalScheduleNumber(void) const;
	void SetThermalSchedule(const ScheduleT* LTfPtr);

	/** returns true if the material has internal forces in the unloaded
	 * configuration, ie thermal strains */
	virtual int HasInternalStrain(void) const;

protected:

	double fMass;
	ThermalDilatationT* fThermal;
};

#endif /* _RODMATERIALT_H_ */