/* $Id: CrystalLatticeT.cpp,v 1.7 2002-09-09 23:10:29 saubry Exp $ */
#include "CrystalLatticeT.h"

#include <iostream>
#include <fstream>
#include "StringT.h"
#include "ifstreamT.h"
#include "ExceptionCodes.h"
#include "dArrayT.h"
#include "dArray2DT.h"

#include "dMatrixT.h"
#include "Rotate2DT.h"
#include "Rotate3DT.h"

CrystalLatticeT::CrystalLatticeT(int nlsd, int nuca,
				 dArray2DT mat_rot,double angle) 
{
	nLSD = nlsd;
	nUCA = nuca;
        vBasis.Dimension(nLSD,nUCA);
        vLatticeParameters.Dimension(nLSD);
        vAxis.Dimension(nLSD,nLSD);

	// Define rotation
	if(nLSD == 3)
	  {
	    // Directions in 3D only
	    if(mat_rot.MinorDim() != nLSD) throw eSizeMismatch;
	    if(mat_rot.MajorDim() != nLSD) throw eSizeMismatch;
	    matrix_rotation.Dimension(nLSD,nLSD);
	    
	    norm_vec.Dimension(nLSD);

	    for (int j=0; j<nLSD; j++)
	      {
		norm_vec[j] = 0.0;
		for (int i=0; i<nLSD; i++)
		  norm_vec[j] += mat_rot(i,j)*mat_rot(i,j);
		norm_vec[j] = sqrt(norm_vec[j]);
	      }

	    for (int j=0; j<nLSD; j++)
	      {	
		if(norm_vec[j] != 0) 
		  {
		    for (int i=0; i<nLSD; i++) 
		      for (int j=0; j<nLSD; j++) 
			matrix_rotation(i,j) = mat_rot(i,j)/norm_vec[j];
		  }
	      }

	    // check input matrix
	    double yz = matrix_rotation(1,0)*matrix_rotation(2,1)-
                        matrix_rotation(1,1)*matrix_rotation(2,0);
	    double xz = matrix_rotation(0,1)*matrix_rotation(2,0)-
                        matrix_rotation(0,0)*matrix_rotation(2,1); 
	    double xy = matrix_rotation(0,0)*matrix_rotation(1,1)-
	                matrix_rotation(0,1)*matrix_rotation(1,0);  
	    if ( fabs(yz-matrix_rotation(0,2)) <= 1.e-5 ||
		 fabs(xz-matrix_rotation(1,2)) <= 1.e-5 ||
		 fabs(xy-matrix_rotation(2,2)) <= 1.e-5 ) 

	      {
		/*
		  cout << "Matrix of rotation is not right.\n Changing it ...\n";
		  cout << "was " << matrix_rotation(0,2) << "  " 
	 	       << matrix_rotation(1,2) << "  " 
		       << matrix_rotation(2,2) << "\n";
		  cout << "is now " << yz << "  " 
		       << xz << "  " 
		       << xy << "\n";
		*/
		matrix_rotation(0,2) = yz;
		matrix_rotation(1,2) = xz;
		matrix_rotation(2,2) = xy;
	      }

	    angle_rotation = 0;
	  }
	else if(nLSD == 2)  
	  {
	    // input angle has to be in degrees
	    angle_rotation = angle;
	  }
}

CrystalLatticeT::CrystalLatticeT(const CrystalLatticeT& source) 
{

  nLSD = source.nLSD;
  nUCA = source.nUCA;

  vBasis.Dimension(nLSD,nUCA);
  vBasis = source.vBasis;

  vLatticeParameters.Dimension(nLSD);
  vLatticeParameters = source.vLatticeParameters;

  density = source.density;
}


void CrystalLatticeT::CalculateDensity() 
{
	double ucvolume = 1.0;
	for (int i=0; i<nLSD; i++) 
	  ucvolume *= vLatticeParameters[i];
	density = ((double) nUCA)/ucvolume;
}

double CrystalLatticeT::GetDensity() 
{
	return density;
}

dArray2DT  CrystalLatticeT::AxisRotation(dArray2DT A)
{
  if(A.MajorDim() != nLSD || A.MinorDim() != nLSD) 
    throw eSizeMismatch;

  dArray2DT B(nLSD,nLSD);
  dMatrixT Q(nLSD,nLSD);

  if(nLSD==2)
    {
      if(fabs(angle_rotation) <= 1.e-5) return A;
      Rotate2DT R(angle_rotation);
      Q = R.Q();

      for (int i=0; i<nLSD; i++)
	for (int j=0; j<nLSD; j++)
	  B(i,j) = 0.0;

      cout << "Q:\n";
      cout << Q(0,0) << "  " << Q(1,0) << "\n";
      cout << Q(0,1) << "  " << Q(1,1) << "\n\n";

      for (int i=0; i<nLSD; i++)
	for (int j=0; j<nLSD; j++)
	  {
	    for (int k=0; k<nLSD; k++)
	      B(i,j) += Q(i,k)*A(k,j);
	  }
      
      
    }
  else if(nLSD==3)
    {
      double norm = norm_vec[0] + norm_vec[1] + norm_vec[2];
      if (norm < 1.e-5) return A;

      Rotate3DT R;
      R.GiveTransfoMatrix(matrix_rotation);
      Q = R.Q();
      
      for (int i=0; i<nLSD; i++)
	for (int j=0; j<nLSD; j++)
	  B(i,j) = 0.0;
      
      for (int i=0; i<nLSD; i++)
	for (int j=0; j<nLSD; j++)
	  {
	    for (int k=0; k<nLSD; k++)
	      B(i,j) += Q(i,k)*A(k,j);
	  }
    }

  return B;
}
