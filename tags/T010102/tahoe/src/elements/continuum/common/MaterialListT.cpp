/* $Id: MaterialListT.cpp,v 1.2 2001-10-24 02:01:32 paklein Exp $ */
/* created: paklein (02/16/1997) */

#include "MaterialListT.h"
#include "ContinuumMaterialT.h"

/* constructors */
MaterialListT::MaterialListT(int length):
	pArrayT<ContinuumMaterialT*>(length),
	fHasHistory(false),
	fHasLocalizers(false),
	fHasThermal(false)
{

}

/* use in conjunction with ReadMaterialData */
void MaterialListT::WriteMaterialData(ostream& out) const
{
	out << "\n Material Data:\n";
	out << " Number of materials . . . . . . . . . . . . . . = " << fLength << '\n';

	for (int i = 0; i < fLength; i++)
	{
		out << "\n Material number . . . . . . . . . . . . . . . . = " << i+1 << '\n';
		fArray[i]->Print(out);
	}
}

/* apply pre-conditions at the current time step */
void MaterialListT::InitStep(void)
{
	/* loop over list */
	for (int i = 0; i < fLength; i++)
		fArray[i]->InitStep();
}

/* finalize the current time step */
void MaterialListT::CloseStep(void)
{
	/* loop over list */
	for (int i = 0; i < fLength; i++)
		fArray[i]->CloseStep();
}
