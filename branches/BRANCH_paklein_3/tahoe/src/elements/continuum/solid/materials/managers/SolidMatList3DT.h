/* $Id: SolidMatList3DT.h,v 1.8 2003-01-29 07:34:58 paklein Exp $ */
/* created: paklein (02/14/1997) */
#ifndef _MATLIST_3D_T_H_
#define _MATLIST_3D_T_H_

/* base class */
#include "SolidMatListT.h"
#include "SolidT.h"

namespace Tahoe {

/** materials list for 3D structural analysis */
class SolidMatList3DT: public SolidMatListT, public SolidT
{
public:

	/** constructors */
	SolidMatList3DT(int length, const SolidMatSupportT& support);

	/** read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

private:
	
	/** \name errror messages */
	/*@{*/
	void Error_no_small_strain(ostream& out, int matcode) const;
	void Error_no_finite_strain(ostream& out, int matcode) const;
	/*@}*/
};

} // namespace Tahoe 
#endif /* _MATLIST_3D_T_H_ */