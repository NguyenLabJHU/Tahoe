#pragma once
#ifndef GLOBFUNCS_H
#define GLOBFUNCS_H

#include "Vec.h"
#include "Matrix.h"
#include "realtypes.h"
#include <iostream>

namespace periDynamics{

	void gauss3D(const int nip, dem::Matrix& gp_loc3D, dem::Matrix& gp_weigth3D);
	
	void gauss1D(const int nintElem, dem::Matrix& gp_loc1D, dem::Matrix& gp_weight1D );
	
	void invert(const dem::Matrix& a, dem::Matrix& a_inv, REAL& determinant);

	void shp3d(const REAL, const REAL, const REAL, dem::Matrix&, dem::Matrix&, REAL&);
	
	REAL windowfunction(REAL length);

	dem::Matrix dyadicProduct(const dem::Vec&, const dem::Vec&);

} // end periDynamics

#endif
