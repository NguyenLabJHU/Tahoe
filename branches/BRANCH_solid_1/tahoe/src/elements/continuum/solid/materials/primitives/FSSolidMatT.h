/* $Id: FSSolidMatT.h,v 1.1.1.1.2.2 2001-06-07 03:01:26 paklein Exp $ */
/* created: paklein (06/09/1997)                                          */
/* Defines the interface large strain materials which account             */
/* for thermal strains with the multiplicative split:                     */
/* F_tot = F_(the rest)*F_thermal                                         */
/* Note: At this point, no optimizations are added to compute             */
/* the inverse of F_thermal only once per time step.                      */

#ifndef _FD_STRUCT_MAT_T_H_
#define _FD_STRUCT_MAT_T_H_

/* base class */
//#include "FDContinuumT.h"
//DEV
#include "StructuralMaterialT.h"
#include "TensorTransformT.h"

/* forward declarations */
class ShapeFunctionT;

/** base class for finite deformation constitutive models */
class FSSolidMatT: /* DEV - protected FDContinuumT */
public StructuralMaterialT, protected TensorTransformT
{
public:

	/** constructor */
	FSSolidMatT(ifstreamT& in, const ElasticT& element);

	/** write name to output stream */
	virtual void PrintName(ostream& out) const;

	/** required parameter flags. \return true */
	virtual bool NeedDisp(void) const { return true; };

	/** initialization. call immediately after constructor */
	virtual void Initialize(void);
	
	/** test for localization. check for bifurvation using current
	 * Cauchy stress and the spatial tangent moduli.
	 * \param normal orientation of the localization if localized
	 * \return 1 if the determinant of the acoustical tensor is negative
	 * or 0 if the determinant is positive. */
	virtual int IsLocalized(dArrayT& normal);

	/** initialize step. compute thermal dilatation */
	virtual void InitStep(void);

	/* the shape functions */
//	const ShapeFunctionT& ShapeFunction(void) const;
//DEV
	
protected:

	/** deformation gradient */
	const dMatrixT& F(void) const; 

	/** deformation gradient from end of previous step */
	const dMatrixT& F_last(void) const; 

	/** left stretch tensor. \param b return value */
	void Compute_b(dSymMatrixT& b) const { b.MultAAT(F()); };

	/** right stretch tensor. \param b return value */
	void Compute_C(dSymMatrixT& C) const { C.MultATA(F()); };

	/** Green-Lagrangian strain. \param E return value */
	void Compute_E(dSymMatrixT& E) const;



//	const dMatrixT& F(const LocalArrayT& disp); 	
//	const dSymMatrixT& C(void); // right stretch
//	const dSymMatrixT& b(void); // left stretch
//	const dSymMatrixT& E(void); // Green-Lagrange strain
//DEV

	/** acoustical tensor.
	 * \param normal wave propagation direction
	 * \return acoustical tensor */
	virtual const dSymMatrixT& AcousticalTensor(const dArrayT& normal);

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

	/* shape functions */
//	const ShapeFunctionT& fShapes;
//DEV
	
	/* reference to nodal displacements */
//	const LocalArrayT& fLocDisp;
//DEV

	/* work space */
	dSymMatrixT fQ;  /**< return value */
//	dMatrixT fGradU; /**< displacement gradient matrix */
//DEV	

	/* multiplicative thermal dilatation F */
	dMatrixT fFtherminverse;		
};

/* inlines */
//inline const ShapeFunctionT& FSSolidMatT::ShapeFunction(void) const { return fShapes; }
//DEV

#endif /* _FD_STRUCT_MAT_T_H_ */
