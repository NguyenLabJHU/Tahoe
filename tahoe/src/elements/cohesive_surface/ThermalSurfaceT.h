/* $Id: ThermalSurfaceT.h,v 1.1.2.1 2002-05-04 20:22:03 paklein Exp $ */

#ifndef _THERMAL_SURFACE_T_H_
#define _THERMAL_SURFACE_T_H_

/* base class */
#include "CSEBaseT.h"

/* direct members */
#include "pArrayT.h"

/* forward declarations */
class C1FunctionT;

/** surface with thermal resistance */
class ThermalSurfaceT: public CSEBaseT
{
public:

	/** constructor */
	ThermalSurfaceT(const ElementSupportT& support, const FieldT& field);

	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** initialize class data */
	virtual void Initialize(void);

protected:

	/** tangent matrix */
	virtual void LHSDriver(void);

	/** force vector */
	virtual void RHSDriver(void);

	/** compute output values */
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
		const iArrayT& e_codes, dArray2DT& e_values);

private:

	/** conduction parameters */
	enum ParametersT {
		kK_0 = 0, /**< initial conduction */
		kd_c = 1  /**< opening to zero conductivity */
	};
	
private:

	/** \name conduction parameters
	 * This is temporary. There parameters will be generalized later. */
	/*@{*/
	dArray2DT fConduction;
	/*@}*/
	
	/** nodal temperatures */
	LocalArrayT fLocTemperatures;
};

#endif /* _THERMAL_SURFACE_T_H_ */
