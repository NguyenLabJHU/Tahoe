/* $Id: ContinuumMaterialT.h,v 1.8.18.1 2004-04-08 07:32:30 paklein Exp $ */
/* created: paklein (11/20/1996) */
#ifndef _CONTINUUM_MATERIAL_T_H_
#define _CONTINUUM_MATERIAL_T_H_

#include "Environment.h"
#include "GlobalT.h"
#include "ios_fwd_decl.h"

/* base class */
#include "ParameterInterfaceT.h"

/* direct members */
#include "MaterialSupportT.h"

namespace Tahoe {

/* forward declarations */
class ElementCardT;
class dArrayT;
template <class TYPE> class ArrayT;
class StringT;

/** interface for continuum materials. */
class ContinuumMaterialT: virtual public ParameterInterfaceT
{
public:

	/** constructor
	 * \param support reference to the host element */
	ContinuumMaterialT(const MaterialSupportT& support);

	/** constructor */
	ContinuumMaterialT(void);

	/** destructor */
	virtual ~ContinuumMaterialT(void);

	/** set the material support or pass NULL to clear */
	virtual void SetMaterialSupport(const MaterialSupportT* support);

	/** form of tangent matrix. \return symmetric by default */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** reference to the material support */
	const MaterialSupportT& MaterialSupport(void) const;

	/** reference to the host element */
	const ContinuumElementT& ContinuumElement(void) const;

	/** number of degrees of freedom (per node) in the host
	 * element group. */
	int NumDOF(void) const;

	/** number of spatial dimensions in the host element group. */
	int NumSD(void) const;

	/** the total number of integration points per element in the
	 * host element group. */
	int NumIP(void) const;

	/** the current integration point within the element of evaluation. */
	int CurrIP(void) const;

	/** return the total number of elements in the host element
	 * group. */
	int NumElements(void) const;

	/** return the number of the current element of evaluation. */
	int CurrElementNumber(void) const;

	/** reference to the ElementCardT for the  specified element. */
	ElementCardT& ElementCard(int card) const;

	/** reference to the ElementCardT for the current element of
	 * evaluation */
	ElementCardT& CurrentElement(void) const;

	/** initialization. Called immediately after constructor to allow
	 * class specific initializations. */
	virtual void Initialize(void);

	/** return true if model needs ContinuumMaterialT::PointInitialize
	 * to be called for every integration point of every element as
	 * part of the model initialization. \return false by default. */
	virtual bool NeedsPointInitialization(void) const;
	
	/** model initialization. Called per integration point for every
	 * element using the model. Deformation variables are available
	 * during this call. */
	virtual void PointInitialize(void);

	/** apply pre-conditions at the current time step. Called once for
	 * the model at the beginning of a time increment */
	virtual void InitStep(void);

	/** finalize the current time step. Called once for the model at 
	 * the end of a time increment */
	virtual void CloseStep(void);

	/** update internal variables. Called once per element for all
	 * elements using the model, hence no deformation variables are
	 * available during this call. */
	virtual void UpdateHistory(void);

	/** restore internal variables to their state at the beginning of
	 * the current time increment. Called once per element for all
	 * elements using the model, hence no deformation variables are
	 * available during this call. */
	virtual void ResetHistory(void);

	/** write parameters to the output stream. */
	virtual void Print(ostream& out) const = 0;

	/** write the model name to the output stream. */
	virtual void PrintName(ostream& out) const = 0;

	/** return the number of constitutive model output parameters
	 * per evaluation point. Used by the host element group in
	 * conjunction with ContinuumMaterialT::OutputLabels and
	 * ContinuumMaterialT::ComputeOutput to collect model variables
	 * for output. \return zero by default */
	virtual int NumOutputVariables(void) const;

	/** return the labels for model output parameters
	 * per evaluation point. Used by the host element group in
	 * conjunction with ContinuumMaterialT::NumOutputVariables and
	 * ContinuumMaterialT::ComputeOutput to collect model variables
	 * for output.
	 * \param labels destination for the variable labels. Returns
	 *        with length ContinuumMaterialT::NumOutputVariables */
	virtual void OutputLabels(Tahoe::ArrayT<StringT>& labels) const;

	/** return material output variables. Used by the host element group 
	 * in conjunction with ContinuumMaterialT::NumOutputVariables and
	 * ContinuumMaterialT::OutputLabels to collect model variables
	 * for output. Called per integration point. Deformation variables
	 * are available.
	 * \param output destination for the output. Must be passed in with
	 *        length ContinuumMaterialT::NumOutputVariables */
	virtual void ComputeOutput(Tahoe::dArrayT& output);

	/** returns true if two materials have compatible output variables.
	 * Used by the host element to determine whether the two material
	 * models can be used within the same host element group when
	 * requesting model-specific, materials output. */
	static bool CompatibleOutput(const ContinuumMaterialT& m1, const ContinuumMaterialT& m2);
	
protected:

	/** support from the host code */
	const MaterialSupportT* fMaterialSupport;
	
	/** number of degrees of freedom */
	int fNumDOF;

	/** number of degrees of spatial dimensions */
	int fNumSD;
	
	/** number of integration points */
	int fNumIP;
};

/* inlines */
inline int ContinuumMaterialT::NumDOF(void) const { return fNumDOF; }
inline int ContinuumMaterialT::NumSD(void) const { return fNumSD; }
inline int ContinuumMaterialT::NumIP(void) const { return fNumIP; }

inline const MaterialSupportT& ContinuumMaterialT::MaterialSupport(void) const
{ 
#if __option(extended_errorcheck)
	if (!fMaterialSupport)
		ExceptionT::GeneralFail("ContinuumMaterialT::MaterialSupport", "material support not set");
#endif
	return *fMaterialSupport; 
}

inline int ContinuumMaterialT::CurrIP(void) const { return MaterialSupport().CurrIP(); };

inline const ContinuumElementT& ContinuumMaterialT::ContinuumElement(void) const { 
	return *(MaterialSupport().ContinuumElement()); 
}

} // namespace Tahoe 

#endif /* _CONTINUUM_MATERIAL_T_H_ */
