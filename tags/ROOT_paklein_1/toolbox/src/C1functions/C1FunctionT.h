/* $Id: C1FunctionT.h,v 1.11 2002-10-04 01:32:57 thao Exp $ */
/* created: paklein (12/04/1996) */

#ifndef _C2_FUNCTION_T_H_
#define _C2_FUNCTION_T_H_

#include "Environment.h"

#include "ios_fwd_decl.h"

namespace Tahoe {

/* forward declarations */
class dArrayT;

/** interface for a twice differentiable function */
class C1FunctionT
{
public:

	/* function codes of derived classes */
	enum TypesT {kLennardJones = 0,
	            kSmithFerrante = 1,
	                 kGaoKlein = 2,
	                kQuadratic = 3,
	              kCubicSpline = 4,
	        kLinearExponential = 5,
	          kTriantafyllidis = 6,
                            kGaoJi = 10,
                           kGaoJi2 = 11,
	                 kGaoVicky = 12,
		              kSF2 = 13};

	/* constructor */
	C1FunctionT(void);

	/* destructor */
	virtual ~C1FunctionT(void);
	
	/* I/O */
	virtual void Print(ostream& out) const = 0;     	    	   	
	virtual void PrintName(ostream& out) const = 0;     	    	   	
	    	   	    	
	/* returning values */
	virtual double Function(double x) const = 0;
	virtual double DFunction(double x) const = 0;
	virtual double DDFunction(double x) const = 0;

	/* returning values in groups - derived classes should define
	 * their own non-virtual function called within this functon
	 * which maps in to out w/o requiring a virtual function call
	 * everytime. Default behavior is just to map the virtual functions
	 * above */
	virtual dArrayT& MapFunction(const dArrayT& in, dArrayT& out) const;
	virtual dArrayT& MapDFunction(const dArrayT& in, dArrayT& out) const;
	virtual dArrayT& MapDDFunction(const dArrayT& in, dArrayT& out) const;
	
	/* Return 0th, 1st, and 2nd derivative in the respective
	 * fields of the dArrayT. Default behavior is just to call the
	 * virtual functions above */  	
	virtual void SetAll(double x, dArrayT& data) const;   	
	    	   	    	
};

} // namespace Tahoe 
#endif /* _C2_FUNCTION_T_H_ */