#ifndef _BCC_T_H_
#define _BCC_T_H_

#include "StringT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "CrystalLatticeT.h"

using namespace Tahoe;

class BCCT : public CrystalLatticeT 
{
public:
	BCCT(int nlsd,int nuca,double alat,
	     dArray2DT mat_rot,double angle);

	~BCCT() { };

	BCCT(const BCCT& source);

        const dArrayT& GetLatticeParameters();
        const dArray2DT& GetBasis();
	const dArray2DT& GetAxis();

};

#endif
