/* $Id: RectGaussianWindowT.cpp,v 1.3.54.1 2004-04-24 19:57:38 paklein Exp $ */

#include "RectGaussianWindowT.h"
#include "ExceptionT.h"
#include <math.h>


using namespace Tahoe;

const double sqrtPi = sqrt(acos(-1.0));
static double Max(double a, double b) { return (a > b) ? a : b; };

/* constructor */
RectGaussianWindowT::RectGaussianWindowT(const dArrayT& dilation_scaling, double sharpening_factor,
						       double cut_off_factor):
	fDilationScaling(dilation_scaling),
	fSharpeningFactor(sharpening_factor),
	fCutOffFactor(cut_off_factor)
{
        int count = 0;
        for (int i = 0; i < fDilationScaling.Length(); i++)
	{
	  if (fDilationScaling[i] < 0.0)
	    count++;
	}
	if (count > 0 || fSharpeningFactor < 0.0)
		throw ExceptionT::kBadInputValue;
}

/* "synchronization" of nodal field parameters. */
void RectGaussianWindowT::SynchronizeSupportParameters(dArray2DT& params_1, 
	dArray2DT& params_2) const
{
	/* should be over the same global node set (marked by length) */
	if (params_1.Length() != params_2.Length() ||
	    params_1.MinorDim() != NumberOfSupportParameters() ||
	    params_2.MinorDim() != NumberOfSupportParameters())
	{
		cout << "\n RectGaussianWindowT::SynchronizeSupportParameters: nodal\n"
		     << " parameters dimension mismatch" << endl;
		throw ExceptionT::kSizeMismatch;
	}
		
	/* "synchronize" means take max of dmax */
	double* p1 = params_1.Pointer();
	double* p2 = params_2.Pointer();
	int length = params_1.Length();
	for (int i = 0; i < length; i++)
	{
		*p1 = *p2 = Max(*p1, *p2);
		p1++; p2++;
	}
}

void RectGaussianWindowT::WriteParameters(ostream& out) const
{
	/* window function parameters */
	out << " Dilation scaling factor . . . . . . . . . . . . :\n" << fDilationScaling << '\n';
	out << " Window function sharpening factor . . . . . . . = " << fSharpeningFactor << '\n';
	out << " Neighbor cutoff factor. . . . . . . . . . . . . = " << fCutOffFactor << '\n';
}

/* Single point evaluations */
bool RectGaussianWindowT::Window(const dArrayT& x_n, const dArrayT& param_n, const dArrayT& x,
		int order, double& w, dArrayT& Dw, dSymMatrixT& DDw)
{
  /* Compute window function and its derivatives - accomplish by scalar product of individual
   * window functions in x/y/z directions */

  if (!RectGaussianWindowT::Covers(x_n, x, param_n))
  {
    w = 0.0;
    if (order > 0)
    {
      Dw = 0.0;
      if (order > 1)
	DDw = 0.0;
    }
    return false;
  }
  else      
  {
    Dw.DiffOf(x, x_n);
    if (x.Length() == 2)      // 2D calculation
    {
      double admx = param_n[0] * fSharpeningFactor;
      double admy = param_n[1] * fSharpeningFactor;
      double admx2 = admx * admx;
      double admy2 = admy * admy;
      double qx = Dw[0] / admx;
      double qy = Dw[1] / admy;
      double wx = exp(-qx * qx) / (sqrtPi * admx);
      double wy = exp(-qy * qy) / (sqrtPi * admy);
      w = wx * wy;
      if (order > 0)
      {
	if (order > 1)
	{
	  DDw[0] = (2.0 * w / admx2) * (2.0 * Dw[0] * Dw[0] / admx2 - 1);
	  DDw[1] = (2.0 * w / admy2) * (2.0 * Dw[1] * Dw[1] / admy2 - 1);
	}
	Dw[0] *= -2.0 * w / admx2;
	Dw[1] *= -2.0 * w / admy2;
      }

    }
    else if (x.Length() == 3)     // 3D calculation
    {
      double admx = param_n[0] * fSharpeningFactor;
      double admy = param_n[1] * fSharpeningFactor;
      double admz = param_n[2] * fSharpeningFactor;
      double admx2 = admx * admx;
      double admy2 = admy * admy;
      double admz2 = admz * admz;
      double qx = Dw[0] / admx;
      double qy = Dw[1] / admy;
      double qz = Dw[2] / admz;
      double wx = exp(-qx * qx) / (sqrtPi * admx);
      double wy = exp(-qy * qy) / (sqrtPi * admy);
      double wz = exp(-qz * qz) / (sqrtPi * admz);
      w = wx * wy * wz;
      if (order > 0)
      {
	if (order > 1)
	{
	  DDw[0] = (2.0 * w / admx2) * (2.0 * Dw[0] * Dw[0] / admx2 - 1);
	  DDw[1] = (2.0 * w / admy2) * (2.0 * Dw[1] * Dw[1] / admy2 - 1);
	  DDw[2] = (2.0 * w / admz2) * (2.0 * Dw[2] * Dw[2] / admz2 - 1);
	}
	Dw[0] *= -2.0 * w / admx2;
	Dw[1] *= -2.0 * w / admy2;
	Dw[2] *= -2.0 * w / admz2;
      }
    }
    else
      throw ExceptionT::kGeneralFail;
      
		return true;
  }
  return false;
}

/* multiple point calculations */
int RectGaussianWindowT::Window(const dArray2DT& x_n, const dArray2DT& param_n, const dArrayT& x,
		int order, dArrayT& w, dArray2DT& Dw, dArray2DT& DDw)
{
  /* compute window function and derivatives for multiple field points */

  /* allocate */
  int nsd = x.Length();
  fNSD.Dimension(nsd);
  fNSDsym.Dimension(nsd);

  /* work space */
  dArrayT x_node, param_node;
  int count = 0;    
  int numwindowpoints = x_n.MajorDim(); 

  for (int i = 0; i < numwindowpoints; i++)
  {
    /* collect nodal values */
    x_n.RowAlias(i, x_node);
    param_n.RowAlias(i, param_node);
      
    if (RectGaussianWindowT::Window(x_node, param_node, x, order, w[i], fNSD, fNSDsym))
      count ++;

    /* store derivatives */
    if (order > 0)
    {
      Dw.SetColumn(i, fNSD);
      if (order > 1)
	DDw.SetColumn(i, fNSDsym);
    }
  }
  return count;
}

bool RectGaussianWindowT::Covers(const dArrayT& x_n, const dArrayT& x, 
	const dArrayT& param_n) const
{
  dArrayT dx(x.Length());
  dx.DiffOf(x, x_n);

  /* check individual directions to see if outside the "box" */
  for (int i = 0; i < x.Length(); i++)
  {
    if (fabs(dx[i]) > fCutOffFactor * param_n[i])
      return false;
  }
  return true;
}

int RectGaussianWindowT::Covers(const dArray2DT& x_n, const dArrayT& x, 
	const dArray2DT& param_n, ArrayT<bool>& covers) const
{
  int count = 0;
  int numwindowpoints = x_n.MajorDim();        // Could be MajorDim!
  dArrayT dx(x.Length()), temprow(x.Length());
  for (int i = 0; i < numwindowpoints; i++)
  {
    x_n.RowCopy(i, temprow);          // Could be COLUMN copy!!!
    dx.DiffOf(x, temprow);
    for (int j = 0; j < x.Length(); j++)
    {
      if (fabs(dx[j]) > fCutOffFactor * param_n(i,j))
		count++;
    }
    if (count == 0)
      covers[i] = true;
    else
      covers[i] = false;
  }
  return count;
}

/* spherical upport size */
double RectGaussianWindowT::SphericalSupportSize(const dArrayT& param_n) const
{
#pragma unused(param_n)
	ExceptionT::GeneralFail("RectGaussianWindowT", "not implemented");
	return 0.0;
}

/* rectangular support size */
const dArrayT& RectGaussianWindowT::RectangularSupportSize(const dArrayT& param_n) const
{
#pragma unused(param_n)
	ExceptionT::GeneralFail("RectGaussianWindowT", "not implemented");
	return param_n; /* dummy */
}

/* spherical support sizes in batch */
void RectGaussianWindowT::SphericalSupportSize(const dArray2DT& param_n, ArrayT<double>& support_size) const
{
#pragma unused(param_n)
#pragma unused(support_size)
	ExceptionT::GeneralFail("RectGaussianWindowT", "not implemented");
}

/* rectangular support sizes in batch */
void RectGaussianWindowT::RectangularSupportSize(const dArray2DT& param_n, dArray2DT& support_size) const
{
#pragma unused(param_n)
#pragma unused(support_size)
	ExceptionT::GeneralFail("RectGaussianWindowT", "not implemented");
}