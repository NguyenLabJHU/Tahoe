// DEVELOPMENT
#include "CORUNT.h"
#include "CrystalLatticeT.h"
#include <iostream>
#include <fstream>
#include "StringT.h"
#include "ifstreamT.h"
#include "ExceptionCodes.h"
#include "dArrayT.h"
#include "dArray2DT.h"

// Lattice parameter: a = b <> c 


CORUNT::CORUNT(int nlsd,int nuca,dArrayT alat,
	   int which_rot,dArray2DT mat_rot,
	   double angle) : 
  CrystalLatticeT(nlsd,nuca,which_rot,mat_rot,angle)
{
  for (int i=0; i<nlsd; i++) 
    vLatticeParameters[i] = alat[i];

  // Define Lattice Type
  sLATTYPE = "CORUN";

  if (nlsd==2)
    {
      cout << "Cannot create a 2-dimensional CORUN lattice!" << "\n";
      cout << "Try a 2-d HEX lattice instead." << "\n";
      throw eBadInputValue;
    }
  
  if (nlsd==3) 
    {

      if(nuca != 20) {cout << "Wrong nuca\n"; throw eSizeMismatch;}

      // Define atoms in cell

      vBasis(0,0) = 0.0;
      vBasis(1,0) = 0.0;
      vBasis(2,0) = 0.0;

      vBasis(0,1) = 0.5/3.0;
      vBasis(1,1) = 0.5;
      vBasis(2,1) = 0.0;

      vBasis(0,2) = 1.0/3.0;
      vBasis(1,2) = 0.0;
      vBasis(2,2) = 0.0;

      vBasis(0,3) = 1.5/3.0;
      vBasis(1,3) = 0.5;
      vBasis(2,3) = 0.0;

      vBasis(0,4) = 2.0/3.0;
      vBasis(1,4) = 0.0;
      vBasis(2,4) = 0.0;

      vBasis(0,5) = 2.5/3.0;
      vBasis(1,5) = 0.5;
      vBasis(2,5) = 0.0;

      vBasis(0,6) = 0.0;
      vBasis(1,6) = 1.0/3.0;
      vBasis(2,6) = 0.5;

      vBasis(0,7) = 0.5/3.0;
      vBasis(1,7) = 5.0/6.0;
      vBasis(2,7) = 0.5;

      vBasis(0,8) = 1.0/3.0;
      vBasis(1,8) = 1.0/3.0;
      vBasis(2,8) = 0.5;

      vBasis(0,9) = 1.5/3.0;
      vBasis(1,9) = 5.0/6.0;
      vBasis(2,9) = 0.5;

      vBasis(0,10) = 2.0/3.0;
      vBasis(1,10) = 1.0/3.0;
      vBasis(2,10) = 0.5;

      vBasis(0,11) = 2.5/3.0;
      vBasis(1,11) = 5.0/6.0;
      vBasis(2,11) = 0.5;

      vBasis(0,12) = 0.0;
      vBasis(1,12) = 2.0/3.0;
      vBasis(2,12) = 0.25;

      vBasis(0,13) = 0.5/3.0;
      vBasis(1,13) = 1.0/6.0;
      vBasis(2,13) = 0.25;

      vBasis(0,14) = 2.0/3.0;
      vBasis(1,14) = 2.0/3.0;
      vBasis(2,14) = 0.25;

      vBasis(0,15) = 2.5/3.0;
      vBasis(1,15) = 1.0/6.0;
      vBasis(2,15) = 0.25;

      vBasis(0,16) = 0.0;
      vBasis(1,16) = 2.0/3.0;
      vBasis(2,16) = 0.75;

      vBasis(0,17) = 0.5/3.0;
      vBasis(1,17) = 1.0/6.0;
      vBasis(2,17) = 0.75;

      vBasis(0,18) = 2.0/3.0;
      vBasis(1,18) = 2.0/3.0;
      vBasis(2,18) = 0.75;

      vBasis(0,19) = 2.5/3.0;
      vBasis(1,19) = 1.0/6.0;
      vBasis(2,19) = 0.75;

      // Define basis vectors

      vAxis(0,0) = 3.0*vLatticeParameters[0];
      vAxis(1,0) = 0.0;
      vAxis(2,0) = 0.0;

      vAxis(0,1) = 0.0;
      vAxis(1,1) = sqrt(3.0)*vLatticeParameters[1];
      vAxis(2,1) = 0.0;

      vAxis(0,2) = 0.0;
      vAxis(1,2) = 0.0;
      vAxis(2,2) = vLatticeParameters[2];

      // Define new type for Al atoms
      
      for (int i=12; i<20; i++)
	vType[i] = 2;

      // Rotate axis if necessary  (put a flag later...)

      double norm = sqrt(norm_vec[0] + norm_vec[1] + norm_vec[2]);
      if (norm > 1.e-5) vAxis = AxisRotation(vAxis);


    }
}


CORUNT::CORUNT(const CORUNT& source) : CrystalLatticeT(source.nLSD,source.nUCA,
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

  for (int i=0; i<source.nUCA; i++) 
    vType[i] = source.vType[i];

}

const dArray2DT& CORUNT::GetBasis() 
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

const dArray2DT& CORUNT::GetAxis() 
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

const dArrayT& CORUNT::GetLatticeParameters() 
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


