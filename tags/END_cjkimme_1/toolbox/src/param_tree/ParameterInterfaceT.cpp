/* $Id: ParameterInterfaceT.cpp,v 1.5.2.1 2003-09-25 17:29:35 cjkimme Exp $ */
#include "ParameterInterfaceT.h"
#include "ParameterListT.h"

using namespace Tahoe;

/* array behavior */
namespace Tahoe {
const bool ArrayT<ParameterInterfaceT*>::fByteCopy = true;
const bool ArrayT<const ParameterInterfaceT*>::fByteCopy = true;
const bool ArrayT<SubListDescriptionT>::fByteCopy = false;
}

/* constructor */
ParameterInterfaceT::ParameterInterfaceT(const StringT& name)
{
	SetName(name);
}

void ParameterInterfaceT::SetName(const StringT& name)
{
	fName = name;
}

/* accept completed parameter list */
void ParameterInterfaceT::TakeParameterList(const ParameterListT& list) 
{
	const char caller[] = "ParameterInterfaceT::TakeParameterList";
	if (list.Name() != Name())
		ExceptionT::GeneralFail(caller, "list name \"%s\" must be \"%s\"",
			list.Name().Pointer(), Name().Pointer());
}

/* build complete parameter list description */
void ParameterInterfaceT::DefineParameters(ParameterListT& list) const
{
	const char caller[] = "ParameterInterfaceT::DefineParameters";
	if (list.Name() != Name())
		ExceptionT::GeneralFail(caller, "list name \"%s\" must be \"%s\"",
			list.Name().Pointer(), Name().Pointer());
}

/* validate the given parameter list */
void ParameterInterfaceT::ValidateParameterList(const ParameterListT& raw_list, ParameterListT& valid_list) const
{
	/* check name */
	const char caller[] = "ParameterInterfaceT::ValidateParameterList";
	if (raw_list.Name() != Name())
		ExceptionT::GeneralFail(caller, "raw list name \"%s\" must be \"%s\"",
			raw_list.Name().Pointer(), Name().Pointer());
	if (valid_list.Name() != Name())
		ExceptionT::GeneralFail(caller, "list name \"%s\" must be \"%s\"",
			raw_list.Name().Pointer(), Name().Pointer());

	/* collect description */
	ParameterListT description(Name());
	DefineParameters(description);

	/* parameter lists */
	const ArrayT<ParameterT>& raw_parameters = raw_list.Parameters();
	const ArrayT<ParameterT>& parameters = description.Parameters();
	const ArrayT<ParameterListT::OccurrenceT>& occurrences = description.ParameterOccurrences();

	/* look through parameters */
	for (int i = 0; i < parameters.Length(); i++)
	{
		const ParameterT& parameter = parameters[i];
		ParameterListT::OccurrenceT occurrence = occurrences[i];

		/* search entire list - parameters are not ordered */
		int count = 0;
		for (int j = 0; j < raw_parameters.Length(); j++) {

			ParameterT& raw_parameter = raw_parameters[j];

			/* name match */
			if (raw_parameter.Name() == parameter.Name()) {
			
				/* check occurrence */
				if (count > 0 && (occurrence == ParameterListT::Once || occurrence == ParameterListT::ZeroOrOnce))
					ExceptionT::BadInputValue(caller, "parameter \"%s\" cannot appear in \"%s\" more than once",
						parameter.Name().Pointer(), Name().Pointer());
									
				/* get value */
				ParameterT new_parameter(parameter.Type(), parameter.Name());
				new_parameter.SetDescription(parameter.Description());
				if (raw_parameter.Type() == parameter.Type())
					new_parameter = raw_parameter;
				else if (raw_parameter.Type() == ParameterT::String)
				{
					/* convert to string */
					const StringT& value_str = raw_parameter;

					/* set from string */
					new_parameter.FromString(value_str);	
				}
				else
					ExceptionT::BadInputValue(caller, "source for \"%s\" must have type %d or %d: %d", 
						parameter.Name().Pointer(), ParameterT::String, parameter.Type());

				/* increment count */
				count++;
				
				/* check limits */
				if (parameter.InBounds(new_parameter))
				{
					/* fix string-value pairs */
					if (parameter.Type() == ValueT::Enumeration)
						parameter.FixEnumeration(new_parameter);
				
					/* add it */
					valid_list.AddParameter(new_parameter);
				}
				else 
				{
					/* check again, now verbose */
					parameter.InBounds(new_parameter, true);
					ExceptionT::BadInputValue(caller, "improper value for parameter \"%s\" in \"%s\"",
						parameter.Name().Pointer(), Name().Pointer());
				}
			}
		}
				
		/* look for default value */
		if (count == 0 && (occurrence == ParameterListT::Once || occurrence == ParameterListT::OnePlus)) 
		{
			const ValueT* default_value = parameter.Default();
			if (default_value) {
				ParameterT new_parameter(parameter.Type(), parameter.Name());
				new_parameter.SetDescription(parameter.Description());
				new_parameter = *default_value;

				/* check limits */
				if (parameter.InBounds(new_parameter))
					/* add it */
					valid_list.AddParameter(new_parameter);
				else 
				{	
					/* check again, now verbose */
					parameter.InBounds(new_parameter, true);
					ExceptionT::BadInputValue(caller, "improper value for parameter \"%s\" in \"%s\"",
						parameter.Name().Pointer(), Name().Pointer());
				}
			}
			else
				ExceptionT::BadInputValue(caller, 
					"required parameter \"%s\" is missing from \"%s\" and has no default",
					parameter.Name().Pointer(), Name().Pointer());
		}
	}
}

/* the order of subordinate lists */
ParameterListT::ListOrderT ParameterInterfaceT::ListOrder(void) const
{
	return ParameterListT::Sequence;
}

/* return the list of subordinate names */
void ParameterInterfaceT::DefineSubs(SubListT& sub_list) const
{
	sub_list.Dimension(0);
}

void ParameterInterfaceT::DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order,
	SubListT& sub_sub_list) const
{
#ifdef __MWERKS__
#pragma unused(sub)
#endif
	order = ParameterListT::Sequence;
	sub_sub_list.Dimension(0);
}

/* return a pointer to the ParameterInterfaceT */
ParameterInterfaceT* ParameterInterfaceT::NewSub(const StringT& list_name) const
{
#ifdef __MWERKS__
#pragma unused(list_name)
#endif
	return NULL;
}