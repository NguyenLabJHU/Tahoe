/* $Id: ParameterTreeT.cpp,v 1.4 2003-08-18 03:33:37 paklein Exp $ */
#include "ParameterTreeT.h"
#include "ParameterInterfaceT.h"

using namespace Tahoe;

ParameterTreeT::ParameterTreeT(void) {}

ParameterTreeT::~ParameterTreeT(void)
{
	/* free parameter list data */
	for (int i = 0; i < fBranches.Length(); i++)
		delete fBranches[i];

	/* free instantiations */
	for (int i = 0; i < fDeleteMe.Length(); i++)
		delete fDeleteMe[i];
}

/* add a branch to the tree with the given root */
void ParameterTreeT::BuildDescription(const ParameterInterfaceT& root)
{
	/* branch exists */
	if (fDictionary.HasKey(root.Name())) return;

	/* collect parameters into a new list */
	ParameterListT* root_list = new ParameterListT(root.Name());
	BuildBranch(root, *root_list);
	
	/* store branch */
	fBranches.Append(root_list);
}

/* create a validated parameter list */
void ParameterTreeT::Validate(const ParameterInterfaceT& source, const ParameterListT& raw_list, 
	ParameterListT& valid_list)
{
	const char caller[] = "ParameterTreeT::Validate";
	const char* stage_name[] = {"parameters", "sub-lists"};
	int stage = 0;
	
	try { /* catch all exceptions */

	/* clear destination */
	valid_list.Clear();

	/* contents of subordinates */
	SubListT sub_list;

	/* validating self */
	if (source.Name() == raw_list.Name())
	{
		stage = 0;

		/* collect parameters */
		valid_list.SetName(raw_list.Name());
		source.ValidateParameterList(raw_list, valid_list);
		stage = 1;
			
		/* add to the dictionary */
		fDictionary.Insert(source.Name(), &source);

		/* collect information on subordinate lists */
		source.DefineSubs(sub_list);

		/* list order */
		ParameterListT::ListOrderT order = source.ListOrder();
		if (order == ParameterListT::Sequence)
			ValidateSequence(source, raw_list.Lists(), valid_list, sub_list, true);
		else if (order == ParameterListT::Choice)
			ValidateChoice(source, raw_list.Lists(), valid_list, sub_list, true);
		else
			ExceptionT::GeneralFail(caller, "unrecognized list order %d", order);
	}
	else /* defining inlined list */
	{

		//TEMP
		ExceptionT::GeneralFail(caller, "why is there an inlined list here?????????");

#if 0
		/* list must be inlined */
		if (!raw_list.Inline())
			ExceptionT::GeneralFail(caller, "source \"%s\" cannot build list for non-inlined \"%s\"",
				source.Name().Pointer(), raw_list.Name().Pointer());
		
		/* collect information on inlined list */
		ParameterListT::ListOrderT order;
		source.DefineInlineSub(raw_list.Name(), order, sub, sub_occur, sub_is_inline);
#endif
	}
	
	} /* end try */

	/* rethrow */
	catch (ExceptionT::CodeT error) {
		ExceptionT::BadInputValue(caller, "exception %d validating %s in \"%s\"", 
			error, stage_name[stage], source.Name().Pointer());
	}
}

/* return the parameter list for the given branch or NULL if it does not exist */
const ParameterListT* ParameterTreeT::Branch(const StringT& name) const
{
	for (int i = 0; i < fBranches.Length(); i++)
		if (fBranches[i]->Name() == name)
			return fBranches[i];
		
	return NULL;
}

/*************************************************************************
 * Private
 *************************************************************************/

bool ParameterTreeT::ValidateChoice(
	const ParameterInterfaceT& source, 
	const ArrayT<ParameterListT>& raw_sub_list, 
	ParameterListT& valid_list,
	const AutoArrayT<SubListDescriptionT>& sub_lists,
#if 0
	const AutoArrayT<StringT>& sub_name,
	const AutoArrayT<ParameterListT::OccurrenceT>& sub_occur,
	const AutoArrayT<bool>& sub_is_inline,
#endif
	bool throw_on_error)
{
	const char caller[] = "ParameterTreeT::ValidateChoice";

	/* can't match an empty list */
	if (raw_sub_list.Length() == 0 && sub_lists.Length() > 0)
	{
		if (throw_on_error)
			ExceptionT::BadInputValue(caller, "empty source unable to match choice at position %d in \"%s\"",
				valid_list.NumLists()+1, valid_list.Name().Pointer());
		return false;
	}

	/* TEMP - just try to match first item */
	const ParameterListT& raw_sub_0 = raw_sub_list[0];
	const StringT& name_0 = raw_sub_0.Name();

	/* search possible sub names */
	for (int i = 0; i < sub_lists.Length(); i++)
	{
		/* sub list info */
		const SubListDescriptionT& sub_list = sub_lists[i];
	
		//TEMP
		if (sub_list.IsInline())
			ExceptionT::GeneralFail(caller, "inlined list \"%s\" not supported as choice in \"%s\"",
				sub_list.Name().Pointer(), valid_list.Name().Pointer());
	
		/* check */
		if (sub_list.Occurrence() != ParameterListT::Once)
			ExceptionT::GeneralFail(caller, "occurrence of items in choice must be Once");
	
		/* found match */
		if (sub_list.Name() == name_0)
		{
			/* get parameter source */
			const ParameterInterfaceT* sub = NULL;
			if (fDictionary.HasKey(name_0))
				sub = fDictionary[name_0];
			else {
				sub = source.NewSub(name_0);
				fDeleteMe.Append(sub);
			}
		
			/* create validated sub-list */
			ParameterListT valid_sub_list;
			Validate(*sub, raw_sub_0, valid_sub_list);
			valid_list.AddList(valid_sub_list);

			return true;
		}
	}
	
	/* fail on passing through */
	if (throw_on_error)
		ExceptionT::BadInputValue(caller, "\"%s\" does not match choice at position %d in \"%s\"",
			name_0.Pointer(), valid_list.NumLists()+1, valid_list.Name().Pointer());
	return false;
}

bool ParameterTreeT::ValidateSequence(
	const ParameterInterfaceT& source, 
	const ArrayT<ParameterListT>& raw_sub_list, 
	ParameterListT& valid_list,
	const AutoArrayT<SubListDescriptionT>& sub_lists,
#if 0
	const AutoArrayT<StringT>& sub_name,
	const AutoArrayT<ParameterListT::OccurrenceT>& sub_occur,
	const AutoArrayT<bool>& sub_is_inline,
#endif
	bool throw_on_error)
{
	const char caller[] = "ParameterTreeT::ValidateSequence";

	bool check_OK = true;
	int raw_dex = 0;
	int sub_dex = 0;
	for (sub_dex = 0; check_OK && sub_dex < sub_lists.Length() && raw_dex < raw_sub_list.Length(); sub_dex++)
	{
		/* next list entry */
		bool is_inline = sub_lists[sub_dex].IsInline();
		const StringT& name = sub_lists[sub_dex].Name();
		ParameterListT::OccurrenceT occur = sub_lists[sub_dex].Occurrence();
		
		/* inlined sub list */
		if (is_inline)
		{
			/* get sub-list description */
			ParameterListT::ListOrderT in_order;
			SubListT in_sub;
			source.DefineInlineSub(name, in_order, in_sub);

			switch (occur)
			{
				case ParameterListT::Once:
				case ParameterListT::ZeroOrOnce:
				{
					/* alias to tail of the parameter list */
					const ArrayT<ParameterListT> raw_sub_list_tail(raw_sub_list.Length() - raw_dex, raw_sub_list.Pointer(raw_dex));
					
					/* error handling */
					bool in_throw_on_error = (occur == ParameterListT::Once) ? throw_on_error : false;
				
					/* must have one */
					int last_list_count = valid_list.NumLists();
					if (in_order == ParameterListT::Sequence)
						check_OK = ValidateSequence(source, raw_sub_list_tail, valid_list, in_sub, in_throw_on_error);
					else if (in_order == ParameterListT::Choice)
						check_OK = ValidateChoice(source, raw_sub_list_tail, valid_list, in_sub, in_throw_on_error);
					else
						ExceptionT::GeneralFail(caller, "unrecognized list order %d", in_order);

					/* assumes the number of values added to valid list is the number removed from the raw list */
					raw_dex += valid_list.NumLists() - last_list_count; 
				
					/* check */
					if (!check_OK && in_throw_on_error)
						ExceptionT::BadInputValue(caller, "error with inlined item \"%s\" at position %d in \"%s\"", 
							name.Pointer(), valid_list.NumLists() + 1, source.Name().Pointer());
					break;
				}
				case ParameterListT::OnePlus:
				case ParameterListT::Any:
				{
					/* accept many */
					int count = 0;
					while (raw_dex < raw_sub_list.Length() && check_OK)
					{
						/* alias to tail of the parameter list */
						const ArrayT<ParameterListT> raw_sub_list_tail(raw_sub_list.Length() - raw_dex, raw_sub_list.Pointer(raw_dex));

						/* error handling */
						bool in_throw_on_error = (count == 0 && occur == ParameterListT::OnePlus) ? throw_on_error : false;
					
						/* search */
						int last_list_count = valid_list.NumLists();
						if (in_order == ParameterListT::Sequence)
							check_OK = ValidateSequence(source, raw_sub_list_tail, valid_list, in_sub, in_throw_on_error);
						else if (in_order == ParameterListT::Choice)
							check_OK = ValidateChoice(source, raw_sub_list_tail, valid_list, in_sub, in_throw_on_error);
						else
							ExceptionT::GeneralFail(caller, "unrecognized list order %d", in_order);

						if (check_OK) count++;

						/* assumes the number of values added to valid list is the number removed from the raw list */
						raw_dex += valid_list.NumLists() - last_list_count; 
					}
						
					/* must have at least one full pass */
					if (occur == ParameterListT::OnePlus && count < 1)
					{
						check_OK = false;
						if (throw_on_error)
							ExceptionT::BadInputValue(caller, "error with inlined item \"%s\" at position %d in \"%s\"", 
								name.Pointer(), valid_list.NumLists() + 1, source.Name().Pointer());
					}
					else /* no problem */
						check_OK = true;
					break;
				}
				default:
					ExceptionT::GeneralFail(caller, "unrecognized occurrence %d", occur);
			}		
		}
		else /* process non-inlined entries */
		{
			switch (occur)
			{
				case ParameterListT::Once:
				case ParameterListT::ZeroOrOnce:
				{
					/* look for one */
					int count = 0;
					if (raw_sub_list[raw_dex].Name() == name) 
					{
						count++;
					
						/* get parameter source */
						const ParameterInterfaceT* sub = NULL;
						if (fDictionary.HasKey(name))
							sub = fDictionary[name];
						else {
							sub = source.NewSub(name);
							fDeleteMe.Append(sub);
						}

						/* create validated sub-list */
						ParameterListT valid_sub_list;
						Validate(*sub, raw_sub_list[raw_dex], valid_sub_list);
						valid_list.AddList(valid_sub_list);
							
						/* next source item */
						raw_dex++;
					}

					/* check */
					if (occur == ParameterListT::Once && count != 1)
					{
						check_OK = false;						
						if (throw_on_error)
							ExceptionT::BadInputValue(caller, "item %d in \"%s\" must be \"%s\"",
								valid_list.NumLists() + 1, valid_list.Name().Pointer(), name.Pointer());
					}	
					break;
				}
				case ParameterListT::OnePlus:
				case ParameterListT::Any:
				{
					/* accept many */
					int count = 0;
					while (raw_dex < raw_sub_list.Length() && raw_sub_list[raw_dex].Name() == name)
					{
						count++;

						/* get parameter source */
						const ParameterInterfaceT* sub = NULL;
						if (fDictionary.HasKey(name))
							sub = fDictionary[name];
						else {
							sub = source.NewSub(name);
							fDeleteMe.Append(sub);
						}

						/* create validated sub-list */
						ParameterListT valid_sub_list;
						Validate(*sub, raw_sub_list[raw_dex], valid_sub_list);
						valid_list.AddList(valid_sub_list);
				
						/* next source item */
						raw_dex++;
					}
						
					/* must have at least one */
					if (occur == ParameterListT::OnePlus && count < 1)
					{
						check_OK = false;
						if (throw_on_error)
							ExceptionT::BadInputValue(caller, "list \"%s\" must contain at least one \"%s\" beginning at position %d",
								valid_list.Name().Pointer(), name.Pointer(), valid_list.NumLists() + 1);
					}
					break;
				}
				default:
					ExceptionT::GeneralFail(caller, "unrecognized occurrence %d", occur);
			}
		}
	}
			
	/* check any remaining */
	for (;check_OK && sub_dex < sub_lists.Length(); sub_dex++)
		if (sub_lists[sub_dex].Occurrence() != ParameterListT::ZeroOrOnce && 
			sub_lists[sub_dex].Occurrence() != ParameterListT::Any)
		{
			check_OK = false;
			if (throw_on_error)
				ExceptionT::GeneralFail(caller, "\"%s\" required at position %d in \"%s\"",
					sub_lists[sub_dex].Name().Pointer(), valid_list.NumLists()+1, valid_list.Name().Pointer());
		}

	return check_OK;
}

/* build the branch */
void ParameterTreeT::BuildBranch(const ParameterInterfaceT& source, ParameterListT& params)
{
	const char caller[] = "ParameterTreeT::BuildBranch";
	
	/* contents of subordinates */
	SubListT sub_list;

	/* defining self */
	if (source.Name() == params.Name()) 
	{	
		/* collect parameters */
		source.DefineParameters(params);
			
		/* add list to the dictionary */
		if (!fDictionary.Insert(source.Name(), &source))
			ExceptionT::GeneralFail(caller, "dictionary already contains \"%s\"",
				source.Name().Pointer());

		/* collect information on subordinate lists */
		source.DefineSubs(sub_list);
	}
	else /* defining inlined list */
	{
		/* list must be inlined */
		if (!params.Inline())
			ExceptionT::GeneralFail(caller, "source \"%s\" cannot build list for non-inlined \"%s\"",
				source.Name().Pointer(), params.Name().Pointer());
		
		/* collect information on inlined list */
		ParameterListT::ListOrderT order;
		source.DefineInlineSub(params.Name(), order, sub_list);
		params.SetListOrder(order);
	}

	/* define sublists */
	for (int i = 0; i < sub_list.Length(); i++) {
	
		/* sub list info */
		const SubListDescriptionT& sub = sub_list[i];

		/* list is inline */
		if (sub.IsInline())
		{
			/* source builds inline list */
			ParameterListT inline_params(sub.Name());
			inline_params.SetInline(true);
			BuildBranch(source, inline_params);
			
			/* add list */
			if (!params.AddList(inline_params, sub.Occurrence()))
				ExceptionT::GeneralFail(caller, "could not add inline \"%s\" to list \"%s\"",
					inline_params.Name().Pointer(), params.Name().Pointer());
		}
		/* already in the tree */
		else if (fDictionary.HasKey(sub.Name()))
		{
			/* add a blank list as a placeholder */
			ParameterListT sub_params(sub.Name());
			if (!params.AddList(sub_params, sub.Occurrence()))
				ExceptionT::GeneralFail(caller, "could not add sublist \"%s\" to list \"%s\"",
					sub_params.Name().Pointer(), params.Name().Pointer());
		}
		else /* new tree entry */
		{
			/* sublist */
			ParameterInterfaceT* sub_interface = source.NewSub(sub.Name());
			if (!sub_interface)
				ExceptionT::GeneralFail(caller, "source \"%s\" did not return sublist \"%s\"",
					source.Name().Pointer(), sub.Name().Pointer());
					
			/* build the sublist */
			ParameterListT sub_params(sub_interface->Name());
			BuildBranch(*sub_interface, sub_params);
			
			/* add list */
			if (!params.AddList(sub_params, sub.Occurrence()))
				ExceptionT::GeneralFail(caller, "could not add sublist \"%s\" to list \"%s\"",
					sub_params.Name().Pointer(), params.Name().Pointer());
	
			/* add to list of items to delete */
			fDeleteMe.Append(sub_interface);
		}
	}	
}