#ifndef _FCC_T_H_
#define _FCC_T_H_

#include "ifstreamT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "CrystalLatticeT.h"

using namespace Tahoe;

class FCCT : public CrystalLatticeT 
{
public:
	FCCT(int nlsd,int nuca,double alat,
	     int which_rot,dArray2DT mat_rot,
	     double angle);

	~FCCT() { };

	FCCT(const FCCT& source);

        const dArrayT& GetLatticeParameters();
        const dArray2DT& GetBasis();
	const dArray2DT& GetAxis();
};

#endif
