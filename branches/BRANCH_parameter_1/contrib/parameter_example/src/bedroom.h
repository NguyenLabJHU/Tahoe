/* $Id: bedroom.h,v 1.1.2.2 2003-05-03 09:08:27 paklein Exp $ */
#ifndef _BEDROOM_H_
#define _BEDROOM_H_

/* base class */
#include "room.h"

/* forward declarations */
class window;

using namespace Tahoe;

class bedroom: public room
{
public:

	/** constructor */
	bedroom(void);

	/** destructor */
	~bedroom(void);

	/** \name implementation of ParameterInterfaceT */
	/*@{*/
	virtual void DefineParameters(ParameterListT& list) const;
	virtual void SetParameters(const ParameterListT& list);

	virtual void SubNames(ArrayT<StringT>& names, ArrayT<ParameterListT::OccurrenceT>& occur, ArrayT<bool>& is_inline) const;
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;
	/*@}*/

private:

	int floor;
	
	/** windows */
	ArrayT<window*> windows_;
};

#endif /* _BEDROOM_H_ */
