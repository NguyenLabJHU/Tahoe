/* $Id: ABAQUS_BaseT.h,v 1.1.2.1 2003-11-22 22:23:51 paklein Exp $ */
#ifndef _ABAQUS_BASE_T_H_
#define _ABAQUS_BASE_T_H_

/* library support options */
#ifdef __F2C__

/* f2c */
#include "f2c.h"

namespace Tahoe {

/* forward declarations */
class dMatrixT;
class dSymMatrixT;
class ElementCardT;

/** some ABAQUS basics */
class ABAQUS_BaseT
{
public:

	/** constructor */
	ABAQUS_BaseT(void);

protected:

	/** name stress conversion functions */
	/*@{*/
	void dMatrixT_to_ABAQUS(const dMatrixT& A, nMatrixT<doublereal>& B) const;
	void ABAQUS_to_dSymMatrixT(const doublereal* pA, dSymMatrixT& B) const;
	void dSymMatrixT_to_ABAQUS(const dSymMatrixT& A, doublereal* pB) const;
	/*@}*/

	/** \name read ABAQUS-format input */
	/*@{*/
	void Read_ABAQUS_Input(ifstreamT& in, StringT& name, nArrayT<doublereal>& properties,
		integer& nstatv) const;
	bool Next_ABAQUS_Keyword(ifstreamT& in) const;
	bool Skip_ABAQUS_Symbol(ifstreamT& in, char c) const;
	void Skip_ABAQUS_Comments(ifstreamT& in) const;
	void Read_ABAQUS_Word(ifstreamT& in, StringT& word, bool to_upper = true) const;
	/*@}*/	
};

#else /* __F2C__ */

#ifndef __MWERKS__
#error "ABAQUS_BaseT requires __F2C__"
#endif

#endif /* __F2C__ */

} /* namespace Tahoe */

#endif /* _ABAQUS_BASE_T_H_ */
