/* $Id: IC_CardT.cpp,v 1.6 2002-02-27 16:47:49 paklein Exp $ */
/* created: paklein (07/16/1997)                                          */
/* Container class for kinematic initial condition data.                  */
/* Handles mainly I/O and provides access to data via                     */
/* (inline) accessors.                                                    */

#include "IC_CardT.h"

#include <iostream.h>
#include <iomanip.h>

#include "Constants.h"
#include "ExceptionCodes.h"
#include "fstreamT.h"

/* copy behavior for arrays IC_CardT's */
const bool ArrayT<IC_CardT*>::fByteCopy = true;
const bool ArrayT<IC_CardT>::fByteCopy = false;
	
/* default constructor */
IC_CardT::IC_CardT(void): fnode(-1), fdof(-1), fvalue(0.0)			
{
	//initialize to inappropriate values
}

/* modifiers */
void IC_CardT::SetValues(ifstreamT& in)
{
	/* parameters */
	int    node;
	int    dof;
	CodeT  code;
	double value;

	/* read */
	in >> node >> dof >> code >> value;
	
	/* correct offset */
	if (node > 0) node--;
	dof--;
	
	/* set and echo */
	SetValues(node, dof, code, value);
}

void IC_CardT::SetValues(int node, int dof, CodeT code, double value)
{
	/* set */
	fnode  = node;
	fdof   = dof;
	fcode  = code;
	fvalue = value;			

	/* check */
	if (fcode < kDsp || fcode > kAcc) throw eBadInputValue;
}

/* I/O */
void IC_CardT::WriteHeader(ostream& out) const
{
	int d_width = out.precision() + kDoubleExtra;
	out << setw(kIntWidth)    << "node";
	out << setw(kIntWidth)    << "dof";
	out << setw(kIntWidth)    << "code";
	out << setw(d_width)      << "value" << '\n';
}

void IC_CardT::WriteValues(ostream& out) const
{
	/* output format */
	int d_width = out.precision() + kDoubleExtra;

	/* echo */
	if (fnode == -1)
		out << setw(kIntWidth)    << "(ALL) 0";
	else
		out << setw(kIntWidth)    << fnode + 1;
	out << setw(kIntWidth) << fdof + 1;
	out << setw(kIntWidth) << fcode;
	out << setw(d_width)   << fvalue << '\n';
}

IC_CardT::CodeT IC_CardT::int_to_CodeT (int i_code)
{
	/* resolve code */
	switch (i_code)
	{
		case IC_CardT::kDsp:
			return IC_CardT::kDsp;
			break;
		case IC_CardT::kVel:
			return IC_CardT::kVel;
			break;
		case IC_CardT::kAcc:
			return IC_CardT::kAcc;
			break;
		default:
			cout << "\n IC_CardT::int_to_CodeT: unknown code: "
			<< i_code << endl;
			throw eBadInputValue;	
	}
	/* dummy */
	return IC_CardT::kDsp;
}

/* input operator for codes */
istream& operator>>(istream& in, IC_CardT::CodeT& code)
{
	int i_code;
	in >> i_code;

	/* resolve code */
	switch (i_code)
	{
		case IC_CardT::kDsp:
			code = IC_CardT::kDsp;
			break;
		case IC_CardT::kVel:
			code = IC_CardT::kVel;
			break;
		case IC_CardT::kAcc:
			code = IC_CardT::kAcc;
			break;
		default:
			cout << "\n operator>>IC_CardT::CodeT: unknown code: "
			<< i_code<< endl;
			throw eBadInputValue;	
	}
	return in;
}
