/* $Id: DiffusionMatListT.h,v 1.8.18.1 2004-04-08 07:32:33 paklein Exp $ */
/* created: paklein (10/02/1999) */
#ifndef _DIFFUSE_MAT_LIST_T_H_
#define _DIFFUSE_MAT_LIST_T_H_

/* base class */
#include "MaterialListT.h"

namespace Tahoe {

/* forward declarations */
class DiffusionMatSupportT;
class DiffusionMaterialT;

/** list of materials for diffusion analysis */
class DiffusionMatListT: public MaterialListT
{
public:

	/** enum defining material types */
	enum TypeT {
        kLinear = 1,
     kNonLinear = 2};

	/** constructors */
	DiffusionMatListT(int length, const DiffusionMatSupportT& support);
	DiffusionMatListT(void);

	/** read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order, 
		SubListT& sub_sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

	/** construct the specified material or NULL if the request cannot be completed */
	DiffusionMaterialT* NewDiffusionMaterial(const StringT& name) const;
	
private:

	/** support for diffusion materials */
	const DiffusionMatSupportT* fDiffusionMatSupport;
};

} // namespace Tahoe 
#endif /* _DIFFUSE_MAT_LIST_T_H_ */
