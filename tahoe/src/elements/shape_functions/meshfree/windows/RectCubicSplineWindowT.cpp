#include "RectCubicSplineWindowT.h"
#include "ExceptionCodes.h"
#include <math.h>

const double sqrtPi = sqrt(acos(-1.0));
static double Max(double a, double b) { return (a > b) ? a : b; };

/* constructor */
RectCubicSplineWindowT::RectCubicSplineWindowT(const dArrayT& dilation_scaling, double sharpening_factor,
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
		throw eBadInputValue;
}

/* "synchronization" of nodal field parameters. */
void RectCubicSplineWindowT::SynchronizeSupportParameters(dArray2DT& params_1, 
	dArray2DT& params_2) const
{
	/* should be over the same global node set (marked by length) */
	if (params_1.Length() != params_2.Length() ||
	    params_1.MinorDim() != NumberOfSupportParameters() ||
	    params_2.MinorDim() != NumberOfSupportParameters())
	{
		cout << "\n RectCubicSplineWindowT::SynchronizeSupportParameters: nodal\n"
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
void RectCubicSplineWindowT::ModifySupportParameters(dArray2DT& nodal_params) const
{
	/* scale supports */
	for (int i = 0; i < fDilationScaling.Length(); i++)
		nodal_params.ScaleColumn(i, fDilationScaling[i]);	
}

void RectCubicSplineWindowT::WriteParameters(ostream& out) const
{
	/* window function parameters */
	out << " Dilation scaling factor . . . . . . . . . . . . :\n" << fDilationScaling << '\n';
	out << " Window function sharpening factor . . . . . . . = " << fSharpeningFactor << '\n';
	out << " Neighbor cutoff factor. . . . . . . . . . . . . = " << fCutOffFactor << '\n';
}

/* Single point evaluations */
bool RectCubicSplineWindowT::Window(const dArrayT& x_n, const dArrayT& param_n, const dArrayT& x,
		int order, double& w, dArrayT& Dw, dSymMatrixT& DDw)
{
  /* Compute window function and its derivatives - accomplish by scalar product of individual
   * window functions in x/y/z directions */

  if (!RectCubicSplineWindowT::Covers(x_n, x, param_n))
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
      const double rx = Dw[0] / param_n[0];
      const double ry = Dw[1] / param_n[1];

      /* check x-direction */
      if ((rx>=-2)&&(rx<-1)) 
	wx = 1.0 / (6.0 * param_n[0]) * (2.0 + rx) * (2.0 + rx) * (2.0 + rx);
      else if ((rx>=-1)&&(rx<0)) 
	wx = (2.0 / 3.0 - rx * rx - .5 * rx * rx * rx) / param_n[0];
      else if ((rx>=0)&&(rx<1)) 
	wx = (2.0 / 3.0 - rx * rx + .5 * rx * rx * rx) / param_n[0];
      else if ((rx>=1)&&(rx<=2)) 
	wx = 1.0 / (6.0 * param_n[0]) * (2.0 - rx) * (2.0 - rx) * (2.0 - rx);
      else
	wx = 0.0;

      /* check y-direction */
      if ((ry>=-2)&&(ry<-1))
	wy = 1.0 / (6.0 * param_n[1]) * (ry + 2.0) * (ry + 2.0) * (ry + 2.0);
      else if ((ry>=-1)&&(ry<0))
	wy = (2.0 / 3.0 - ry * ry - .5 * ry * ry * ry) / param_n[1];
      else if ((ry>=0)&&(ry<1))
	wy = (2.0 / 3.0 - ry * ry + .5 * ry * ry * ry) / param_n[1];
      else if ((ry>=1)&&(ry<=2))
	wy = 1.0 / (6.0 * param_n[1]) * (2.0 - ry) * (2.0 - ry) * (2.0 - ry);
      else
	wy = 0.0;

      w = wx * wy;

      if (order > 0)
      {
	if (order > 1)
	{
	  /* check x-direction */
	  if ((rx >= -2) && (rx < -1))
	    DDw[0] = wy / (param_n[0] * param_n[0] * param_n[0]) * (2.0 + rx);
	  else if ((rx >= -1) && (rx < 0))
	    DDw[0] = - wy / (param_n[0] * param_n[0] * param_n[0]) * (2.0 + 3.0 * rx);
	  else if ((rx >= 0) && (rx < 1))
	    DDw[0] = - wy / (param_n[0] * param_n[0] * param_n[0]) * (2.0 - 3.0 * rx);
	  else if ((rx >= 1) && (rx <= 2))
	    DDw[0] = wy / (param_n[0] * param_n[0] * param_n[0]) * (2.0 - rx);
	  else
	    DDw[0] = 0.0;
	  
	  /* check y-direction */
	  if ((ry >= -2) && (ry < -1))
	    DDw[1] = wx / (param_n[1] * param_n[1] * param_n[1]) * (2.0 + ry);
	  else if ((ry >= -1) && (ry < 0))
	    DDw[1] = - wx / (param_n[1] * param_n[1] * param_n[1]) * (2.0 + 3.0 * ry);
	  else if ((ry >= 0) && (ry < 1))
	    DDw[1] = - wx / (param_n[1] * param_n[1] * param_n[1]) * (2.0 - 3.0 * ry);
	  else if ((ry >= 1) && (ry <= 2))
	    DDw[1] = wx / (param_n[1] * param_n[1] * param_n[1]) * (2.0 - ry);
	  else
	    DDw[1] = 0.0;
	}

	/* check x-direction */
	if ((rx >= -2) && (rx < -1))
	  Dw[0] = - .5 * wy / (param_n[0] * param_n[0]) * (2.0 + rx) * (2.0 + rx);
	else if ((rx >= -1) && (rx < 0))
	  Dw[0] = wy / (param_n[0] * param_n[0]) * (2.0 * rx + 1.5 * rx * rx);
	else if ((rx >= 0) && (rx < 1))
	  Dw[0] = wy / (param_n[0] * param_n[0]) * (2.0 * rx - 1.5 * rx * rx);
	else if ((rx >= 1) && (rx <= 2))
	  Dw[0] = .5 * wy / (param_n[0] * param_n[0]) * (2.0 - rx) * (2.0 - rx);
	else
	  Dw[0] = 0.0;
	  
	  /* check y-direction */
	if ((ry >= -2) && (ry < -1))
	  Dw[1] = - .5 * wx / (param_n[1] * param_n[1]) * (2.0 + ry) * (2.0 + ry);
	else if ((ry >= -1) && (ry < 0))
	  Dw[1] = wx / (param_n[1] * param_n[1]) * (2.0 * ry + 1.5 * ry * ry);
	else if ((ry >= 0) && (ry < 1))
	  Dw[1] = wx / (param_n[1] * param_n[1]) * (2.0 * ry - 1.5 * ry * ry);
	else if ((ry >= 1) && (ry <= 2))
	  Dw[1] = .5 * wx / (param_n[1] * param_n[1]) * (2.0 - ry) * (2.0 - ry);
	else
	  Dw[1] = 0.0;

	cout << "Derivatives of window function are" << Dw << '\n';
      }
    }
    else if (x.Length() == 3)     // 3D calculation
    {
      double wx, wy, wz;      
      double rx = Dw[0] / param_n[0];
      double ry = Dw[1] / param_n[1];
      double rz = Dw[2] / param_n[2];
      
      /* check x-direction */
      if ((rx >= -2) && (rx < -1))
	wx = 1.0 / (6.0 * param_n[0]) * (2.0 + rx) * (2.0 + rx) * (2.0 + rx);
      else if ((rx >= -1) && (rx < 0))
	wx = (2.0 / 3.0 - rx * rx - .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 0) && (rx < 1))
	wx = (2.0 / 3.0 - rx * rx + .5 * rx * rx * rx) / param_n[0];
      else if ((rx >= 1) && (rx <= 2))
	wx = 1.0 / (6.0 * param_n[0]) * (2 - rx) * (2 - rx) * (2 - rx);
      else
	wx = 0.0;

      /* check y-direction */
      if ((ry >= -2) && (ry < -1))
	wy = 1.0 / (6.0 * param_n[1]) * (ry + 2.0) * (ry + 2.0) * (ry + 2.0);
      else if ((ry >= -1) && (ry < 0))
	wy = 1.0 / param_n[1] * (2.0 / 3.0 - ry * ry * (1.0 + .5 * ry));
      else if ((ry >= 0) && (ry < 1))
	wy = 1.0 / param_n[1] * (2.0 / 3.0 - ry * ry * (1.0 - .5 * ry));
      else if ((ry >= 1) && (ry <= 2))
	wy = 1.0 / (6.0 * param_n[1]) * (2.0 - ry) * (2.0 - ry) * (2.0 - ry);
      else
	wy = 0.0;

      /* check z-direction */
      if ((rz >= -2) && (rz < -1))
	wz = 1.0 / (6.0 * param_n[2]) * (rz + 2.0) * (rz + 2.0) * (rz + 2.0);
      else if ((rz >= -1) && (rz < 0))
	wz = 1.0 / param_n[2] * (2.0 / 3.0 - rz * rz * (1.0 + .5 * rz));
      else if ((rz >= 0) && (rz < 1))
	wz = 1.0 / param_n[2] * (2.0 / 3.0 - rz * rz * (1.0 - .5 * rz));
      else if ((rz >= 1) && (rz <= 2))
	wz = 1.0 / (6.0 * param_n[2]) * (2.0 - rz) * (2.0 - rz) * (2.0 - rz);
      else
	wz = 0.0;

      w = wx * wy * wz;
      if (order > 0)
      {
	if (order > 1)
	{
	  /* check x-direction */
	  if ((rx >= -2) && (rx < -1))
	    DDw[0] = wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2.0 + rx);
	  else if ((rx >= -1) && (rx < 0))
	    DDw[0] = - wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2.0 + 3.0 * rx);
	  else if ((rx >= 0) && (rx < 1))
	    DDw[0] = - wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2.0 - 3.0 * rx);
	  else if ((rx >= 1) && (rx <= 2))
	    DDw[0] = wy * wz / (param_n[0] * param_n[0] * param_n[0]) * (2.0 - rx);
	  else
	    DDw[0] = 0.0;
	  
	  /* check y-direction */
	  if ((ry >= -2) && (ry < -1))
	    DDw[1] = wx * wz / (param_n[1] * param_n[1] * param_n[1]) * (2.0 + ry);
	  else if ((ry >= -1) && (ry < 0))
	    DDw[1] = - wx * wz / (param_n[1] * param_n[1] * param_n[1]) * (2.0 + 3.0 * ry);
	  else if ((ry >= 0) && (ry < 1))
	    DDw[1] = - wx * wz / (param_n[1] * param_n[1] * param_n[1]) * (2.0 - 3.0 * ry);
	  else if ((ry >= 1) && (ry <= 2))
	    DDw[1] = wx * wz / (param_n[1] * param_n[1] * param_n[1]) * (2.0 - ry);
	  else
	    DDw[1] = 0.0;

	  /* check z-direction */
	  if ((rz >= -2) && (rz < -1))
	    DDw[2] = wx * wy / (param_n[2] * param_n[2] * param_n[2]) * (2.0 + rz);
	  else if ((rz >= -1) && (rz < 0))
	    DDw[2] = - wx * wy / (param_n[2] * param_n[2] * param_n[2]) * (2.0 + 3.0 * rz);
	  else if ((rz >= 0) && (rz < 1))
	    DDw[2] = - wx * wy / (param_n[2] * param_n[2] * param_n[2]) * (2.0 - 3.0 * rz);
	  else if ((rz >= 1) && (rz <= 2))
	    DDw[2] = wx * wy / (param_n[2] * param_n[2] * param_n[2]) * (2.0 - rz);
	  else
	    DDw[2] = 0.0;
	}

	/* check x-direction */
	if ((rx >= -2) && (rx < -1))
	  Dw[0] = - .5 * wy * wz / (param_n[0] * param_n[0]) * (2.0 + rx) * (2.0 + rx);
	else if ((rx >= -1) && (rx < 0))
	  Dw[0] = wy * wz / (param_n[0] * param_n[0]) * (2.0 * rx + 1.5 * rx * rx);
	else if ((rx >= 0) && (rx < 1))
	  Dw[0] = wy * wz / (param_n[0] * param_n[0]) * (2.0 * rx - 1.5 * rx * rx);
	else if ((rx >= 1) && (rx <= 2))
	  Dw[0] = .5 * wy * wz / (param_n[0] * param_n[0]) * (2.0 - rx) * (2.0 - rx);
	else
	  Dw[0] = 0.0;
	
	/* check y-direction */
	if ((ry >= -2) && (ry < -1))
	  Dw[1] = - .5 * wx * wz / (param_n[1] * param_n[1]) * (2.0 + ry) * (2.0 + ry);
	else if ((ry >= -1) && (ry < 0))
	  Dw[1] = wx * wz / (param_n[1] * param_n[1]) * (2.0 * ry + 1.5 * ry * ry);
	else if ((ry >= 0) && (ry < 1))
	  Dw[1] = wx * wz / (param_n[1] * param_n[1]) * (2.0 * ry - 1.5 * ry * ry);
	else if ((ry >= 1) && (ry <= 2))
	  Dw[1] = .5 * wx * wz / (param_n[1] * param_n[1]) * (2.0 - ry) * (2.0 - ry);
	else
	  Dw[1] = 0.0;

	/* check z-direction */
	if ((rz >= -2) && (rz < -1))
	  Dw[2] = - .5 * wx * wy / (param_n[2] * param_n[2]) * (2.0 + rz) * (2.0 + rz);
	else if ((rz >= -1) && (rz < 0))
	  Dw[2] = wx * wy / (param_n[2] * param_n[2]) * (2.0 * rz + 1.5 * rz * rz);
	else if ((rz >= 0) && (rz < 1))
	  Dw[2] = wx * wy / (param_n[2] * param_n[2]) * (2.0 * rz - 1.5 * rz * rz);
	else if ((rz >= 1) && (rz <= 2))
	  Dw[2] = .5 * wx * wy / (param_n[2] * param_n[2]) * (2.0 - rz) * (2.0 - rz);
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
int RectCubicSplineWindowT::Window(const dArray2DT& x_n, const dArray2DT& param_n, const dArrayT& x,
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
      
    if (RectCubicSplineWindowT::Window(x_node, param_node, x, order, w[i], fNSD, fNSDsym))
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

bool RectCubicSplineWindowT::Covers(const dArrayT& x_n, const dArrayT& x, 
	const dArrayT& param_n) const
{
  dArrayT dx(x.Length());
  dx.DiffOf(x, x_n);

  /* check individual directions to see if outside the "box" */
  for (int i = 0; i < x.Length(); i++)
  {
    if (fabs(dx[i] / param_n[i]) > 2.0)
      return false;
  }
  return true;
}

int RectCubicSplineWindowT::Covers(const dArray2DT& x_n, const dArrayT& x, 
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
      if (fabs(dx[j] / param_n(i,j)) > 2.0)
		count++;
    }
    if (count == 0)
      covers[i] = true;
    else
      covers[i] = false;
  }
  return count;
}
