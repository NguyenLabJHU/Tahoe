/* $Id: Material2DT.cpp,v 1.3 2002-10-05 20:04:19 paklein Exp $ */
/* created: paklein (02/15/1997) */
#include "Material2DT.h"

#include <iostream.h>

#include "ExceptionCodes.h"
#include "fstreamT.h"

namespace Tahoe {

/* stream extraction operator */
istream& operator>>(istream& in, Material2DT::ConstraintOptionT& option)
{
	int i_option;
	in >> i_option;

	/* resolve code */
	switch (i_option)
	{
		case Material2DT::kPlaneStress:
			option = Material2DT::kPlaneStress;
			break;
		case Material2DT::kPlaneStrain:
			option = Material2DT::kPlaneStrain;
			break;
		default:
			cout << "\n operator>>Material2DT::ConstraintOptionT: unknown option: "
			<< i_option << endl;
			throw eBadInputValue;	
	}
	return in;
}

} // namespace Tahoe

using namespace Tahoe; 

/* constructor */
Material2DT::Material2DT(ifstreamT& in)
{
	in >> fThickness;
	in >> fConstraintOption;

	/* checks */
	if (fThickness <= 0.0) throw eBadInputValue;
	if (fConstraintOption != kPlaneStress &&
	    fConstraintOption != kPlaneStrain) throw eBadInputValue;
}

Material2DT::Material2DT(ifstreamT& in, ConstraintOptionT constraintopt):
	fConstraintOption(constraintopt)
{
	int junk;	//ignore constraint option
	in >> fThickness;
	in >> junk;	
	
	if (fThickness <= 0.0) throw eBadInputValue;
}

/* default material output */
void Material2DT::Print(ostream& out) const
{
	out << " Thickness . . . . . . . . . . . . . . . . . . . = " << fThickness << '\n';
	out << " 2D constraint option. . . . . . . . . . . . . . = " << fConstraintOption << '\n';
	out << "    eq." << kPlaneStress	<< ", plane stress\n";
	out << "    eq." << kPlaneStrain	<< ", plane strain\n";
}