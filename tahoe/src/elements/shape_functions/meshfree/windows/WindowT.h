/* $Id: WindowT.h,v 1.10 2004-10-12 00:20:26 paklein Exp $ */
#ifndef _WINDOW_T_H_
#define _WINDOW_T_H_

#include "ios_fwd_decl.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class dArrayT;
class dArray2DT;
class dSymMatrixT;
template <class TYPE> class ArrayT;

/** base class for various support types, and hence different 
 * window functions */
class WindowT
{
  public:
	
	/** constants to identify the required neighbor search type */
	enum SearchTypeT {kNone = 0,
	             kSpherical = 1,
	           kRectangular = 2,
	          kConnectivity = 3};

	/** constructor */
	WindowT(void) { };  

	/** destructor */
	virtual ~WindowT(void) { };

	/** neighbor search type.
	 * \return search type recommended for construction support size */
	virtual SearchTypeT SearchType(void) const = 0;

	/** nodal field parameters.
	 * \return the number of nodal parameters associated
	 * with the window function */
	virtual int NumberOfSupportParameters(void) const = 0;
	
	/** "synchronization" of nodal field parameters.
	 * reconcile nodal field parameters generated by separate
	 * passes over the same set of nodes.
	 * \param params_1 nodal field parameters from pass 1
	 * \param params_2 nodal field parameters from pass 2 */
	virtual void SynchronizeSupportParameters(dArray2DT& params_1, 
		dArray2DT& params_2) const = 0;	
	/*@}*/

	/** window function name */
	virtual const char* Name(void) const = 0;

	/** write parameters to output stream */
	virtual void WriteParameters(ostream& out) const;

	/** \name single point evaluations */
	/*@{*/
	/** window function evaluation.
	 * \param x_n center of the window function
	 * \param param_n array of window function parameters associated with x_n
	 * \param x field point of evaluation
	 * \param order highest order derivative to be calculated
	 * \param w the value at x of the window function centered at x_n
	 * \param Dw window function derivatives: [nsd]
	 * \param DDw window function second derivatives: [nstr] 
	 * \return true if the support covers the field point */
	virtual bool Window(const dArrayT& x_n, const dArrayT& param_n, const dArrayT& x,
		int order, double& w, dArrayT& Dw, dSymMatrixT& DDw) = 0;

	/** coverage test.
	 * \return true if the window function centered at x_n covers the
	 * point x. */
	virtual bool Covers(const dArrayT& x_n, const dArrayT& x, const dArrayT& param_n) const = 0;

	/** \name spherical upport size
	 * transform the nodal parameters into a size scale that circumscribes
	 * the support of the nodal support */
	/*@{*/
	virtual double SphericalSupportSize(const dArrayT& param_n) const = 0;

	/** \name rectangular support size
	 * transform the nodal parameters into rectangular dimensions that circumscribes
	 * the support of the nodal support */
	/*@{*/
	virtual void RectangularSupportSize(const dArrayT& param_n, dArrayT& support_size) const = 0;
	/*@}*/
	
	/** \name multi-point evaluations */
	/*@{*/
	/** window function evaluation. 
	 * \param x_n array of window function centers: [npts] x [nsd]
	 * \param param_n array of window function parameters: [npts] x [nparam]
	 * \param x field point of evaluation
	 * \param order highest order derivative to be calculated
	 * \param w values of the window function: [npts]
	 * \param w values of the window function derivaties: [npts] x [nsd]
	 * \param w values of the window function derivaties: [npts] x [nstr] 
	 * \return the number of points covered by the window function */
	virtual int Window(const dArray2DT& x_n, const dArray2DT& param_n, const dArrayT& x,
		int order, dArrayT& w, dArray2DT& Dw, dArray2DT& DDw) = 0;	

	/** coverage test.
	 * \param x_n array of window function centers: [npts] x [nsd]
	 * \param x field point of evaluation
	 * \param covers array of coverage test results: [npts] 
	 * \return the number of points covering x */
	virtual int Covers(const dArray2DT& x_n, const dArrayT& x, const dArray2DT& param_n, ArrayT<bool>& covers) const;

	/** compute spherical support size in batch. Default implementation uses the relies on the
	 * purely virtual method WindowT::SupportSize to evaluate the support size over
	 * collection of nodal support parameters. Sub-classes should override this method
	 * for higher efficiency.
	 * \param param_n nodal support parameters for many nodes
	 * \param support_size returns with the support size for associated with the parameters
	 *        in each row of param_n */
	virtual void SphericalSupportSize(const dArray2DT& param_n, ArrayT<double>& support_size) const;

	/** compute rectangular support size in batch. Default implementation uses the relies on the
	 * purely virtual method WindowT::SupportSize to evaluate the support size over
	 * collection of nodal support parameters. Sub-classes should override this method
	 * for higher efficiency.
	 * \param param_n nodal support parameters for many nodes
	 * \param support_size returns with the support size for associated with the parameters
	 *        in each row of param_n */
	virtual void RectangularSupportSize(const dArray2DT& param_n, dArray2DT& support_size) const;
	/*@}*/
};

/* write parameters to output stream */
inline void WindowT::WriteParameters(ostream& out) const
{
#pragma unused(out)
}

} // namespace Tahoe 
#endif /* _WINDOW_T_H_ */
