/* $Id: IC_CardT.h,v 1.3 2002-06-08 20:20:53 paklein Exp $ */
/* created: paklein (07/16/1997) */

#ifndef _IC_CARD_T_H_
#define _IC_CARD_T_H_

#include "Environment.h"

/* forward declarations */
#include "ios_fwd_decl.h"
class ifstreamT;

/** container class for kinematic initial condition data.
 * Handles mainly I/O and provides access to data via (inline) accessors */
class IC_CardT
{
public:

	/** constructor */
	IC_CardT(void);

	/** \name modifiers */
	/*@{*/
	void SetValues(ifstreamT& in);
	void SetValues(int node, int dof, int order, double value);
	/*@}*/
	
	/** \name accessors */
	/*@{*/
	int Node(void) const;
	int DOF(void) const;
	int Order(void) const;
	double Value(void) const;
	/*@}*/

	/** \name I/O methods */
	/*@{*/
	static void WriteHeader(ostream& out);
	void WriteValues(ostream& out) const;
	/*@}*/
	
private:

	int    fnode;
	int    fdof;
	int    forder; /**< time derivative */
	double fvalue;			
};

/* inline functions */

/* accessors */
inline int IC_CardT::Node(void) const     { return fnode;  }
inline int IC_CardT::DOF(void) const      { return fdof;   }
inline int IC_CardT::Order(void) const    { return forder; }
inline double IC_CardT::Value(void) const { return fvalue; }

#endif /* _IC_CARD_T_H_ */
