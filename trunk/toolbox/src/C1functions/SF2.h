/* $Id: SF2.h,v 1.1 2002-10-04 20:51:05 thao Exp $ */
/* created: paklein (10/30/1997)                                          */
/* Cohesive force law:                                                    */
/* F(dr) = A dr Exp[-dr^2/B]                                                */
/* where: dr = l - l_0                                                    */
/* 	                                                                      */

#ifndef _SF2_H_
#define _SF2_H_

/* base class */
#include "C1FunctionT.h"

namespace Tahoe{

class SF2: public C1FunctionT
{
public:

	/*
	 * Constructor
	 */
	SF2(double A, double B, double l_0 = 1.0);

	/*
	 * I/O
	 */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	
	/*
	 * Returning values
	 */
	virtual double Function(double x) const;
	virtual double DFunction(double x) const;
	virtual double DDFunction(double x) const;

	/*
	 * Returning values in groups - derived classes should define
	 * their own non-virtual function called within this functon
	 * which maps in to out w/o requiring a virtual function call
	 * everytime. Default behavior is just to map the virtual functions
	 * above.
	 */
	virtual dArrayT& MapFunction(const dArrayT& in, dArrayT& out) const;
	virtual dArrayT& MapDFunction(const dArrayT& in, dArrayT& out) const;
	virtual dArrayT& MapDDFunction(const dArrayT& in, dArrayT& out) const;

private:

	/* potential parameters */
	double fA;
	double fB;
	double fl_0; //equilibrium length
};
}
#endif /* _SMITH_FERRANTE_H_ */
