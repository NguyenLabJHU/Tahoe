/* $Id: ValueT.h,v 1.5 2003-04-22 18:32:16 paklein Exp $ */
#ifndef _VALUE_T_H_
#define _VALUE_T_H_

/* direct members */
#include "StringT.h"

namespace Tahoe {

/** basic parameter value */
class ValueT
{
public:

	/** enumerator for value type */
	enum TypeT {
		None,
		Integer,
		Double,
		String,
		Enumeration /**< string-integer pair */
	};

	/** \name constructors */
	/*@{*/
	ValueT(int a);
	ValueT(double x);
	ValueT(const StringT& s);

	/** enumeration. Enumerations are string-integer pairs. For all operators
	 * below, enumerations cast to both integers and strings. */
	ValueT(const StringT& name, int value);

	/** set type without assigning value */
	ValueT(TypeT t);

	/** copy constructor */
	ValueT(const ValueT& source);
	
	/** default constructor */
	ValueT(void);
	/*@}*/

	/** write the value to the output stream */
	void Write(ostream& out) const;

	/** stream insertion operator */
	friend ostream& operator<<(ostream& out, const ValueT& value);

	/** value type */
	TypeT Type(void) const { return fType; };

	/** \name set values with assignment operators 
	 * Only type conversion from int to double is allowed. All other
	 * type mismatched will through an exception. */
	/*@{*/
	ValueT& operator=(int a);
	ValueT& operator=(double x);
	ValueT& operator=(const StringT& s);
	ValueT& operator=(const ValueT& rhs);

	/** extract value from string, performing required type conversion */
	void FromString(const StringT& source);
	/*@}*/
	
	/** \name type conversion operators not lvalues */
	/*@{*/
	operator const int&() const;
	operator const double&() const;
	operator const StringT&() const;
	/*@}*/

protected:

	/** value type */
	TypeT fType;

	/** \name stored values */
	/*@{*/
	int fInteger;
	double fDouble;
	StringT fString;
	/*@}*/
};

} // namespace Tahoe 
#endif /* _VALUE_T_H_ */