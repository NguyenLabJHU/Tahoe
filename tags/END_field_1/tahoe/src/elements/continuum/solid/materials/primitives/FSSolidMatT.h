/* $Id: FSSolidMatT.h,v 1.4.2.1 2002-05-03 09:48:05 paklein Exp $ */
/* created: paklein (06/09/1997) */

#ifndef _FD_STRUCT_MAT_T_H_
#define _FD_STRUCT_MAT_T_H_

/* base class */
#include "StructuralMaterialT.h"
#include "TensorTransformT.h"

/* forward declarations */
class FiniteStrainT;

/** base class for finite deformation constitutive models. The interface
 * provides access to the element-computed deformation as well as
 * functions to compute the Green Lagragian strain tensor \b E
 * and the stretch tensors \b C and \b b. The class provides support
 * for a multiplicative thermal strain:\n
 * F_total = F_mechanical F_thermal\n
 * where the total deformation gradient is available through
 * FSSolidMatT::F_total, the "mechanical" part of the deformation
 * gradient is available through FSSolidMatT::F_total, and the
 * \a inverse of the thermal deformation gradient is available
 * through FSSolidMatT::F_thermal_inverse. */
class FSSolidMatT: public StructuralMaterialT, protected TensorTransformT
{
public:

	/** constructor */
	FSSolidMatT(ifstreamT& in, const FiniteStrainT& element);

	/** initialization. If active, initialize the history of
	 * prescribed thermal strains. */
	virtual void Initialize(void);

	/** write name to output stream */
	virtual void PrintName(ostream& out) const;
	
	/** test for localization. check for bifurvation using current
	 * Cauchy stress and the spatial tangent moduli.
	 * \param normal orientation of the localization if localized
	 * \return 1 if the determinant of the acoustical tensor is negative
	 * or 0 if the determinant is positive. */
	virtual int IsLocalized(dArrayT& normal);

	/** initialize current step. compute thermal dilatation */
	virtual void InitStep(void);

	/** close current step. store thermal dilatation */
	virtual void CloseStep(void);

	/** required parameter flag. Indicates whether the constitutive model
	 * requires calculation of the deformation gradient.
	 * \return true by default. */
	virtual bool Need_F(void) const { return true; };

	/** required parameter flag. Indicates whether the constitutive model
	 * requires the deformation gradient from the previous time increment.
	 * \return false by default. */
	virtual bool Need_F_last(void) const { return false; };

	/** total deformation gradient. \note This function is on its
	 * way out. Use FSSolidMatT::F_total */
	const dMatrixT& F(void) const; 

	/** total deformation gradient at the given integration point. \note This 
	 * function is on its way out. Use FSSolidMatT::F_total */
	const dMatrixT& F(int ip) const; 

	/** total deformation gradient */
	const dMatrixT& F_total(void) const; 

	/** total deformation gradient at the given integration point */
	const dMatrixT& F_total(int ip) const;

	/** mechanical part of the deformation gradient. The part of the
	 * deformation gradient not associated with an imposed thermal
	 * strain. */
	const dMatrixT& F_mechanical(void); 

	/** mechanical part of the deformation gradient at the given integration
	 * point. The part of the deformation gradient not associated with an 
	 * imposed thermal strain. */
	const dMatrixT& F_mechanical(int ip);

	/** total deformation gradient from end of previous step */
	const dMatrixT& F_total_last(void) const; 

	/** total deformation gradient at the given integration point 
	 * from end of previous step */
	const dMatrixT& F_total_last(int ip) const;
	
	/** inverse of the deformation gradient associated with the
	 * imposed thermal strain */
	const dMatrixT& F_thermal_inverse(void) const { return fF_therm_inv; };

	/** inverse of the deformation gradient associated with the
	 * imposed thermal strain from the previous time increment. */
	const dMatrixT& F_thermal_inverse_last(void) const { return fF_therm_inv_last; };

	/** mechanical part of the deformation gradient from the previous
	 * time increment. The part of the deformation gradient not associated 
	 * with an imposed thermal strain. */
	const dMatrixT& F_mechanical_last(void); 

	/** mechanical part of the deformation gradient from the previous
	 * time increment at the given integration point. The part of the 
	 * deformation gradient not associated 
	 * with an imposed thermal strain. */
	const dMatrixT& F_mechanical_last(int ip);
	
protected:

	/** left stretch tensor.
	 * \param F the deformation gradient
	 * \param b return value */
	void Compute_b(const dMatrixT& F, dSymMatrixT& b) const;

	/** right stretch tensor. 
	 * \param F the deformation gradient
	 * \param C return value */
	void Compute_C(const dMatrixT& F, dSymMatrixT& C) const;

	/** Green-Lagrangian strain.
	 * \param F the deformation gradient
	 * \param E return value */
	void Compute_E(const dMatrixT& F, dSymMatrixT& E) const;

	/** left stretch tensor.
	 * \note this version is being removed. Use the version
	 * which requires the deformation to be passed in
	 * \param b return value */
	void Compute_b(dSymMatrixT& b) const;

	/** right stretch tensor. 
	 * \note this version of is being removed. Use the version
	 * which requires the deformation to be passed in
	 * \param C return value */
	void Compute_C(dSymMatrixT& C) const;

	/** Green-Lagrangian strain. 
	 * \note this version is being removed. Use the version
	 * which requires the deformation to be passed in
	 * \param E return value */
	void Compute_E(dSymMatrixT& E) const;

	/** acoustical tensor.
	 * \param normal wave propagation direction
	 * \return acoustical tensor */
	virtual const dSymMatrixT& AcousticalTensor(const dArrayT& normal);

	/** finite strain element group.
	 * allows access to all const functions of the finite strain element
	 * class that are not currently supported with wrappers.
	 * \return a const reference to the supporting element group */
	const FiniteStrainT& FiniteStrain(void) const { return fFiniteStrain; }

private:

	/* set inverse of thermal transformation - return true if active */
	virtual bool SetInverseThermalTransformation(dMatrixT& F_trans_inv);

	/** compute acoustical tensor in 2D.
	 * \param CIJKL material tangent modulus
	 * \param SIJ 2nd Piola-Kirchhoff stress 
	 * \param FkK deformation gradient
	 * \param N wave propogation direction
	 * \param Q resulting acoustical tensor */
	void ComputeQ_2D(const dMatrixT& CIJKL, const dSymMatrixT& SIJ,
		const dMatrixT& FkK, const dArrayT& N, dSymMatrixT& Q) const;

	/** compute acoustical tensor in 3D.
	 * \param CIJKL material tangent modulus
	 * \param SIJ 2nd Piola-Kirchhoff stress 
	 * \param FkK deformation gradient
	 * \param N wave propogation direction
	 * \param Q resulting acoustical tensor */
	void ComputeQ_3D(const dMatrixT& CIJKL, const dSymMatrixT& SIJ,
		const dMatrixT& FkK, const dArrayT& N, dSymMatrixT& Q) const;

private:

	/** reference to finite deformation element group */
	const FiniteStrainT& fFiniteStrain;

	/** return value for FSSolidMatT::AcousticalTensor */
	dSymMatrixT fQ;  

	/** inverse of the multiplicative thermal deformation gradient */
	dMatrixT fF_therm_inv;

	/** inverse of the multiplicative thermal deformation gradient
	 * from the previous time increment. */
	dMatrixT fF_therm_inv_last;
	
	/** return value. Used as the return value of the mechanical part
	 * of the deformation gradient, if there are thermal strain. Otherwise,
	 * is unused. */
	dMatrixT fF_mechanical;
	
	/** true if temperature field found during FSSolidMatT::Initialize */
	bool fTemperatureField;
	dArrayT fTemperature;
};

#endif /* _FD_STRUCT_MAT_T_H_ */
