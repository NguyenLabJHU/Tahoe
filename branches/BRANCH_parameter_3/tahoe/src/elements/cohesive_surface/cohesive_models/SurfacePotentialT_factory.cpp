/* $Id: SurfacePotentialT_factory.cpp,v 1.1.4.1 2004-04-08 07:32:26 paklein Exp $ */
#include "SurfacePotentialT.h"

#include "XuNeedleman2DT.h"
#include "TvergHutch2DT.h"
#include "ViscTvergHutch2DT.h"

#include <string.h>

using namespace Tahoe;

/* factory method */
SurfacePotentialT* SurfacePotentialT::New(const char* name)
{
	if (strcmp(name, "Xu-Needleman_2D") == 0)
		return new XuNeedleman2DT;
	else if (strcmp(name, "Tvergaard-Hutchinson_2D") == 0)
		return new TvergHutch2DT;
	else if (strcmp(name, "viscous_Tvergaard-Hutchinson_2D") == 0)
		return new ViscTvergHutch2DT;
	else
		return NULL;
}