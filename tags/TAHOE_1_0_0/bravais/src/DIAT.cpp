// DEVELOPMENT
#include "DIAT.h"
#include "CrystalLatticeT.h"
#include <iostream>
#include <fstream>
#include "StringT.h"
#include "ifstreamT.h"
#include "ExceptionCodes.h"
#include "dArrayT.h"
#include "dArray2DT.h"

DIAT::DIAT(int nlsd,int nuca,dArrayT alat,
	   int which_rot,dArray2DT mat_rot,
	   double angle) : 
  CrystalLatticeT(nlsd,nuca,which_rot,mat_rot,angle)
{
  for (int i=0; i<nlsd; i++) 
    vLatticeParameters[i] = alat[i];

  if (nlsd==2)
    {
      if(nuca != 2) {cout << "Wrong nuca\n"; throw eSizeMismatch;}
      vBasis(0,0) = 0.0;
      vBasis(1,0) = 0.0;

      vBasis(0,1) = 0.25;
      vBasis(1,1) = 0.25;

      // Define basis vectors
      vAxis(0,0) = vLatticeParameters[0];
      vAxis(1,0) = 0.0;
      
      vAxis(0,1) = 0.0;
      vAxis(1,1) = vLatticeParameters[1];

      // Rotate axis if necessary
      if (fabs(angle_rotation) >=1.e-5) 
      	vAxis = AxisRotation(vAxis);
    }
  
  if (nlsd==3) 
    {
      if(nuca != 8) {cout << "Wrong nuca\n"; throw eSizeMismatch;}
      vBasis(0,0) = 0.0;
      vBasis(1,0) = 0.0;
      vBasis(2,0) = 0.0;

      vBasis(0,1) = 0.5;
      vBasis(1,1) = 0.5;
      vBasis(2,1) = 0.0;

      vBasis(0,2) = 0.5;
      vBasis(1,2) = 0.0;
      vBasis(2,2) = 0.5;

      vBasis(0,3) = 0.0;
      vBasis(1,3) = 0.5;
      vBasis(2,3) = 0.5;

      vBasis(0,4) = 0.25;
      vBasis(1,4) = 0.25;
      vBasis(2,4) = 0.25;

      vBasis(0,5) = 0.75;
      vBasis(1,5) = 0.75;
      vBasis(2,5) = 0.25;

      vBasis(0,6) = 0.75;
      vBasis(1,6) = 0.25;
      vBasis(2,6) = 0.75;

      vBasis(0,7) = 0.25;
      vBasis(1,7) = 0.75;
      vBasis(2,7) = 0.75;

      // Define basis vectors
      vAxis(0,0) = vLatticeParameters[0];
      vAxis(1,0) = 0.0;
      vAxis(2,0) = 0.0;

      vAxis(0,1) = 0.0;
      vAxis(1,1) = vLatticeParameters[1];
      vAxis(2,1) = 0.0;

      vAxis(0,2) = 0.0;
      vAxis(1,2) = 0.0;
      vAxis(2,2) = vLatticeParameters[2];

      // Rotate axis if necessary  (put a flag later...)
      double norm = sqrt(norm_vec[0] + norm_vec[1] + norm_vec[2]);
      if (norm > 1.e-5) vAxis = AxisRotation(vAxis);


    }
}


DIAT::DIAT(const DIAT& source) : CrystalLatticeT(source.nLSD,source.nUCA,
						 source.WhichRot,source.matrix_rotation,
						 source.angle_rotation)
{
  for (int i=0; i<source.nLSD; i++) 
    for (int j=0; j<source.nUCA; j++) 
      vBasis(i,j) = source.vBasis(i,j);

  for (int i=0; i<source.nLSD; i++) 
    for (int j=0; j<source.nLSD; j++) 
      vAxis(i,j) = source.vAxis(i,j);
 
  for (int i=0; i<source.nLSD; i++) 
    vLatticeParameters[i] = source.vLatticeParameters[i];

}

const dArray2DT& DIAT::GetBasis() 
{
  if (!&vBasis) 
    {
      cout << "vBasis doesn't exist!" << "\n";
      throw eDatabaseFail;
    }
  else 
    {
      return vBasis;		
    }
}

const dArray2DT& DIAT::GetAxis() 
{
  if (!&vAxis) 
    {
      cout << "vAxis doesn't exist!" << "\n";
      throw eDatabaseFail;
    }
  else 
    {
      return vAxis;		
    }
}

const dArrayT& DIAT::GetLatticeParameters() 
{
  if (!&vLatticeParameters) 
    {
      cout << "vLatticeParameters doesn't exist!" << "\n";	
      throw eDatabaseFail;
    }
  else 
    {
      return vLatticeParameters;
    }
}	

