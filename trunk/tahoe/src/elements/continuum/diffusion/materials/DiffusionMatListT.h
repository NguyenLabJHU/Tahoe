/* $Id: DiffusionMatListT.h,v 1.2 2002-07-02 19:56:05 cjkimme Exp $ */
/* created: paklein (10/02/1999)                                          */

#ifndef _DIFFUSE_MAT_LIST_T_H_
#define _DIFFUSE_MAT_LIST_T_H_

/* base class */
#include "MaterialListT.h"

/* forward declarations */

namespace Tahoe {

class DiffusionT;

class DiffusionMatListT: public MaterialListT
{
public:

	/* constructors */
	DiffusionMatListT(int length, const DiffusionT& element_group);

	/* read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);
	
private:

	/* element group */
	const DiffusionT& fElementGroup;

};

} // namespace Tahoe 
#endif /* _DIFFUSE_MAT_LIST_T_H_ */
