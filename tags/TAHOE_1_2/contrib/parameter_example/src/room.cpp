/* $Id: room.cpp,v 1.2 2003-05-04 22:49:50 paklein Exp $ */
#include "room.h"

room::room(const StringT& name):
	ParameterInterfaceT(name),
	length(0),
	width(0)
{

}

void room::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ParameterInterfaceT::DefineParameters(list);

	LimitT bound(0, LimitT::Lower);

	ParameterT the_length(length, "length");
	the_length.AddLimit(bound);
	list.AddParameter(the_length);

	ParameterT the_width(width, "width");
	the_width.AddLimit(bound);
	list.AddParameter(the_width);
}

void room::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	ParameterInterfaceT::TakeParameterList(list);
	
	list.GetParameter("length", length);
	list.GetParameter("width", width);
}