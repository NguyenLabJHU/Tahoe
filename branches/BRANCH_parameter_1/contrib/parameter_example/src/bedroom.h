/* $Id: bedroom.h,v 1.1.2.1 2003-04-28 08:17:08 paklein Exp $ */#ifndef _BEDROOM_H_#define _BEDROOM_H_/* base class */#include "room.h"using namespace Tahoe;class bedroom: public room{public:	/** constructor */	bedroom(void);	/** \name implementation of ParameterInterfaceT */	/*@{*/	virtual void DefineParameters(ParameterListT& list) const;	virtual void SetParameters(const ParameterListT& list);	/*@}*/private:	int floor;};#endif /* _BEDROOM_H_ */