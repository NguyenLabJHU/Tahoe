/* $Id: MaterialSupportT.h,v 1.10.16.1 2004-06-14 04:56:27 paklein Exp $ */
#ifndef _MATERIAL_SUPPORT_T_H_
#define _MATERIAL_SUPPORT_T_H_

/* base class */
#include "BasicSupportT.h"

/* direct members */
#include "LocalArrayT.h"
#include "AutoArrayT.h"
#include "ElementCardT.h"

namespace Tahoe {

/* forward declarations */
class ContinuumElementT;
class ElementCardT;

/** support for the Tahoe materials classes. */
class MaterialSupportT: public BasicSupportT
{
public:

	/** constructor */
	MaterialSupportT(int ndof, int nip);

	/** destructor */
	virtual ~MaterialSupportT(void);

	/** \name dimensions */
	/*@{*/
	/** number of spatial dimensions */
//	int NumSD(void) const { return fNumSD; };
	
	/** number of degrees of freedom (per node) */
	int NumDOF(void) const { return fNumDOF; };

	/** stress evaluation points per element */
	int NumIP(void) const { return fNumIP; };
	/*@}*/

	/** the low-level communicator only including processes with non-zero numbers
	 * of elements, or NULL if it doesn't exist */
	const CommunicatorT* GroupCommunicator(void) const { return fGroupCommunicator; };

	/** \name run time status */
	/*@{*/

	/** current stress evaluation point within the element. If
	 * no source for the current point is set using 
	 * MaterialSupportT::SetCurrIP, will return 0. */
	int CurrIP(void) const;

	/** set the source for the current stress evaluation point */
	void SetCurrIP(const int& curr_ip);
	/*@}*/
	
	/** \name host code information */
	/*@{*/
	/** return a pointer to the host element. Returns NULL if no
	 * no element information in available. The ContinuumElementT
	 * pointer is set using MaterialSupportT::SetContinuumElement. */
	const ContinuumElementT* ContinuumElement(void) const;
	
	/** set the source for element cards */
	void SetElementCards(AutoArrayT<ElementCardT>* element_cards);

	/** return the number of elements. If the element cards pointer
	 * is not set with MaterialSupportT::SetElementCards, this will return 0 */
	int NumElements(void) const;

	/** return the current element.  If the element cards pointer
	 * is not set with MaterialSupportT::SetElementCards, this will return -1 */
	int CurrElementNumber(void) const;

	/** return the specified card.  If the element cards pointer
	 * is not set with MaterialSupportT::SetElementCards, this will return NULL */
	//ElementCardT* ElementCard(int card);
	ElementCardT* ElementCard(int card) const;

	/** return the current card.  If the element cards pointer
	 * is not set with MaterialSupportT::SetElementCards, this will return NULL */
	ElementCardT* CurrentElement(void) const;

	/** return a pointer the specified local array, or NULL if the array is not
	 * available. During calls the materials routines these will contain the
	 * values for the current element. */
	virtual const LocalArrayT* LocalArray(LocalArrayT::TypeT t) const;

	/** interpolate the given field to the current integration point. Returns true if the
	 * field is available, false otherwise. */
	bool Interpolate(const LocalArrayT& u, dArrayT& u_ip) const;

	/** interpolate the given field to the given integration point. Returns true if the
	 * field is available, false otherwise. */
	bool Interpolate(const LocalArrayT& u, dArrayT& u_ip, int ip) const;
	/*@}*/

	/** \name set host code information */
	/*@{*/
	/** set the element group pointer */
	virtual void SetContinuumElement(const ContinuumElementT* p);

	/** set pointer local array */
	virtual void SetLocalArray(const LocalArrayT& array);
	/*@}*/

  private:
  
  	/** \name dimensions */
  	/*@{*/
	/** number of spatial dimensions */
//	int fNumSD;

	/** number of degrees of freedom */
	int fNumDOF;
	
	/** number of integration points */
	int fNumIP;
  	/*@}*/
  	
  	/** source for the current integration point */
	const int* fCurrIP;

	/** communicator including only processes with non-zero numbers of elements */
	const CommunicatorT* fGroupCommunicator;

	/** pointer to element card information */
	AutoArrayT<ElementCardT>* fElementCards;	
  
  	/** pointer to the continuum element */
  	const ContinuumElementT* fContinuumElement;

	/** \name pointers to local arrays */
	/*@{*/
	const LocalArrayT* fInitCoords;
	const LocalArrayT* fDisp;
	/*@}*/
};

/* inlines functions */
inline const ContinuumElementT* MaterialSupportT::ContinuumElement(void) const
{
	return fContinuumElement;
}

/* set the source for element cards */
inline void MaterialSupportT::SetElementCards(AutoArrayT<ElementCardT>* element_cards)
{
	fElementCards = element_cards;
}

/* return the number of elements */
inline int MaterialSupportT::NumElements(void) const
{
	if (fElementCards) 
		return fElementCards->Length();
	else
		return 0;
}

/* return the current element */
inline int MaterialSupportT::CurrElementNumber(void) const
{
	if (fElementCards) 
		return fElementCards->Position();
	else
		return -1;
}

/* return the specified card */
inline ElementCardT* MaterialSupportT::ElementCard(int card) const
{
	if (fElementCards) 
		return fElementCards->Pointer(card);
	else
		return NULL;
}

/* return the current */
inline ElementCardT* MaterialSupportT::CurrentElement(void) const
{
	if (fElementCards) 
		return &(fElementCards->Current());
	else
		return NULL;
}

inline int MaterialSupportT::CurrIP(void) const {
	if (fCurrIP) return *fCurrIP;
	else return 0;
}

inline void MaterialSupportT::SetCurrIP(const int& curr_ip) { fCurrIP = &curr_ip; }

} /* namespace Tahoe */

#endif /* _MATERIAL_SUPPORT_T_H_ */
