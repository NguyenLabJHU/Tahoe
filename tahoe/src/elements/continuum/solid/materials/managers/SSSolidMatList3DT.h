/* $Id: SSSolidMatList3DT.h,v 1.1.2.1 2004-01-21 19:10:18 paklein Exp $ */
/* created: paklein (02/14/1997) */
#ifndef _MATLIST_3D_T_H_
#define _MATLIST_3D_T_H_

/* base class */
#include "SolidMatListT.h"
#include "SolidT.h"

namespace Tahoe {

/** materials list for 3D structural analysis */
class SSSolidMatList3DT: public SolidMatListT, public SolidT
{
public:

	/** constructors */
	SSSolidMatList3DT(int length, const SSMatSupportT& support);
	SSSolidMatList3DT(void);

	/** read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

private:

	/** support for finite strain materials */
	const SSMatSupportT* fSSMatSupport;
	
};

} // namespace Tahoe 
#endif /* _MATLIST_3D_T_H_ */
