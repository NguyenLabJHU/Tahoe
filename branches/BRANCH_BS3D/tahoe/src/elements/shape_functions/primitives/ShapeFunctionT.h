/* $Id: ShapeFunctionT.h,v 1.19 2004-01-31 07:20:50 paklein Exp $ */
/* created: paklein (06/26/1996) */

#ifndef _SHAPE_FUNCTION_T_H_
#define _SHAPE_FUNCTION_T_H_

/* base class */
#include "DomainIntegrationT.h"

/* direct members */
#include "LocalArrayT.h" /* needed for inlines */

namespace Tahoe {

/** Interface for element shape functions. Controls domain representation
 * and field representation and spatial derivatives. Integration control
 * is inherited. \note equation numbers refer to equations in the Hughes
 * book and in the class notes. */
class ShapeFunctionT: public DomainIntegrationT
{
public:

	/** constructor. 
	 * \param geometry_code geometry of the parent domain
	 * \param numIP number of integration points 
	 * \param coords array of nodal coordinates in local ordering
	 * \param B_option strain-displacement option */
	ShapeFunctionT(GeometryT::CodeT geometry_code, int numIP, 
		const LocalArrayT& coords);
	
	/** constructor. 
	 * \param link shared parent domain and "synch-ed" CurrIP
	 * \param coords array of nodal coordinates in local ordering */
	ShapeFunctionT(const ShapeFunctionT& link, const LocalArrayT& coords);

	/** type of the domain coordinates */
	LocalArrayT::TypeT DomainCoordType(void) const;

	/** compute global shape derivatives */ 	
	virtual void SetDerivatives(void);

	/** array of jacobian determinant integration points at once */
	const double* IPDets(void) const; // d(fCoords) = j d(parent domain)

//DEV - not used
#if 0
	/** jacobian of the mapping to the parent domain */
	double IPDet(void) const; // d(fCoords) = j d(parent domain)
#endif

	/** \name shape function gradients
	 * Return the matrix of shape function gradients for the specified
	 * integration point */
	/*@{*/
	const dArray2DT& Derivatives_X(int ip) const { return fDNaX[ip]; };
	const dArray2DT& Derivatives_X(void) const { return fDNaX[fCurrIP]; };

	const dArray2DT& Derivatives_U(int ip) const { return (*pDNaU)[ip]; };
	const dArray2DT& Derivatives_U(void) const { return (*pDNaU)[fCurrIP]; };
	/*@}*/

	/** coordinates of the given */
	void IPCoords(dArrayT& coordinates, int ip) const;

	/** coordinates of the current integration point */
	void IPCoords(dArrayT& coordinates) const;
	
	/** interpolate field values to the current integration point 
	 * \param nodal array of nodal values: [nnd] x [nu]
	 * \param u interpolation of the nodal values */
	void InterpolateU(const LocalArrayT& nodal, dArrayT& u) const;

	/** interpolate field values to the specified integration point 
	 * \param nodal array of nodal values: [nnd] x [nu]
	 * \param u interpolation of the nodal values 
	 * \param ip integration point number */
	void InterpolateU(const LocalArrayT& nodal, dArrayT& u, int ip) const;
	
	/* Allow stand-alone use of class ShapeFunctionT */
	void InitializeDomain(void) const;
	
	/** array of shape function values defining the geometry */
	const double* IPShapeX(void) const;

	/** array of shape function values defining the field */
	const double* IPShapeU(void) const;

	/** \name field gradients */
	/*@{*/
	/** field gradients at the current integration point. 
	 * \param nodal array of nodal values: [nnd] x [nu]
	 * \param grad_U field gradient matrix: [nu] x [nsd] */
	void GradU(const LocalArrayT& nodal, dMatrixT& grad_U) const;

	/** field gradients at the specified integration point. 
	 * \param nodal array of nodal values: [nnd] x [nu]
	 * \param grad_U field gradient matrix: [nu] x [nsd] 
	 * \param IPnumber integration point number */
	void GradU(const LocalArrayT& nodal, dMatrixT& grad_U, int IPnumber) const;

	/** field gradients at arbitrary parent domain coordinates. 
	 * \param nodal array of nodal values: [nnd] x [nu]
	 * \param grad_U returns with the field gradient matrix: [nu] x [nsd] 
	 * \param coord coordinates in the parent domain
	 * \param Na returns with array of nodal shape functions (dimensioned during the call): [nnd]
	 * \param DNa returns with array of shape function derivatives 
	 *        (dimensioned during the call): [nsd] x [nnd] */
	void GradU(const LocalArrayT& nodal, dMatrixT& grad_U, const dArrayT& coord, 
		dArrayT& Na, dArray2DT& DNa) const;
	/*@}*/

	/** compute the curl of a vector that is of dimension 3x1
	 *  Values for vector at the node points must be provided 
	 *  T is of dimension num_nodes x (3x1) -- an array of vectors
	 *  For 2D case, put zero's in the 3 components of T, and use 2D DNa
	 *  of dimension 2 x num_nodes.
	 *  Note: Return curl(T) will be 3x1 */
	void CurlU(const ArrayT<dArrayT>& T, dArrayT& curl_T, int IPnumber) const;

	/** compute the curl of a tensor that is of dimension 3x3
	 *  Values for tensor at the node points must be provided 
	 *  T is of dimension num_nodes x (3x3) -- an array of tensors
	 *  For 2D case, put zero's in the 3 components of T, and use 2D DNa
	 *  of dimension 2 x num_nodes.
	 *  Note: Return curl(T) will be 3x3 */
	void CurlU(const ArrayT<dMatrixT>& T, dMatrixT& curl_T, int IPnumber) const;

	/** convert derivatives of the enhanced modes by applying a chain rule
	 * transformation:
		\f[
			\frac{\partial N_A}{\partial x_i} =
				\frac{\partial N_A}{\partial X_J} \frac{\partial X_J}{\partial x_i}
		\f]
	 *
	 * \param changeofvar jacobian matrix of the coordinate transformation
	 * \param derivatives transformed shape function derivatives. This array is
	 *        dimensioned during the call: [nsd] x [num_nodes] */
	void TransformDerivatives(const dMatrixT& changeofvar, dArray2DT& derivatives);

	/** shape function gradients matrix at the current integration point
	 * as in Hughes (4.90)
	 * \param grad_Na returns with a matrix of shape function derivatives with
	 *        respect to the coordinates passes to ShapeFunctionT::ShapeFunctionT.
	   \f[
	      \left[\nabla \mathbf{N}\right]_{iA} =
	          \left( \frac{\partial N_A}{\partial x_i} \right)^{T}
	   \f] */
	void GradNa(dMatrixT& grad_Na) const;

	/** shape function gradients matrix as in Hughes (4.90) */
	void GradNa(const dArray2DT& derivatives, dMatrixT& grad_Na) const;

/*******************************************/

	/** print the shape function values to the output stream */
	virtual void Print(ostream& out) const;

protected:

	/** apply change of variables to the shape function derivatives */
	void DoTransformDerivatives(const dMatrixT& changeofvar, const dArray2DT& original,
		dArray2DT& transformed);

	/** set Grad_x matrix. used by the meshfree classes to substitutite a set
	 * of shape function derivatives. the set values are retained until the next 
	 * TopIP/NextIP loop */
	void SetGrad_x(const dArray2DT& Grad_x);

	/** replace field shape function for non-isoparametric formulations */
	void SetUShapeFunctions(const dArray2DT& NaU, const ArrayT<dArray2DT>& DNaU);	

	/** access to the (geometry) shape function derivatives */
	const ArrayT<dArray2DT>& DNaX(void) const;

private:

	/** configure work space arrays. initializes shape function to be
	 * isoparametric */
	void Construct(void);
	
	/** hide access to DomainIntegrationT function */
	const double* IPShape(void) const;

protected:

	/* local coordinates */
	const LocalArrayT& fCoords;

private:

	/* global shape function derivatives */
	dArrayT	fDet;	         // d(fCoords) = j d(parent domain)
	ArrayT<dArray2DT> fDNaX; // geometry: d(phi_X)/d(fCoords)

	/* field shape functions */
	const dArray2DT*         pNaU;  // phi_U
	const ArrayT<dArray2DT>* pDNaU; // d(phi_U)/d(fCoords)

	/* return values */
	const dArray2DT* fGrad_x_temp;
	
	/* work space */
	dArrayT fv1, fv2;
};

/* type of the domain coordinates */
inline LocalArrayT::TypeT ShapeFunctionT::DomainCoordType(void) const
{
	return fCoords.Type();
}

/* data for all integration points at once */
inline const double* ShapeFunctionT::IPDets(void) const
{
	return fDet.Pointer();
}

/************************ for the current integration point *********************/

//DEV - not used
#if 0
inline double ShapeFunctionT::IPDet(void) const
{
#if __option(extended_errorcheck)
/* range checking */
if (fCurrIP < 0 || fCurrIP >= fNumIP) throw ExceptionT::kOutOfRange;
#endif

	return *(fDet.Pointer() + fCurrIP);
}
#endif

inline void ShapeFunctionT::InitializeDomain(void) const
{
	fDomain->Initialize();
}

/* data for the current integration point */
inline const double* ShapeFunctionT::IPShapeX(void) const
{
	return fDomain->Shape(fCurrIP);
}

inline const double* ShapeFunctionT::IPShapeU(void) const
{
	return (*pNaU)(fCurrIP);
}

inline void ShapeFunctionT::IPCoords(dArrayT& coordinates) const
{
	fDomain->Interpolate(fCoords, coordinates, fCurrIP);
}

inline void ShapeFunctionT::IPCoords(dArrayT& coordinates, int ip) const
{
	fDomain->Interpolate(fCoords, coordinates, ip);
}

/* spatial gradients */
inline void ShapeFunctionT::GradU(const LocalArrayT& nodal,
	dMatrixT& grad_U) const
{
	if (fCurrIP != -1)
		fDomain->Jacobian(nodal, (*pDNaU)[fCurrIP], grad_U);
	else
		fDomain->Jacobian(nodal, *fGrad_x_temp, grad_U);
}

inline void ShapeFunctionT::GradU(const LocalArrayT& nodal,
        dMatrixT& grad_U, int IPnumber) const
{
        fDomain->Jacobian(nodal, (*pDNaU)[IPnumber], grad_U);
}

inline void ShapeFunctionT::CurlU(const ArrayT<dArrayT>& T,
        dArrayT& curl_T, int IPnumber) const
{
        fDomain->Curl(T, (*pDNaU)[IPnumber], curl_T);
}

inline void ShapeFunctionT::CurlU(const ArrayT<dMatrixT>& T,
        dMatrixT& curl_T, int IPnumber) const
{
        fDomain->Curl(T, (*pDNaU)[IPnumber], curl_T);
}

#if 0
inline void ShapeFunctionT::B(dMatrixT& B_matrix) const
{
	B((*pDNaU)[fCurrIP], B_matrix);
}
#endif

inline void ShapeFunctionT::GradNa(dMatrixT& grad_Na) const
{
	GradNa((*pDNaU)[fCurrIP], grad_Na);
}

#if 0
inline void ShapeFunctionT::B_q(dMatrixT& B_matrix) const
{
	B_q((*pDNaU)[fCurrIP], B_matrix);
}
#endif

inline void ShapeFunctionT::TransformDerivatives(const dMatrixT& changeofvar, dArray2DT& derivatives)
{
	DoTransformDerivatives(changeofvar, (*pDNaU)[fCurrIP], derivatives);
}

/********************************************************************************/

/* set Grad_x matrix - valid until next TopIP/NextIP loop */
inline void ShapeFunctionT::SetGrad_x(const dArray2DT& Grad_x)
{
	fGrad_x_temp = &Grad_x;

	/* disable IP counter */
	fCurrIP = -1;
}

/* replace field shape function (for non-isoparametric) */
inline void ShapeFunctionT::SetUShapeFunctions(const dArray2DT& NaU,
	const ArrayT<dArray2DT>& DNaU)
{
#if __option(extended_errorcheck)
	if (DNaU.Length() != NumIP()) throw ExceptionT::kSizeMismatch;
#endif

	/* set pointers to external data */
	pNaU  = &NaU;
	pDNaU = &DNaU;
}

/* access to the (geometry) shape function derivatives */
inline const ArrayT<dArray2DT>& ShapeFunctionT::DNaX(void) const { return fDNaX; }

} // namespace Tahoe 
#endif /* _SHAPE_FUNCTION_T_H_ */
