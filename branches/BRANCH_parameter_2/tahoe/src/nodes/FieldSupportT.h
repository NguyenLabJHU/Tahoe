/* $Id: FieldSupportT.h,v 1.5.16.3 2004-03-22 18:39:39 paklein Exp $ */
#ifndef _FIELD_SUPPORT_T_H_
#define _FIELD_SUPPORT_T_H_

namespace Tahoe {

/* forward declarations */
class FEManagerT;
class ElementMatrixT;
template <class TYPE> class nArrayT;
class dArrayT;
class ifstreamT;
class ofstreamT;
class FBC_ControllerT;
class KBC_ControllerT;
class FieldT;
class NodeManagerT;
class ModelManagerT;
class ScheduleT;

/** support for FieldT. Limited interface to get information out
 * of a FieldT. Wrapper for functions in FEManagerT. */
class FieldSupportT
{
public:

	/** constructor */
	FieldSupportT(const FEManagerT& fe, NodeManagerT& nodes);

	/** \name accessors to higher levels */
	/*@{*/
	/** the top-level */
	const FEManagerT& FEManager(void) const { return fFEManager; };

	/** the nodes */
	const NodeManagerT& NodeManager(void) const { return fNodeManager; };

	/** the model */
	ModelManagerT& ModelManager(void) const;
	/*@}*/

	/** \name assembly functions */
	/*@{*/
	void AssembleLHS(int group, const ElementMatrixT& elMat, const nArrayT<int>& eqnos) const;
	void AssembleRHS(int group, const dArrayT& elRes, const nArrayT<int>& eqnos) const;
	/*@}*/
	
	/** \name streams */
	/*@{*/
	ifstreamT& Input(void) const;
	ofstreamT& Output(void) const;
	/*@}*/

	/** \name construct BC controllers
	 * Construct new kinematic or force boundary condition controllers. Responsibility 
	 * for deleteting instantiations resides with the client who requested them.
	 */
	/*@{*/
	KBC_ControllerT* NewKBC_Controller(FieldT& field, int code) const;
	FBC_ControllerT* NewFBC_Controller(FieldT& field, int code) const;
	/*@}*/

	/** return a pointer to the specified schedule */
	const ScheduleT* Schedule(int num) const;

private:

	/** the top-level manager */
	const FEManagerT& fFEManager;

	/** the node manager */
	NodeManagerT& fNodeManager;
};

/* constructor */
inline FieldSupportT::FieldSupportT(const FEManagerT& fe, NodeManagerT& nodes):
	fFEManager(fe),
	fNodeManager(nodes)
{

}

} // namespace Tahoe 
#endif /* _FIELD_SUPPORT_T_H_ */
