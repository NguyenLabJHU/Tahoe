// DEVELOPMENT
/* $Id: VolumeT.h,v 1.13 2003-07-25 18:18:34 jzimmer Exp $ */

#ifndef _VOLUME_T_H_
#define _VOLUME_T_H_

#include <iostream>

#include "iArrayT.h"
#include "iArray2DT.h"

#include "dArrayT.h"
#include "dArray2DT.h"
#include "CrystalLatticeT.h"
#include "ifstreamT.h"

namespace Tahoe {


class VolumeT {

protected:

	int nSD;
	int nATOMS;
	double volume;
	StringT sLATTYPE;

	StringT atom_names;
	iArrayT atom_ID;
	dArray2DT atom_coord;
	iArray2DT atom_connectivities;
	ArrayT< iArray2DT * > atom_part_connect;   
	dArray2DT atom_bounds;
	iArrayT atom_types;
	iArrayT atom_parts;

	iArrayT WhichSort;
	iArrayT Map;

public:

        VolumeT(int n);
	~VolumeT() {};
        VolumeT(const VolumeT& source);

	int GetDimensions();
	int GetNumberAtoms();
	double GetVolume();

	virtual void CreateLattice(CrystalLatticeT* pcl) = 0;
	virtual void SortLattice(CrystalLatticeT* pcl) = 0;

	virtual void CalculateBounds(CrystalLatticeT* pcl) = 0;

	virtual iArrayT GetNCells() = 0;
        virtual dArray2DT GetLength() = 0; 

	StringT*   GetAtomNames();
	iArrayT*   GetAtomID();
	dArray2DT* GetAtomCoordinates();
	iArray2DT* GetAtomConnectivities();
	ArrayT< iArray2DT * > * GetAtomPartsConnectivities();
	dArray2DT* GetAtomBounds();
	iArrayT*   GetAtomTypes();
	iArrayT*   GetAtomParts();
};

}
#endif

