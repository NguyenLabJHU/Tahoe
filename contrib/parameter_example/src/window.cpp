/* $Id: window.cpp,v 1.2 2003-05-04 22:49:50 paklein Exp $ */
#include "window.h"

window::window(void):
  ParameterInterfaceT("window"),
	width_(0.0),
	height_(0.0)
{

}

void window::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ParameterInterfaceT::DefineParameters(list);

	LimitT bound(0, LimitT::Lower);

	ParameterT width(width_, "width");
	width.AddLimit(bound);
	width.SetDefault(2.5);
	list.AddParameter(width);

	ParameterT height(height_, "height");
	height.AddLimit(bound);
	height.SetDefault(4.0);
	list.AddParameter(height);
}

void window::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	ParameterInterfaceT::TakeParameterList(list);

	list.GetParameter("height", height_);
	list.GetParameter("width", width_);
}
