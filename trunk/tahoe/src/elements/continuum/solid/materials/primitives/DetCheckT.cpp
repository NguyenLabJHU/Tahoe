/* $Id: DetCheckT.cpp,v 1.20 2002-07-10 16:47:51 cfoster Exp $ */
/* created: paklein (09/11/1997) */

#include "DetCheckT.h"
#include <math.h>
#include "ExceptionCodes.h"
#include "dSymMatrixT.h"
#include "dMatrixT.h"
#include "dMatrixEXT.h"
#include "dArrayT.h"
#include "dTensor4DT.h"
//#include <fstream.h>
#include "ofstreamT.h"
# include "AutoArrayT.h"

/* constants */

using namespace Tahoe;

const double Pi = acos(-1.0);

/* constructor */
DetCheckT::DetCheckT(const dSymMatrixT& s_jl, const dMatrixT& c_ijkl):
	fs_jl(s_jl),
	fc_ijkl(c_ijkl)
{

}

/* inline functions */

/* determinant function and derivatives */
inline double DetCheckT::det(double t) const
{
	return A0 + A2*sin(2.0*(phi2+t)) + A4*sin(4.0*(phi4+t));
}

inline double DetCheckT::ddet(double t) const
{
	return 2.0*A2*cos(2.0*(phi2+t)) + 4.0*A4*cos(4.0*(phi4+t));
}

inline double DetCheckT::dddet(double t) const
{
	return -4.0*A2*sin(2.0*(phi2+t)) - 16.0*A4*sin(4.0*(phi4+t));
}


/* Tests whether two normals are equivalent, checking to see whether they
 * are the same within a given tolerance tol, or opposites within that same
 * tolerance (opposite normals are considered equilvalent for this purpose).
 * Returns true if the normals are distinct, false if not. 
 * Both vectors should be normalized before being passed into this function. 
 */

bool NormalCompare(const dArrayT& normal1, const dArrayT& normal2)
{
  double tol = 10e-10;
  dArrayT resid(3), negResid(3);
  
  resid.DiffOf(normal1, normal2);
  negResid.SumOf(normal1, normal2);

  return ( resid [0]*resid[0] + resid[1]*resid[1] + resid[2]*resid[2] < tol ||
  	  negResid [0]*negResid[0] + negResid[1]*negResid[1] + negResid[2]*negResid[2] < tol);
};



/*
* Returns 1 if acoustic tensor isn't positive definite,
* and returns the normal to the surface of localization.
* Returns 0, otherwise.
*/
int DetCheckT::IsLocalized(dArrayT& normal)
{
	if (fs_jl.Rows() == 2)
		return DetCheck2D(normal);
	else
	  {
	//TEMP - not implemented
	  cout << "Finite Strain Localization Check not implemented in 3D \n";
	  return 5;
	  }
}


/* check ellipticity of tangent modulus for small strain formulation 
* 2D version is closed form solution for plane strain taken from 
* R.A.Regueiro's SPINLOC.
* 3d is a numerical search algorithm after Ortiz, et. al. (1987) */

int DetCheckT::IsLocalized_SS(dArrayT& normal)
{
  /* clear normal */
  normal = 0.0;

	if (fs_jl.Rows() == 2)
	{
		/* call SPINLOC routine */
		double theta = 0.0;
		int check = 0; 
		SPINLOC_localize(fc_ijkl.Pointer(), &theta, &check);
		if (check == 1)
		{
			normal[0] = cos(theta);
			normal[1] = sin(theta);		
		}
		return check;
	}
	else
	// not ready yet. remove to run problem to debug
	return DetCheck3D_SS(normal);
	//return 7;    
}


/**********************************************************************
* Private
**********************************************************************/

/* 2D determinant check function */
int DetCheckT::DetCheck2D(dArrayT& normal)
{
	/* set det function */
	ComputeCoefficients();

	/* quick exit */
	if ( A0 > 0 && A0 > (fabs(A2) + fabs(A4)) ) return 0;

	/* local minima */
	double min2  = ((A2 > 0.0) ? 3.0*Pi/4.0 : Pi/4.0) - phi2;

	double dat[4] = {-Pi/8.0, 3.0*Pi/8.0, 7.0*Pi/8.0, 11.0*Pi/8.0};

	dArrayT mins4(4,dat);
	if (A4 > 0) mins4 -= phi4;
	else mins4 -= (phi4 + Pi/4.0);

	/* closest local minima in sin2t and sin4t*/
	int    dex;
	double min = Pi;
	for (int i = 0; i < 4; i++)
	{	
		double diff = fabs(mins4[i] - min2);

		if (diff < min)
		{	
			dex = i;
			min = diff;
		}
	}
	
	/* search starting value */
	double angle = 0.5*(min2 + mins4[dex]);
	double maxstep = Pi/18.0; //10 degrees max per step
	
	/* Newton search */
	double res  = ddet(angle);
	double res0 = res;
	int   count = 0;
	while ( fabs(res/res0) > 1.0e-10 && ++count < 15)
	{
		double dangle = res/dddet(angle);
		dangle = (fabs(dangle) > maxstep) ? maxstep*((dangle < 0) ? -1:1) : dangle;
	
		angle -= dangle;
		res  = ddet(angle);
	}

	/* check minima */
	if (dddet(angle) < 0.0)
	{
		cout << "\n DetCheckT::IsLocalized: ERROR:\n";
		cout << "f =  " << A0 << " + " << A2 << "*sin(2.0*(";
		cout << phi2 << "+t)) + " << A4 << "*sin(4.0*(";
		cout << phi4 << "+t))" << '\n';
		cout << "root found at " << angle << " rad" << endl;
		cout << "starting at   " << 0.5*(min2 + mins4[dex]) << " rad" << endl;


		/* search starting value */
		double angle = 0.5*(min2 + mins4[dex]);
		
		/* Newton search */
		double res  = ddet(angle);
		double res0 = res;
		int   count = 0;
		while( fabs(res/res0) > 1.0e-10 && ++count < 10)
		{
			double dangle = res/dddet(angle);

			cout << count << '\t' << res << '\t' << dangle;
		
			angle -= dangle;
			res  = ddet(angle);
		}
			throw eGeneralFail;
	}
	
	if (det(angle) > 0.0)
		return 0;
	else
	{
		normal.Allocate(2);

		/* angle in [0, Pi] */
		if (angle > Pi) angle -= Pi;
		
		/* compute normal */
		normal[0] = cos(angle);
		normal[1] = sin(angle);

		return 1;
	}
}

/* 3D determinant check function */
/* assumes small strain formulation */
int DetCheckT::DetCheck3D_SS(dArrayT& normal)
{
  int i,j,k,l,m,n; // counters 

  double theta, phi; //horizontal plane angle and polar angle for normal
  double detA [numThetaChecks] [numPhiChecks]; //determinant of acoustic tensor at each increment
  int localmin [numThetaChecks] [numPhiChecks]; //1 for local minimum, 0 if not

  int numev = 0; //number of eigenvectors for given eigenvalue
  double tol=1.0e-10; 
  double leastmin=2*tol; 
  int newtoncounter=0; //makes sure Newton iteration doesn't take too long
  double resid=0.0, guess=0.0;
  
  dMatrixEXT A(3), Ainverse(3); //acoustic tensor and inverse
  dTensor4DT C(3,3,3,3); // Rank 4 Tensor version of Tangent Modulus
  dMatrixEXT J(3); // det(A)*C*Ainverse

  /* for eigen analysis */
  dArrayT prevnormal(3), realev(3), imev(3), altnormal(3), altnormal2(3);

  dArrayT slipdir(3); //normal in direction of slip plane



  /* variables for normalSet output */
  const int outputPrecision = 12;
  const int outputFileWidth = outputPrecision + 4;
  AutoArrayT <dArrayT> normalSet;

  //const ElementCardT& element = CurrentElement();
  //const ElementSupportT& support = ContinuumElement().ElementSupport();
  
  /* Set up output file */
  normalSet.Free();
  ofstreamT normal_out("normal.info", ios::app);
  normal_out << "\ntime step    element #    ip#\n";
  // normal_out << support.StepNumber() << endl << endl;
  normal_out << setw(outputFileWidth) << "approx normal0" << setw(outputFileWidth) << "approx normal1" <<  setw(outputFileWidth) << " approx normal2" <<  setw(outputFileWidth) << "normal0" <<  setw(outputFileWidth) << "normal1" <<  setw(outputFileWidth) << "normal2" << setw(outputFileWidth + 3) << "detA" <<  setw(outputFileWidth) << "in normalSet?\n";
  
  // initialize variables
  normal=0.0;
  A = 0.0;
  C = 0.0;
  Ainverse = 0.0;
  J = 0.0;
  prevnormal = 0.0;
  realev = 0.0;
  imev = 0.0;
  altnormal = 0.0;
  altnormal2 = 0.0;
  A0 = 0.0;
  A2 = 0.0;
  slipdir=0.0;	
  for (m=0; m<numThetaChecks; m++)
    for (n=0; n<numPhiChecks; n++)
      {
	detA [m] [n] = 0.0;
	localmin [m] [n] = 0;
      }
  
  /* Get C from fc_ijkl */
  ConvertTangentFrom2DTo4D(C);
  FindApproxLocalMins(detA, localmin, C);
  
  /* Newton iteration to refine minima*/
  
  for (i=0; i<numThetaChecks; i++)
    {
      for (j=0 ;j<numPhiChecks; j++)
	{

	  if (localmin [i] [j] ==1)
	    {
     
	      theta=Pi/180.0*sweepIncr*i;
	      phi=Pi/180.0*sweepIncr*j;

	      normal[0]=cos(theta)*cos(phi);
	      normal[1]=sin(theta)*cos(phi);
	      normal[2]=sin(phi);
	     
	      normal_out << endl << setw(outputFileWidth) << normal[0] << setw(outputFileWidth) << normal[1] << setw(outputFileWidth) << normal[2];

	      newtoncounter=0;	    
	      resid=0.0;

	      while ( resid < (1.0-tol) && newtoncounter <= 100 )
		{
		  newtoncounter++;
		  if (newtoncounter > 100)
		    {
		      cout << "Warning: Bifurcation check failed. Newton refinement did not converge after 100 iterations. \n"; 
		      normal_out << setw(2*outputFileWidth) << "Did not converge";
		      //return 8;
		      //normal=0.0;
		      //return 0;
		    }

		  prevnormal=normal;
		  
		  // initialize acoustic tensor A
		  A = 0.0;
		  A.formacoustictensor(A, C, normal);
		  detA [i] [j]= A.Det();
		  Ainverse.Inverse(A);
		  
		  // initialize J
		  J = 0.0;

		  //form Jmn=det(A)*Cmkjn*(A^-1)jk
		  for (m=0;m<3;m++)
		    for (n=0;n<3;n++)
		      for (k=0;k<3;k++)
			for (l=0;l<3;l++)
			  {
			    J (m,n)+=detA [i] [j]*C(m,k,l,n)*Ainverse(l,k); 
			  }

		  // find least eigenvector of J
		  normal = ChooseNewNormal(prevnormal, J, tol);
		  
		  resid=fabs(prevnormal.Dot(prevnormal, normal));
		  //possibly change resid for more accuracy
		} //end while statement

	      /* output info to normal.info */
		   normal_out << setw(outputFileWidth) <<  normal[0] << setw(outputFileWidth) << normal [1] << setw(outputFileWidth) << normal[2] << setw(outputFileWidth + 3) << detA [i] [j];

	      if (detA [i] [j] - leastmin < - tol && (detA [i] [j] - leastmin)/fabs(leastmin) < -tol )
		{
		  //clear auto array
		  normalSet.Free();

		  // add normal to auto array and reset leastmin
		  normalSet.Append(normal);
		  leastmin = detA [i] [j];

		  /* output to normal.info */
		  normal_out << setw(outputFileWidth) << "Yes - 1st";
		}
	      else if (fabs(detA [i] [j] - leastmin) < tol || fabs((detA [i] [j] - leastmin)/leastmin) < tol )
		{
		  // add normal to auto array and output to normal.info
		  if (normalSet.AppendUnique(normal, NormalCompare))
		    normal_out << setw(outputFileWidth) << "Yes - Added";
		  else
		    normal_out << setw(outputFileWidth) << "Already Exists";
		}
	      else
		{
		  /* output to normal.info */
		  normal_out << setw(outputFileWidth) << "No";
		}

	    } //end if localmin  
	} //end j		  
    } //end i

  /* output of function */
  
  if (leastmin > tol)  //no bifurcation occured
    {
      normal=0.0;
      return 0;
    }
  else               //bifurcation occured
    {

      //choose normal from auto array
      normal = ChooseNormalFromNormalSet(normalSet, C); 

      //determine slip direction
      A.formacoustictensor(A, C, normal);

      A.eigvalfinder(A, realev, imev);
      
      guess=realev[0];
      if (realev[1]<guess)
	guess=realev[1];
      if (realev[2]<guess)
	guess=realev[2];
      
      A.eigenvector3x3(A, guess, numev, slipdir, altnormal, altnormal2);      

      return 1;
    }
} // end DetCheckT::DetCheck3D_SS

void DetCheckT::ConvertTangentFrom2DTo4D(dTensor4DT& C)
{
	
int i,j,k,l;

int HughesIndexArray [3] [3];

/* Set values for Hughes index array. HughesIndexArray(i,j) = k,
 * where k is the location in the vectorized (Voigt) version of
 * a symmetric 2D tensor, e.g. sigma(i,j) = s_ij(k)
 */
for (i = 0; i < 3; i++)
  HughesIndexArray[i] [i] = i;
HughesIndexArray [0] [1] = 5;
HughesIndexArray [1] [0] = 5;
HughesIndexArray [0] [2] = 4;
HughesIndexArray [2] [0] = 4;
HughesIndexArray [1] [2] = 3;
HughesIndexArray [2] [1] = 3; 

// convert tangent modulus from rank 2 to rank 4 using dTensor4DT

 for (i=0; i<3; i++)
   for (j=0; j<3; j++)
     for (k=0; k<3; k++)
       for (l=0; l<3; l++)
	 C (i,j,k,l) = 
	   fc_ijkl(HughesIndexArray[i] [j], HughesIndexArray[k] [l]);

} // end ConvertTangentFrom2DTo4D


 /* initial sweep in sweepIncr-degree increments to determine approximate 
  *local minima */

void DetCheckT::FindApproxLocalMins(double detA [numThetaChecks] [numPhiChecks],
  int localmin [numThetaChecks] [numPhiChecks], dTensor4DT& C)
{
  int i,j,k,l,m,n;
  double theta, phi; //horizontal plane angle and polar angle for normal, resp
  dMatrixEXT A(3), Ainverse(3); //acoustic tensor and its inverse
  dArrayT normal (3);

  //case where j = numPhiChecks - 1, i.e. pole

  normal = 0.0;
  normal [2] = 1.0;
  
  A = 0.0;
  A.formacoustictensor(A, C, normal);

  detA [0] [numPhiChecks - 1] = A.Det();

  for (i = 1; i < numThetaChecks; i++)
   detA [i] [numPhiChecks - 1] = detA [0] [numPhiChecks - 1];
  
  // general case
  for (i=0; i<numThetaChecks; i++)
    for (j=0; j<numPhiChecks - 1; j++)
      {

	theta=Pi/180.0*sweepIncr*i;
	phi=Pi/180.0*sweepIncr*j;

	normal[0]=cos(theta)*cos(phi);
	normal[1]=sin(theta)*cos(phi);
	normal[2]=sin(phi);

	A.formacoustictensor(A, C, normal);	

	detA [i] [j]= A.Det();
      }
	
  for (i=0; i<numThetaChecks; i++)
    for (j=0; j<numPhiChecks; j++)
      {
	if (j == numPhiChecks - 1)
	  {
	    if (i == 0)
	      {
		localmin [i] [j] = 1;
		for (k=0; k<numThetaChecks; k++)
		  if (detA [i] [j] > detA [k] [j - 1])
		    {
		      localmin [i] [j] = 0;
		      break;
		    }
	      }
	  }
	else
	  {
	    if (detA [i] [j] < detA [(i-1)%numThetaChecks] [j]) 
	      if (detA [i] [j] <= detA [(i+1)%numThetaChecks] [j])
		if (detA [i] [j] < detA [i] [j+1])
		    if ( j == 0)
		    {
		      if (detA [i] [j] <= detA [i + numThetaChecks/2] [j+1])
			localmin [i] [j] = 1;
		    }
		    else
		    {
		      if (detA [i] [j] <= detA [i] [j-1])
			localmin [i] [j] = 1;
		    }
	  }
      } //end j, end i   
} // end FindApproxLocalMins


  dArrayT DetCheckT::ChooseNewNormal(dArrayT& prevnormal, dMatrixEXT& J,
  			    double tol)
{

  dArrayT normal(3), trialNormal(3);
  dArrayT altnormal(3), altnormal2(3);
  dArrayT realev(3), imev(3);
  int numev = 0, i;
  dMatrixEXT Atrial(3); //trial acoustic tensor
  double inprod, maxInprod;
  Atrial = 0.0;
  trialNormal = 0.0;

  // chooses eigvector by closest approx to previous normal
  J.eigvalfinder(J, realev, imev);

  for (i=0; i<3; i++)
    {  
      if ( fabs(imev[i])<tol || fabs(0.001*imev[i]/realev[i])<tol )
	{

	  J.eigenvector3x3(J, realev[i], numev, trialNormal, altnormal, altnormal2);
	  //J.Eigenvector(realev[i], trialNormal);
	  inprod = fabs(trialNormal.Dot(trialNormal, prevnormal));
	  
	  if ( i==0 )
	    {
	      maxInprod = inprod;
	      normal = trialNormal;
	    }
	  else if (inprod > maxInprod)
	    {
	      maxInprod = inprod;
	      normal = trialNormal;
	    }	    
	}
    }
  return normal;
}



dArrayT DetCheckT::ChooseNormalFromNormalSet(AutoArrayT <dArrayT> &normalSet, dTensor4DT &C)
{
  dMatrixEXT A(3);
  dArrayT trialNormal(3), bestNormal(3);
  int i;
  double leastmin, detA;
  
  bestNormal = 0.0;

  normalSet.Top();

while (normalSet.Next())
    {
      /* Currently chooses normal by least minimum value of the determinant
       * of the acoustic tensor A. Experience suggests that two unique 
       * normals are created and that this choice is based simply on 
       * numerical error. Better to choose another criterion. Wells and Sluys
       * (2001) suggest maximum plastic dissipation, which may be best
       * handled in the function calling IsLocalized_SS, after it is called.  
       */

      trialNormal = normalSet.Current();
      A.formacoustictensor(A, C, trialNormal);
      detA = A.Det();
      if (detA < leastmin || normalSet.Position() == 0)
	{
	  leastmin = detA;
	  bestNormal = trialNormal;
	}
    }      
      return bestNormal;
}

/* compute coefficients of det(theta) function */
void DetCheckT::ComputeCoefficients(void)
{
	/* moduli components */
	double c11 = fc_ijkl(0,0);
	double c22 = fc_ijkl(1,1);
	double c33 = fc_ijkl(2,2);
	double c23 = fc_ijkl(1,2);
	double c13 = fc_ijkl(0,2);
	double c12 = fc_ijkl(0,1);
	
	/* stress components */
	double s11 = fs_jl[0];
	double s22 = fs_jl[1];
	double s12 = fs_jl[2];

	/* intermediate values */
	double s2t = (-(c12*c13) + c13*c22 + c11*c23 - c12*c23 + c13*s11 +
			c23*s11 + c11*s12 + c22*s12 + 2*c33*s12 + 2*s11*s12 + c13*s22 +
			c23*s22 + 2*s12*s22)/2;
	double s4t = (-(c12*c13) - c13*c22 + c11*c23 + c12*c23 + c13*s11 +
			c23*s11 + c11*s12 - c22*s12 + 2*s11*s12 - c13*s22 - c23*s22 -
			2*s12*s22)/4;
			
	double c2t = (-c13*c13 + c23*c23 + c11*c33 - c22*c33 + c11*s11 + c33*s11
			+ s11*s11 - c22*s22 - c33*s22 - s22*s22)/2;
	double c4t = (c12*c12 - c13*c13 - c11*c22 - 2*c13*c23 - c23*c23 +
			c11*c33 + 2*c12*c33 + c22*c33 + c11*s11 - c22*s11 + s11*s11 -
			4*c13*s12 - 4*c23*s12 - 4*s12*s12 - c11*s22 + c22*s22 - 2*s11*s22
			+ s22*s22)/8;

	/* phase shifts */
	phi2 = atan2(c2t,s2t)/2.0;
	phi4 = atan2(c4t,s4t)/4.0;
	
	/* amplitudes */
	A0 = (-c12*c12 - 3*c13*c13 + c11*c22 + 2*c13*c23 - 3*c23*c23 +
			3*c11*c33 - 2*c12*c33 + 3*c22*c33 + 3*c11*s11 + c22*s11 +
			4*c33*s11 + 3*s11*s11 + 4*c13*s12 + 4*c23*s12 + 4*s12*s12 +
			c11*s22 + 3*c22*s22 + 4*c33*s22 + 2*s11*s22 + 3*s22*s22)/8;

	A2 = c2t/sin(2.0*phi2);
	A4 = c4t/sin(4.0*phi4);
}

/* ***************************************************************** */
/* closed-form check for localization, assuming plane strain condition */
/* 1 is 11 */
/* 2 is 22 */
/* 3 is 12 */
/* angle theta subtends from the x1 axis to the band normal */
int DetCheckT::SPINLOC_localize(double *c__, double *thetan, int *loccheck)
{
    /* Initialized data */
    double zero = 0.;
    double one = 1.;
    double two = 2.;
    double three = 3.;
    double four = 4.;
    double tol = .01;

    /* System generated locals */
    int i__1;
    double d__1, d__2, d__3, d__4;

    /* Local variables */
    double capa, capb, half, fmin, temp, xmin, temp2, temp3, a, b, 
	    f;
    int i__, n;
    double p, q, r__, x[3], theta, third, a0, a1, a2, a3, a4, qq, rad;

    /* Parameter adjustments */
    c__ -= 4;

    /* Function Body */
    half = one / two;
    third = one / three;
    rad = four * atan(one) / 180.;


    //  cout << "c__=\n";
    //  cout << c__[4] << ' ' <<  c__[7] << ' ' << c__[10] << '\n';
    //  cout << c__[5] << ' ' <<  c__[8] << ' ' << c__[11] << '\n';
    //  cout << c__[6] << ' ' <<  c__[9] << ' ' << c__[12] << '\n';


    a0 = c__[4] * c__[12] - c__[10] * c__[6];
    a1 = c__[4] * (c__[9] + c__[11]) - c__[10] * c__[5] - c__[6] * c__[7];
    a2 = c__[4] * c__[8] + c__[10] * c__[9] + c__[6] * c__[11] - c__[7] * (
	    c__[12] + c__[5]) - c__[12] * c__[5];
    a3 = c__[8] * (c__[10] + c__[6]) - c__[11] * c__[7] - c__[9] * c__[5];
    a4 = c__[12] * c__[8] - c__[9] * c__[11];

    p = three / four * (a3 / a4);
    q = a2 / a4 * (one / two);
    r__ = a1 / a4 * (one / four);

	/* Computing 2nd power */
    d__1 = p;
    a = (three * q - d__1 * d__1) * third;
/* Computing 3rd power */
    d__1 = p, d__2 = d__1;
    b = (two * (d__2 * (d__1 * d__1)) - p * 9. * q + r__ * 27.) / 27.;

/* Computing 2nd power */
    d__1 = b;
/* Computing 3rd power */
    d__2 = a, d__3 = d__2;
    qq = d__1 * d__1 / four + d__3 * (d__2 * d__2) / 27.;
    if (fabs(qq) < 1e-8) {
	qq = zero;
    }

    temp = p * third;

    if (qq > zero || qq == 0.f) {
	temp2 = one;
	temp3 = -half * b + sqrt(qq);
	if (temp3 < zero) {
	    temp2 = -one;
	}
	d__1 = fabs(temp3);
	capa = temp2 * pow(d__1, third);
	temp2 = one;
	temp3 = -half * b - sqrt(qq);
	if (temp3 < zero) {
	    temp2 = -one;
	}
	d__1 = fabs(temp3);
	capb = temp2 * pow(d__1, third);
	x[0] = capa + capb - temp;
	x[1] = -(capa + capb) * half - temp;
	x[2] = x[1];
    } else {
	if (a < zero) {
/* Computing 3rd power */
	    d__2 = a * third, d__3 = d__2;
	    theta = acos(-half * b / sqrt((d__1 = -(d__3 * (d__2 * d__2)), 
		    fabs(d__1))));
	    temp2 = two * sqrt((d__1 = -a * third, fabs(d__1)));
	    x[0] = temp2 * cos(theta * third) - temp;
	    x[1] = -temp2 * cos(theta * third + rad * 60.) - temp;
	    x[2] = -temp2 * cos(theta * third - rad * 60.) - temp;
	} else {
		cout << "\n DetCheckT::SPINLOC_localize: a is positive when it should be negative" << endl;
	}
    }

    fmin = 1e50;
    n = 3;
    if (fabs(qq) < 1e-8) {
	n = 2;
    }
    if (qq > zero) {
	n = 1;
    }
    i__1 = n;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing 4th power */
	d__1 = x[i__ - 1], d__1 *= d__1;
/* Computing 3rd power */
	d__2 = x[i__ - 1], d__3 = d__2;
/* Computing 2nd power */
	d__4 = x[i__ - 1];
	f = a4 * (d__1 * d__1) + a3 * (d__3 * (d__2 * d__2)) + a2 * (d__4 * 
		d__4) + a1 * x[i__ - 1] + a0;
	if (f <= fmin) {
	    fmin = f;
	    xmin = x[i__ - 1];
	}
/* L5: */
    }

/* .. output */

    *thetan = atan(xmin);

/* 	if(fmin.lt.tol) then */
    if (fmin / c__[4] < tol) {
/* localized */
	*loccheck = 1;
    } else {
/* not localized */
	*loccheck = 0;
    }
    return 0;
}

