/* $Id: SurfacePotentialT_factory.cpp,v 1.1.4.3 2004-06-24 04:56:17 paklein Exp $ */
#include "SurfacePotentialT.h"

#include "XuNeedleman2DT.h"
#include "XuNeedleman3DT.h"
#include "TvergHutch2DT.h"
#include "TvergHutch3DT.h"
#include "ViscTvergHutch2DT.h"
#include "Tijssens2DT.h"
#include "RateDep2DT.h"
#include "YoonAllen2DT.h"
#include "YoonAllen3DT.h"

#include <string.h>

using namespace Tahoe;

/* factory method */
SurfacePotentialT* SurfacePotentialT::New(const char* name)
{
	if (strcmp(name, "Xu-Needleman_2D") == 0)
		return new XuNeedleman2DT;
	else if (strcmp(name, "Xu-Needleman_3D") == 0)
		return new XuNeedleman3DT;
	else if (strcmp(name, "Tvergaard-Hutchinson_2D") == 0)
		return new TvergHutch2DT;
	else if (strcmp(name, "Tvergaard-Hutchinson_3D") == 0)
		return new TvergHutch3DT;
	else if (strcmp(name, "viscous_Tvergaard-Hutchinson_2D") == 0)
		return new ViscTvergHutch2DT;
	else if (strcmp(name, "Tijssens_2D") == 0)
		return new Tijssens2DT;
	else if (strcmp(name, "Tvergaard-Hutchinson_rate_dep_2D") == 0)
		return new RateDep2DT;
	else if (strcmp(name, "Yoon-Allen_2D") == 0)
		return new YoonAllen2DT;
	else if (strcmp(name, "Yoon-Allen_3D") == 0)
		return new YoonAllen3DT;
	else
		return NULL;
}
