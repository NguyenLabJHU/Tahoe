/* $Id: ParameterListT.cpp,v 1.10 2003-11-04 01:21:25 paklein Exp $ */
#include "ParameterListT.h"

using namespace Tahoe;

/* array behavior */
namespace Tahoe {
DEFINE_TEMPLATE_STATIC const bool ArrayT<ParameterListT>::fByteCopy = false;
DEFINE_TEMPLATE_STATIC const bool ArrayT<ParameterListT*>::fByteCopy = true;
DEFINE_TEMPLATE_STATIC const bool ArrayT<ParameterListT::OccurrenceT>::fByteCopy = false;
}

/* constructor */
ParameterListT::ParameterListT(const char* name):
	fName(name),
	fListOrder(Sequence),
	fInline(false),
	fDuplicateListNames(true)
{

}

/* default constructor */
ParameterListT::ParameterListT(void):
	fListOrder(Sequence),
	fInline(false),
	fDuplicateListNames(true)
{

}

/* set/change the list type */
void ParameterListT::SetInline(bool is_inline)
{
	if (is_inline && fParameters.Length() > 0)
		ExceptionT::GeneralFail("ParameterListT::SetInline", 
			"lists with parameters cannot inlined");
	fInline = is_inline;
}

/* set/change the list order */
void ParameterListT::SetListOrder(ListOrderT list_order)
{
	if (list_order == Choice) {
	
		/* all list occurrences must be Once */
		for (int i = 0; i < fParameterListsOccur.Length(); i++)
			if (fParameterListsOccur[i] != Once)
			ExceptionT::GeneralFail("ParameterListT::SetListOrder", 
				"for list order \"Choice\" all lists must occur \"Once\"");
	}

	fListOrder = list_order;
}

/* number of nested parameter lists with the given name */
int ParameterListT::NumLists(const char* name) const
{
	int count = 0;
	for (int i = 0; i < fParameterLists.Length(); i++)
		if (fParameterLists[i].Name() == name)
			count ++;
	return count;
}

/* add parameter */
bool ParameterListT::AddParameter(const ParameterT& param, OccurrenceT occur)
{
	if (fInline)
		ExceptionT::GeneralFail("ParameterListT::AddParameter", 
			"inlined lists cannot have parameters");

	/* "description" is reserved */
	if (param.Name() == "description") {
		cout << "\n ParameterListT::AddParameter: parameter name \"description\" is reserved" << endl;
		return false;
	}

	/* scan name */
	for (int i = 0; i < fParameters.Length(); i++)
		if (fParameters[i].Name() == param.Name())
			return false;
	
	/* add if no matches */
	fParameters.Append(param);
	fParametersOccur.Append(occur);
	return true;
}

/* add a parameter list */
bool ParameterListT::AddList(const ParameterListT& param_list, OccurrenceT occur)
{	
	/* check occurrence */
	if (fListOrder == Choice && occur != Once)
		ExceptionT::GeneralFail("ParameterListT::AddList", 
			"for list order \"Choice\" all lists must occur \"Once\"");

	/* scan name */
	if (!fDuplicateListNames)
		for (int i = 0; i < fParameterLists.Length(); i++)
			if (fParameterLists[i].Name() == param_list.Name())
				return false;

	/* add to list */
	fParameterLists.Append(param_list);
	fParameterListsOccur.Append(occur);
	return true;
}

/* add a reference */
bool ParameterListT::AddReference(const char* ref, OccurrenceT occur)
{
	/* scan name */
	for (int i = 0; i < fReferences.Length(); i++)
		if (fReferences[i] == ref)
			return false;
	
	/* add if no matches */
	fReferences.Append(ref);
	fReferencesOccur.Append(occur);
	return true;
}

/* return the pointer to the given list or NULL if the list is not found */
const ParameterListT* ParameterListT::List(const char* name, int instance) const
{
	/* search list */
	int count = 0;
	for (int i = 0; i < fParameterLists.Length(); i++)
		if (fParameterLists[i].Name() == name)
			if (++count == instance) 
				return fParameterLists.Pointer(i);

	/* fail */
	return NULL;
}

/* return the non-const pointer to the given parameter or NULL if the list is not found */
const ParameterT* ParameterListT::Parameter(const char* name) const
{
	/* search list */
	for (int i = 0; i < fParameters.Length(); i++)
		if (fParameters[i].Name() == name)
			return fParameters.Pointer(i);

	/* fail */
	return NULL;
}

void ParameterListT::GetParameter(const char* name, int& a) const
{
	const char caller[] = "ParameterListT::GetParameter";
	const ParameterT* parameter = Parameter(name);
	if (!parameter)
		ExceptionT::GeneralFail(caller, "parameter \"%s\" not found in \"%s\"", 
			name, Name().Pointer());
	a = *parameter;
}

void ParameterListT::GetParameter(const char* name, double& a) const
{
	const char caller[] = "ParameterListT::GetParameter";
	const ParameterT* parameter = Parameter(name);
	if (!parameter)
		ExceptionT::GeneralFail(caller, "parameter \"%s\" not found in \"%s\"", 
			name, Name().Pointer());
	a = *parameter;
}

void ParameterListT::GetParameter(const char* name, StringT& a) const
{
	const char caller[] = "ParameterListT::GetParameter";
	const ParameterT* parameter = Parameter(name);
	if (!parameter)
		ExceptionT::GeneralFail(caller, "parameter \"%s\" not found in \"%s\"", 
			name, Name().Pointer());
	a = *parameter;
}

void ParameterListT::GetParameter(const char* name, bool& a) const
{
	const char caller[] = "ParameterListT::GetParameter";
	const ParameterT* parameter = Parameter(name);
	if (!parameter)
		ExceptionT::GeneralFail(caller, "parameter \"%s\" not found in \"%s\"", 
			name, Name().Pointer());
	a = *parameter;
}

/* return the given parameter */
const ParameterT& ParameterListT::GetParameter(const char* name) const
{
	const ParameterT* parameter = Parameter(name);
	if (!parameter)
		ExceptionT::GeneralFail("ParameterListT::GetParameter", 
			"parameter \"%s\" not found in \"%s\"", 
			name, Name().Pointer());
	return *parameter;
}

/**********************************************************************
 * Private
 **********************************************************************/

void ParameterListT::Clear(void)
{
	fName.Clear();
	fDescription.Clear();
	fParameters.Dimension(0);
	fParametersOccur.Dimension(0);
	fParameterLists.Dimension(0);
	fParameterListsOccur.Dimension(0);
	fReferences.Dimension(0);
	fReferencesOccur.Dimension(0);
}