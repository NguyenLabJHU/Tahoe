/*
 * File: iAutoArrayT.h
 *
 */

/*
 * created      : PAK (02/08/99)
 * last modified: PAK (02/08/99)
 */

#ifndef _I_AUTO_ARRAY_T_H_
#define _I_AUTO_ARRAY_T_H_

/* base class */
#include "AutoArrayT.h"

class iAutoArrayT: public AutoArrayT<int>
{
  public:

	/* constructors */
	iAutoArrayT(void);
	iAutoArrayT(int headroom);
	iAutoArrayT(int length, int headroom);
	iAutoArrayT(const ArrayT<int>& source); 
	iAutoArrayT(const ArrayT<int>& source, int headroom); 

	/* assignment operators - dimensions must be correct */
 	AutoArrayT<int>& operator=(const AutoArrayT<int>& RHS);
 	AutoArrayT<int>& operator=(const ArrayT<int>& RHS);
 	AutoArrayT<int>& operator=(int value);

	/* max and min */
	int Max(void) const;
	int Min(void) const;
	void MinMax(int& min, int& max) const;

  	/* flagging operations */
	void ChangeValue(int from, int to);
	int	Count(int value) const;
	
	/* output operator */
	friend ostream& operator<<(ostream& out, const iAutoArrayT& array);
};

/* inlines */

/* constructors */
inline iAutoArrayT::iAutoArrayT(void) { }
inline iAutoArrayT::iAutoArrayT(int headroom):
	AutoArrayT<int>(headroom, true) { }
inline iAutoArrayT::iAutoArrayT(int length, int headroom):
	AutoArrayT<int>(length, headroom, true) { }
inline iAutoArrayT::iAutoArrayT(const ArrayT<int>& source):
	AutoArrayT<int>(source, kAutoDefHeadRoom, true) { }
inline iAutoArrayT::iAutoArrayT(const ArrayT<int>& source, int headroom):
	AutoArrayT<int>(source, headroom, true) { }

/* assigment operators */
inline AutoArrayT<int>& iAutoArrayT::operator=(const AutoArrayT<int>& RHS)
{
	AutoArrayT<int>::operator=(RHS);
	return *this;
}

inline AutoArrayT<int>& iAutoArrayT::operator=(const ArrayT<int>& RHS)
{
	AutoArrayT<int>::operator=(RHS);
	return *this;
}

inline AutoArrayT<int>& iAutoArrayT::operator=(int value)
{
	AutoArrayT<int>::operator=(value);
	return *this;
}
	
#endif /* _I_AUTO_ARRAY_T_H_ */
