/* $Id: InverseMapT.h,v 1.4 2003-03-02 18:50:53 paklein Exp $ */
#ifndef _INVERSE_MAP_T_H_
#define _INVERSE_MAP_T_H_

/* base class */
#include "AutoArrayT.h"

namespace Tahoe {

/* forward declarations */
template <class TYPE> class nArrayT;

/** class to construct and access an inverse map. Given a forward map
 * \f$ j = g(i) \f$, the class constructs and handles access to the inverse
 * map \f$ i = g^{-1}(j) \f$. By default, InverseMapT::Map of a value not in 
 * the forward map throws an exception. This behavior can be changed by
 * setting the 
 */
class InverseMapT: private AutoArrayT<int>
{
public:

	/** enum for what to do about trying to map values that 
	 * are out of range */
	enum SettingT {
		Throw,   /**< throw ExceptionT::OutOfRange */
		MinusOne /**< return -1 */
	};

	/** constructor */
	InverseMapT(void);

	/** construct the inverse map */
	void SetMap(const nArrayT<int>& forward);
	
	/** set the flag for handling calls to InverseMapT::Map that
	 * are out of range */
	void SetOutOfRange(SettingT setting) { fOutOfRange = setting; };
	
	/** return the out of range flag */
	SettingT OutOfRange(void) const { return fOutOfRange; };
	
	/** clear the map. Sets InverseMapT::Length to zero without
	 * necessarily freeing any memory. Use InverseMapT::Free to
	 * release allocated memory */
	void ClearMap(void) { Dimension(0); };

	/** map the global index to the local index */
	int Map(int global) const;
	
	/** release memory */
	void Free(void);
	
	/** return the logical size of the map */
	int Length(void) { return AutoArrayT<int>::Length(); };
	
private:

	/** minimum value in the forward map. The index shift allows some
	 * saving in memory since global tags less than the shift are not stored
	 * in the map. */	
	int fShift;	
	
	/** how to handle out of range */
	SettingT fOutOfRange;
};

/* inlines */

/* constructor */
inline InverseMapT::InverseMapT(void): 
	fShift(0),
	fOutOfRange(Throw)
{

}

/* map the global index to the local index */
inline int InverseMapT::Map(int global) const
{
	int dex = global - fShift;
	int map = -1;
	if (dex > -1 && dex < fLength) map = (*this)[dex];

	if (map == -1 && fOutOfRange == Throw) 
		ExceptionT::OutOfRange("InverseMapT::Map", "%d was not in the forward map", global);
	return map;
}

/* release memory */
inline void InverseMapT::Free(void) { AutoArrayT<int>::Free(); }

} /* namespace Tahoe */
 
#endif /* _INVERSE_MAP_T_H_ */