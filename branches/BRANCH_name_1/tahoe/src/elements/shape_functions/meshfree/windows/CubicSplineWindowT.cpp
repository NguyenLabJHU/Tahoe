#include "CubicSplineWindowT.h"
#include "ExceptionCodes.h"
#include <math.h>


using namespace Tahoe;

const double sqrtPi = sqrt(acos(-1.0));
static double Max(double a, double b) { return (a > b) ? a : b; };

/* constructor */
CubicSplineWindowT::CubicSplineWindowT(double dilation_scaling, double sharpening_factor,
						       double cut_off_factor):
	fDilationScaling(dilation_scaling),
	fSharpeningFactor(sharpening_factor),
	fCutOffFactor(cut_off_factor)
{
	if (fDilationScaling || fSharpeningFactor < 0.0)
		throw eBadInputValue;
}

/* "synchronization" of nodal field parameters. */
void CubicSplineWindowT::SynchronizeSupportParameters(dArray2DT& params_1, 
	dArray2DT& params_2) const
{
	/* should be over the same global node set (marked by length) */
	if (params_1.Length() != params_2.Length() ||
	    params_1.MinorDim() != NumberOfSupportParameters() ||
	    params_2.MinorDim() != NumberOfSupportParameters())
	{
		cout << "\n CubicSplineWindowT::SynchronizeSupportParameters: nodal\n"
		     << " parameters dimension mismatch" << endl;
		throw eSizeMismatch;
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

/* modify nodal shape function parameters */
void CubicSplineWindowT::ModifySupportParameters(dArray2DT& nodal_params) const
{
	/* scale supports */
        nodal_params *= fDilationScaling;	
}

void CubicSplineWindowT::WriteParameters(ostream& out) const
{
	/* window function parameters */
	out << " Dilation scaling factor . . . . . . . . . . . . :\n" << fDilationScaling << '\n';
	out << " Window function sharpening factor . . . . . . . = " << fSharpeningFactor << '\n';
	out << " Neighbor cutoff factor. . . . . . . . . . . . . = " << fCutOffFactor << '\n';
}

/* Single point evaluations */
bool CubicSplineWindowT::Window(const dArrayT& x_n, const dArrayT& param_n, const dArrayT& x,
		int order, double& w, dArrayT& Dw, dSymMatrixT& DDw)
{
  /* Compute window function and its derivatives - accomplish by scalar product of individual
   * window functions in x/y/z directions */

  if (!CubicSplineWindowT::Covers(x_n, x, param_n))
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
      double wx, wy;
      double rx = Dw[0] / param_n[0];
      double ry = Dw[1] / param_n[0];

      /* check x-direction */
      if ((rx >= -2) && (rx < -1))
	wx = 1 / (6 * param_n[0]) * (2 + rx) * (2 + rx) * (2 + rx);
      else if ((rx >= -1) && (rx < 0))
	wx = (2 / 3 - rx * rx - .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 0) && (rx < 1))
	wx = (2 / 3 - rx * rx + .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 1) && (rx <= 2))
	wx = 1 / (6 * param_n[0]) * (2 - rx) * (2 - rx) * (2 - rx);
      else
	wx = 0.0;

      /* check y-direction */
      if ((ry >= -2) && (ry < -1))
	wy = 1 / (6 * param_n[0]) * (ry + 2) * (ry + 2) * (ry + 2);
      else if ((ry >= -1) && (ry < 0))
	wy = 1 / param_n[0] * (2/3 - ry * ry * (1 + .5 * ry));
      else if ((ry >= 0) && (ry < 1))
	wy = 1 / param_n[0] * (2/3 - ry * ry * (1 - .5 * ry));
      else if ((ry >= 1) && (ry <= 2))
	wy = 1 / (6 * param_n[0]) * (2 - ry) * (2 - ry) * (2 - ry);
      else
	wy = 0.0;

      w = wx * wy;

      if (order > 0)
      {
	if (order > 1)
	{
	  /* check x-direction */
	  if ((rx >= -2) && (rx < -1))
	    DDw[0] = wy / (param_n[0] * param_n[0] * param_n[0]) * (2 + rx);
	  else if ((rx >= -1) && (rx < 0))
	    DDw[0] = - wy / (param_n[0] * param_n[0] * param_n[0]) * (2 + 3 * rx);
	  else if ((rx >= 0) && (rx < 1))
	    DDw[0] = - wy / (param_n[0] * param_n[0] * param_n[0]) * (2 - 3 * rx);
	  else if ((rx >= 1) && (rx <= 2))
	    DDw[0] = wy / (param_n[0] * param_n[0] * param_n[0]) * (2 - rx);
	  else
	    DDw[0] = 0.0;
	  
	  /* check y-direction */
	  if ((ry >= -2) && (ry < -1))
	    DDw[1] = wx / (param_n[0] * param_n[0] * param_n[0]) * (2 + ry);
	  else if ((ry >= -1) && (ry < 0))
	    DDw[1] = - wx / (param_n[0] * param_n[0] * param_n[0]) * (2 + 3 * ry);
	  else if ((ry >= 0) && (ry < 1))
	    DDw[1] = - wx / (param_n[0] * param_n[0] * param_n[0]) * (2 - 3 * ry);
	  else if ((ry >= 1) && (ry <= 2))
	    DDw[1] = wx / (param_n[0] * param_n[0] * param_n[0]) * (2 - ry);
	  else
	    DDw[1] = 0.0;
	}

	/* check x-direction */
	if ((rx >= -2) && (rx < -1))
	  Dw[0] = - .5 * wy / (param_n[0] * param_n[0]) * (2 + rx) * (2 + rx);
	else if ((rx >= -1) && (rx < 0))
	  Dw[0] = wy / (param_n[0] * param_n[0]) * (2 * rx + 1.5 * rx * rx);
	else if ((rx >= 0) && (rx < 1))
	  Dw[0] = wy / (param_n[0] * param_n[0]) * (2 * rx - 1.5 * rx * rx);
	else if ((rx >= 1) && (rx <= 2))
	  Dw[0] = .5 * wy / (param_n[0] * param_n[0]) * (2 - rx) * (2 - rx);
	else
	  Dw[0] = 0.0;
	  
	  /* check y-direction */
	if ((ry >= -2) && (ry < -1))
	  Dw[1] = - .5 * wx / (param_n[0] * param_n[0]) * (2 + ry) * (2 + ry);
	else if ((ry >= -1) && (ry < 0))
	  Dw[1] = wx / (param_n[0] * param_n[0]) * (2 * ry + 1.5 * ry * ry);
	else if ((ry >= 0) && (ry < 1))
	  Dw[1] = wx / (param_n[0] * param_n[0]) * (2 * ry - 1.5 * ry * ry);
	else if ((ry >= 1) && (ry <= 2))
	  Dw[1] = .5 * wx / (param_n[0] * param_n[0]) * (2 - ry) * (2 - ry);
	else
	  Dw[1] = 0.0;
      }
    }
    else if (x.Length() == 3)     // 3D calculation
    {
      double wx, wy, wz;      
      double rx = Dw[0] / param_n[0];
      double ry = Dw[1] / param_n[0];
      double rz = Dw[2] / param_n[0];
      
      /* check x-direction */
      if ((rx >= -2) && (rx < -1))
	wx = 1 / (6 * param_n[0]) * (2 + rx) * (2 + rx) * (2 + rx);
      else if ((rx >= -1) && (rx < 0))
	wx = (2 / 3 - rx * rx - .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 0) && (rx < 1))
	wx = (2 / 3 - rx * rx + .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 1) && (rx <= 2))
	wx = 1 / (6 * param_n[0]) * (2 - rx) * (2 - rx) * (2 - rx);
      else
	wx = 0.0;

      /* check y-direction */
      if ((ry >= -2) && (ry < -1))
	wy = 1 / (6 * param_n[0]) * (ry + 2) * (ry + 2) * (ry + 2);
      else if ((ry >= -1) && (ry < 0))
	wy = 1 / param_n[0] * (2/3 - ry * ry * (1 + .5 * ry));
      else if ((ry >= 0) && (ry < 1))
	wy = 1 / param_n[0] * (2/3 - ry * ry * (1 - .5 * ry));
      else if ((ry >= 1) && (ry <= 2))
	wy = 1 / (6 * param_n[0]) * (2 - ry) * (2 - ry) * (2 - ry);
      else
	wy = 0.0;

      /* check z-direction */
      if ((rz >= -2) && (rz < -1))
	wz = 1 / (6 * param_n[0]) * (rz + 2) * (rz + 2) * (rz + 2);
      else if ((rz >= -1) && (rz < 0))
	wz = 1 / param_n[0] * (2/3 - rz * rz * (1 + .5 * rz));
      else if ((rz >= 0) && (rz < 1))
	wz = 1 / param_n[0] * (2/3 - rz * rz * (1 - .5 * rz));
      else if ((rz >= 1) && (rz <= 2))
	wz = 1 / (6 * param_n[0]) * (2 - rz) * (2 - rz) * (2 - rz);
      else
	wz = 0.0;

      w = wx * wy * wz;
      if (order > 0)
      {
	if (order > 1)
	{
	  /* check x-direction */
	  if ((rx >= -2) && (rx < -1))
	    DDw[0] = wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 + rx);
	  else if ((rx >= -1) && (rx < 0))
	    DDw[0] = - wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 + 3 * rx);
	  else if ((rx >= 0) && (rx < 1))
	    DDw[0] = - wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 - 3 * rx);
	  else if ((rx >= 1) && (rx <= 2))
	    DDw[0] = wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 - rx);
	  else
	    DDw[0] = 0.0;
	  
	  /* check y-direction */
	  if ((ry >= -2) && (ry < -1))
	    DDw[1] = wx * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 + ry);
	  else if ((ry >= -1) && (ry < 0))
	    DDw[1] = - wx * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 + 3 * ry);
	  else if ((ry >= 0) && (ry < 1))
	    DDw[1] = - wx * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 - 3 * ry);
	  else if ((ry >= 1) && (ry <= 2))
	    DDw[1] = wx * wz / (param_n[0] * param_n[0] * param_n[0]) * (2 - ry);
	  else
	    DDw[1] = 0.0;

	  /* check z-direction */
	  if ((rz >= -2) && (rz < -1))
	    DDw[2] = wx * wy / (param_n[0] * param_n[0] * param_n[0]) * (2 + rz);
	  else if ((rz >= -1) && (rz < 0))
	    DDw[2] = - wx * wy / (param_n[0] * param_n[0] * param_n[0]) * (2 + 3 * rz);
	  else if ((rz >= 0) && (rz < 1))
	    DDw[2] = - wx * wy / (param_n[0] * param_n[0] * param_n[0]) * (2 - 3 * rz);
	  else if ((rz >= 1) && (rz <= 2))
	    DDw[2] = wx * wy / (param_n[0] * param_n[0] * param_n[0]) * (2 - rz);
	  else
	    DDw[2] = 0.0;
	}

	/* check x-direction */
	if ((rx >= -2) && (rx < -1))
	  Dw[0] = - .5 * wy * wz / (param_n[0] * param_n[0]) * (2 + rx) * (2 + rx);
	else if ((rx >= -1) && (rx < 0))
	  Dw[0] = wy * wz / (param_n[0] * param_n[0]) * (2 * rx + 1.5 * rx * rx);
	else if ((rx >= 0) && (rx < 1))
	  Dw[0] = wy * wz / (param_n[0] * param_n[0]) * (2 * rx - 1.5 * rx * rx);
	else if ((rx >= 1) && (rx <= 2))
	  Dw[0] = .5 * wy * wz / (param_n[0] * param_n[0]) * (2 - rx) * (2 - rx);
	else
	  Dw[0] = 0.0;
	
	/* check y-direction */
	if ((ry >= -2) && (ry < -1))
	  Dw[1] = - .5 * wx * wz / (param_n[0] * param_n[0]) * (2 + ry) * (2 + ry);
	else if ((ry >= -1) && (ry < 0))
	  Dw[1] = wx * wz / (param_n[0] * param_n[0]) * (2 * ry + 1.5 * ry * ry);
	else if ((ry >= 0) && (ry < 1))
	  Dw[1] = wx * wz / (param_n[0] * param_n[0]) * (2 * ry - 1.5 * ry * ry);
	else if ((ry >= 1) && (ry <= 2))
	  Dw[1] = .5 * wx * wz / (param_n[0] * param_n[0]) * (2 - ry) * (2 - ry);
	else
	  Dw[1] = 0.0;

	/* check z-direction */
	if ((rz >= -2) && (rz < -1))
	  Dw[2] = - .5 * wx * wy / (param_n[0] * param_n[0]) * (2 + rz) * (2 + rz);
	else if ((rz >= -1) && (rz < 0))
	  Dw[2] = wx * wy / (param_n[0] * param_n[0]) * (2 * rz + 1.5 * rz * rz);
	else if ((rz >= 0) && (rz < 1))
	  Dw[2] = wx * wy / (param_n[0] * param_n[0]) * (2 * rz - 1.5 * rz * rz);
	else if ((rz >= 1) && (rz <= 2))
	  Dw[2] = .5 * wx * wy / (param_n[0] * param_n[0]) * (2 - rz) * (2 - rz);
	else
	  Dw[2] = 0.0;
      }
    }
    else
      throw eGeneralFail;
      
    return true;
  }
  return false;
}

/* multiple point calculations */
int CubicSplineWindowT::Window(const dArray2DT& x_n, const dArray2DT& param_n, const dArrayT& x,
		int order, dArrayT& w, dArray2DT& Dw, dArray2DT& DDw)
{
  /* compute window function and derivatives for multiple field points */

  /* allocate */
  int nsd = x.Length();
  fNSD.Allocate(nsd);
  fNSDsym.Allocate(nsd);

  /* work space */
  dArrayT x_node, param_node;
  int count = 0;    
  int numwindowpoints = x_n.MajorDim(); 

  for (int i = 0; i < numwindowpoints; i++)
  {
    /* collect nodal values */
    x_n.RowAlias(i, x_node);
    param_n.RowAlias(i, param_node);
      
    if (CubicSplineWindowT::Window(x_node, param_node, x, order, w[i], fNSD, fNSDsym))
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

bool CubicSplineWindowT::Covers(const dArrayT& x_n, const dArrayT& x, 
	const dArrayT& param_n) const
{
  double dist = dArrayT::Distance(x, x_n);

  /* check individual directions to see if outside the "box" */
  for (int i = 0; i < x.Length(); i++)
  {
    if (fabs(dist / param_n[0]) > 2.0)
      return false;
  }
  return true;
}

int CubicSplineWindowT::Covers(const dArray2DT& x_n, const dArrayT& x, 
	const dArray2DT& param_n, ArrayT<bool>& covers) const
{
  int count = 0;
  int numwindowpoints = x_n.MajorDim();        // Could be MajorDim!
  for (int i = 0; i < numwindowpoints; i++)
  {
    double dist = dArrayT::Distance(x, x_n);

    for (int j = 0; j < x.Length(); j++)
    {
      if (fabs(dist / param_n[i]) > 2.0)
		count++;
    }
    if (count == 0)
      covers[i] = true;
    else
      covers[i] = false;
  }
  return count;
}