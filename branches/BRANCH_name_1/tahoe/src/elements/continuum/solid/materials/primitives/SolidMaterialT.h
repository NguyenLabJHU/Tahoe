/* $Id: SolidMaterialT.h,v 1.4.2.1 2002-06-27 18:03:51 cjkimme Exp $ */
/* created: paklein (11/20/1996) */

#ifndef _STRUCTURAL_MATERIALT_H_
#define _STRUCTURAL_MATERIALT_H_

#include "GlobalT.h"

/* base class */
#include "ContinuumMaterialT.h"

/* direct members */
#include "dMatrixT.h"
#include "ThermalDilatationT.h"

/* forward declarations */

namespace Tahoe {

class ifstreamT;
class ElementBaseT;
class dMatrixT;
class ThermalDilatationT;
class ScheduleT;
class dSymMatrixT;
class LocalArrayT;
class ElasticT;

/** base class for constitutive models for solids */
class SolidMaterialT: public ContinuumMaterialT
{
public:

	/** constructor */
	SolidMaterialT(ifstreamT& in, const ContinuumElementT& element);

	/** destructor */
	~SolidMaterialT(void);

	/** initialization called immediately after constructor. This function
	 * checks if thermal strain are being imposed and if the material
	 * supports thermal strain, using SolidMaterialT::SupportsThermalStrain. */
	virtual void Initialize(void);

	/** write parameters */
	virtual void Print(ostream& out) const;

	/* spatial description */
	virtual const dMatrixT& c_ijkl(void) = 0;  /**< spatial tangent modulus */
	virtual const dSymMatrixT& s_ij(void) = 0; /**< Cauchy stress */

	/* material description */
	virtual const dMatrixT& C_IJKL(void) = 0;  /**< material tangent moduli */
	virtual const dSymMatrixT& S_IJ(void) = 0; /**< 2nd Piola-Kirchhoff stress */

	/** incremental heat generation. Associated with the stress calculated with the
	 * most recent call to SolidMaterialT::s_ij or SolidMaterialT::S_IJ */
	virtual double IncrementalHeat(void);
	
	/** return true if the material generates heat. The returns false unless 
	 * overridden. */
	virtual bool HasIncrementalHeat(void) const;

	/** strain energy density */
	virtual double StrainEnergyDensity(void) = 0;

	/** acoustical tensor.
	 * \param normal wave propagation direction
	 * \return acoustical tensor */
	virtual const dSymMatrixT& AcousticalTensor(const dArrayT& normal) = 0;

	/** acoustic wave speeds.
	 * \param normal wave propagation direction
	 * \param speeds the computed acoustic wave speeds */
	void WaveSpeeds(const dArrayT& normal, dArrayT& speeds);

	/* required parameter flags - all false by default */
	virtual bool NeedDisp(void) const     { return false; };
	virtual bool NeedLastDisp(void) const { return false; };
	virtual bool NeedVel(void) const      { return false; };

	/** returns true if the material has internal forces in the unloaded
	 * configuration, i.e. thermal strains */
	int HasThermalStrain(void) const;

	/** returns the schedule number for the imposed thermal strain */
	int ThermalStrainSchedule(void) const;

	/** set the schedule for the prescribed temperature */
	void SetThermalSchedule(const ScheduleT* LTfPtr);
	
	/** return the thermal expansion rate as a percentage */
	double ThermalElongation(void) const;
	 	
	/** \return mass density */
	double Density(void) const;
	
	/** Rayleigh damping. \return mass proportional damping coefficient */
	double MassDamping(void) const;

	/** Rayleigh damping. \return stiffness proportional damping coefficient */
	double StiffnessDamping(void) const;

	/** test for localization. check for bifurcation using current
	 * Cauchy stress and the spatial tangent moduli.
	 * \param normal orientation of the localization if localized
	 * \return 1 if the determinant of the acoustical tensor is negative
	 * or 0 if the determinant is positive. */
	virtual int IsLocalized(dArrayT& normal);
	
private:

	/** return true if material implementation supports imposed thermal
	 * strains. */
	virtual bool SupportsThermalStrain(void) const { return false; };

protected:

	/* thermal */
	ThermalDilatationT*	fThermal;

	/* mass density */
	double fDensity;

private:	

	double fMassDamp;
	double fStiffDamp;
};

/* incremental heat generation */
inline double SolidMaterialT::IncrementalHeat(void) { return 0.0; }
inline bool SolidMaterialT::HasIncrementalHeat(void) const { return false; }

/* returns the density */
inline double SolidMaterialT::Density(void) const { return fDensity; }

/* access to Rayleigh damping parameters */
inline double SolidMaterialT::MassDamping(void) const { return fMassDamp; }
inline double SolidMaterialT::StiffnessDamping(void) const { return fStiffDamp;}

/* imposed thermal strains */
inline int SolidMaterialT::HasThermalStrain(void) const { return fThermal->IsActive(); }
inline int SolidMaterialT::ThermalStrainSchedule(void) const { return fThermal->ScheduleNum(); }
inline void SolidMaterialT::SetThermalSchedule(const ScheduleT* LTfPtr) { fThermal->SetSchedule(LTfPtr); }
inline double SolidMaterialT::ThermalElongation(void) const { return fThermal->PercentElongation(); }

} // namespace Tahoe 
#endif /* _STRUCTURAL_MATERIALT_H_ */