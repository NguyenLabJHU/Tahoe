/* $Id: ParentDomainT.cpp,v 1.14 2002-08-22 17:21:21 hspark Exp $ */
/* created: paklein (07/03/1996) */

#include "ParentDomainT.h"
#include "dArray2DT.h"
#include "LocalArrayT.h"

/* parent domain geometries */
#include "LineT.h"
#include "QuadT.h"
#include "TriT.h"
#include "HexahedronT.h"
#include "TetrahedronT.h"

/* vector functions */

using namespace Tahoe;

inline static void CrossProduct(const double* A, const double* B, double* AxB)
{   AxB[0] = A[1]*B[2] - A[2]*B[1];
	AxB[1] = A[2]*B[0] - A[0]*B[2];
	AxB[2] = A[0]*B[1] - A[1]*B[0];
};

/* constructor */
ParentDomainT::ParentDomainT(GeometryT::CodeT geometry_code, int numIP, int numnodes):
	fGeometryCode(geometry_code),
	fNumSD(GeometryT::GeometryToNumSD(fGeometryCode)),
	fNumIP(numIP),
	fNumNodes(numnodes),
	fNa(fNumIP,fNumNodes),
	fDNa(fNumIP),
	fWeights(fNumIP),
	fNodalExtrap(fNumNodes,fNumIP),
	fJacobian(fNumSD)
{
	/* memory for the derivatives */
	for (int i = 0; i < fDNa.Length(); i++)
		fDNa[i].Allocate(fNumSD, fNumNodes);
		
	/* initialize parent domain geometry */		
	switch (fGeometryCode)
	{
		case GeometryT::kLine:		
			fGeometry = new LineT(fNumNodes);
			break;
	
		case GeometryT::kQuadrilateral:
			fGeometry = new QuadT(fNumNodes);
			break;
		
		case GeometryT::kTriangle:
			fGeometry = new TriT(fNumNodes);
			break;

		case GeometryT::kHexahedron:
			fGeometry = new HexahedronT(fNumNodes);
			break;

		case GeometryT::kTetrahedron:
			fGeometry = new TetrahedronT(fNumNodes);
			break;

		default:
			throw eGeneralFail;			
	}
	if (!fGeometry) throw eOutOfMemory;
}

/* destructor */
ParentDomainT::~ParentDomainT(void) { delete fGeometry; }

/* set all local parameters */
void ParentDomainT::Initialize(void)
{
	/* local shape functions and derivatives */
	fGeometry->SetLocalShape(fNa, fDNa, fWeights);

	/* nodal extrapolation matrix */
	fGeometry->SetExtrapolation(fNodalExtrap);
}

/* interpolation to the current integration point */
void ParentDomainT::Interpolate(const LocalArrayT& nodal, dArrayT& interp,
int IPnum) const
{
#if __option(extended_errorcheck)
	if (nodal.MinorDim() != interp.Length() ||
	    nodal.NumberOfNodes() != fNumNodes) throw eSizeMismatch;
#endif

	int num_u = nodal.MinorDim();
	for (int i = 0; i < num_u; i++)
		interp[i] = fNa.DotRow(IPnum, nodal(i));
}

/* interpolate all to integration points: (nip x nu) */
void ParentDomainT::Interpolate(const LocalArrayT& nodal,
	dArray2DT& interp) const
{
#if __option(extended_errorcheck)
	if (interp.MinorDim() != nodal.MinorDim() ||
	    interp.MajorDim() != fNumIP           ||
	    nodal.NumberOfNodes() != fNumNodes) throw eSizeMismatch;
#endif

	int num_u = nodal.MinorDim();
	
	for (int j = 0; j < fNumIP; j++)
		for (int i = 0; i < num_u; i++)
			interp(j,i) = fNa.DotRow(j, nodal(i));

}

/* returns jacobian of the nodal values with respect
* to the variables of the shape function derivatives */
void ParentDomainT::Jacobian(const LocalArrayT& nodal, const dArray2DT& DNa,
	dMatrixT& jac) const
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (DNa.MinorDim() != nodal.NumberOfNodes() ||
DNa.MajorDim() != jac.Cols()            ||
jac.Rows() != nodal.MinorDim()) throw eSizeMismatch;
#endif

	double *pjac = jac.Pointer();
	double *pval = nodal.Pointer();
	
	int nnd   = nodal.NumberOfNodes();
	int num_u = jac.Rows();
	int num_d = jac.Cols();
	
	if (num_d == 2 && num_u == 2)
	{
		double& j11 = *pjac++;
		double& j21 = *pjac++;
		double& j12 = *pjac++;
		double& j22 = *pjac;
	
		j11 = j21 = j12 = j22 = 0.0;

		double* pu1 = nodal(0);
		double* pu2 = nodal(1);
		double* dx1 = DNa(0);
		double* dx2 = DNa(1);

		for (int i = 0; i < nnd; i++)
		{
			j11 += (*pu1)*(*dx1);
			j21 += (*pu2)*(*dx1);
			j12 += (*pu1)*(*dx2);
			j22 += (*pu2)*(*dx2);
			
			pu1++; pu2++; dx1++; dx2++;	
		}
	}
	else if (num_d == 3 && num_u == 3)
	{
		double& j11 = *pjac++;
		double& j21 = *pjac++;
		double& j31 = *pjac++;
		double& j12 = *pjac++;
		double& j22 = *pjac++;
		double& j32 = *pjac++;
		double& j13 = *pjac++;
		double& j23 = *pjac++;
		double& j33 = *pjac  ;
	
		j11 = j21 = j31 = j12 = j22 = j32 = j13 = j23 = j33 = 0.0;

		double* pu1 = nodal(0);
		double* pu2 = nodal(1);
		double* pu3 = nodal(2);
		double* dx1 = DNa(0);
		double* dx2 = DNa(1);
		double* dx3 = DNa(2);

		for (int i = 0; i < nnd; i++)
		{
			j11 += (*pu1)*(*dx1);
			j21 += (*pu2)*(*dx1);
			j31 += (*pu3)*(*dx1);
			j12 += (*pu1)*(*dx2);
			j22 += (*pu2)*(*dx2);
			j32 += (*pu3)*(*dx2);
			j13 += (*pu1)*(*dx3);
			j23 += (*pu2)*(*dx3);
			j33 += (*pu3)*(*dx3);
			
			pu1++; pu2++; pu3++; dx1++; dx2++; dx3++;	
		}
	}
	else
	{
		int j_inc = jac.Rows();
		for (int i = 0; i < num_u; i++)
		{
			double *pjac_j = pjac;
			for (int j = 0; j < num_d; j++)
			{
				*pjac_j = DNa.DotRow(j,pval);
				pjac_j += j_inc;
			}
			pjac++;
			pval += nnd;
		}
	}
}

//---------------------------------------------------------------------------
/* returns curl of a Vector T. Each of the dArrayT's are T at a given node */
void ParentDomainT::Curl(const ArrayT<dArrayT>& T, const dArray2DT& DNa, dArrayT& curl) const
{
  #if __option(extended_errorcheck)
  /* dimension check */
  if (curl.Length() != 3) {
    cout << "..ERROR >>  ParentDomainT::Curl : curl vector must be of size 3 \n";
    throw eSizeMismatch;
  }
  #endif
	double *pcurl = curl.Pointer();

	int nnd   = T.Length(); 

		double& c1 = *pcurl++;
		double& c2 = *pcurl++;
		double& c3 = *pcurl  ;
	
		c1 = c2 = c3 = 0.0;

		double* dx1 = DNa(0);
		double* dx2 = DNa(1);
		double* dx3;

		if (DNa.MajorDim() == 3) { // 3D Problem
		  dx3 = DNa(2);
		}
		else if (DNa.MajorDim() == 2) { // 2D Problem
                 dArrayT zero(nnd);
		 zero = 0.0;
		 dx3 = zero.Pointer(); 
		}
		else {
                  cout << "..ERROR >>  ParentDomainT::Curl : DNa.MajorDim() = "
		       << DNa.MajorDim() << " This != 2 or 3 \n";
		  throw eSizeMismatch;
		}

		double *pT;

		for (int i = 0; i < nnd; i++) {
	
		  pT  = T[i].Pointer();

		  double& T1 = *pT++;
		  double& T2 = *pT++;
		  double& T3 = *pT;

		  c1 +=  T3*(*dx2) - T2*(*dx3) ;
		  c2 +=  T1*(*dx3) - T3*(*dx1) ;
		  c3 +=  T2*(*dx1) - T1*(*dx2) ;

		  dx1++; dx2++; dx3++;	
		}

}

//---------------------------------------------------------------------------
/* returns curl of a Tensor T. Each of the dMatrixT's are T at a given node */
void ParentDomainT::Curl(const ArrayT<dMatrixT>& T, const dArray2DT& DNa, dMatrixT& curl) const
{
  #if __option(extended_errorcheck)
  /* dimension check */
  if (curl.Rows() != 3  || curl.Cols() != 3) {
    cout << "..ERROR >>  ParentDomainT::Curl : curl_T must be 3x3 \n";
    throw eSizeMismatch;
  }
  #endif
	double *pcurl = curl.Pointer();

	int nnd   = T.Length(); 

		double& c11 = *pcurl++;
		double& c21 = *pcurl++;
		double& c31 = *pcurl++;
		double& c12 = *pcurl++;
		double& c22 = *pcurl++;
		double& c32 = *pcurl++;
		double& c13 = *pcurl++;
		double& c23 = *pcurl++;
		double& c33 = *pcurl  ;
	
		c11 = c21 = c31 = c12 = c22 = c32 = c13 = c23 = c33 = 0.0;

		double* dx1 = DNa(0);
		double* dx2 = DNa(1);
		double* dx3;

		if (DNa.MajorDim() == 3) { // 3D Problem
		  dx3 = DNa(2);
		}
		else if (DNa.MajorDim() == 2) { // 2D Problem
                 dArrayT zero(nnd);
		 zero = 0.0;
		 dx3 = zero.Pointer(); 
		}
		else {
                  cout << "..ERROR >>  ParentDomainT::Curl : DNa.MajorDim() = "
		       << DNa.MajorDim() << " This != 2 or 3 \n";
		  throw eSizeMismatch;
		}

		double *pT;

		for (int i = 0; i < nnd; i++) {
	
		  pT  = T[i].Pointer();

		  double& T11 = *pT++;
		  double& T21 = *pT++;
		  double& T31 = *pT++;
		  double& T12 = *pT++;
		  double& T22 = *pT++;
		  double& T32 = *pT++;
		  double& T13 = *pT++;
		  double& T23 = *pT++;
		  double& T33 = *pT  ;

		  c11 += ( T12*(*dx3) - T13*(*dx2) );
		  c21 += ( T22*(*dx3) - T23*(*dx2) );
		  c31 += ( T32*(*dx3) - T33*(*dx2) );

		  c12 += ( T13*(*dx1) - T11*(*dx3) );
		  c22 += ( T23*(*dx1) - T21*(*dx3) );
		  c32 += ( T33*(*dx1) - T31*(*dx3) );

		  c13 += ( T11*(*dx2) - T12*(*dx1) );
		  c23 += ( T21*(*dx2) - T22*(*dx1) );
		  c33 += ( T31*(*dx2) - T32*(*dx1) );
			
		  dx1++; dx2++; dx3++;	
		}

}


//---------------------------------------------------------------------------
/* jacobian of surface mapping */
double ParentDomainT::SurfaceJacobian(const dMatrixT& jacobian) const
{
#if __option(extended_errorcheck)
	if (jacobian.Rows() != jacobian.Cols() + 1) throw eGeneralFail;
	if (fNumSD != 1 &&
	    fNumSD != 2) throw eGeneralFail;
#endif

	if (fNumSD == 1)
	{
		double* n = jacobian.Pointer();
		return sqrt(n[0]*n[0] + n[1]*n[1]);
	}
	else
	{
		double n[3];
		CrossProduct(jacobian(0), jacobian(1), n);
		return sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
	}
}

/* returns jacobian of the nodal values with respect
* to the variables of the shape function derivatives.
* Q returns as the transformation from global to local(')
* coordinates, i.e., t'_i = Q_ki t_k, where t'_j (j = nsd)
* is the "normal" direction */
double ParentDomainT::SurfaceJacobian(const dMatrixT& jacobian, dMatrixT& Q) const
{
#if __option(extended_errorcheck)
	if (jacobian.Rows() != jacobian.Cols() + 1) throw eGeneralFail;
	if (fNumSD != 1 &&
	    fNumSD != 2) throw eGeneralFail;
	if (Q.Rows() != fNumSD + 1 ||
	    Q.Cols() != fNumSD + 1) throw eSizeMismatch;
#endif

	/* surface dimension */
	if (fNumSD == 1)
	{
		double* t = jacobian.Pointer();
		double  j = sqrt(t[0]*t[0] + t[1]*t[1]);

		/* check */
		if (j <= 0.0) throw eBadJacobianDet;

		/* column vectors */
		double* n1 = Q(0);
		double* n2 = Q(1);
		n1[0] = t[0]/j; // n1: tangent
		n1[1] = t[1]/j;

		n2[0] =-n1[1];  // n2: normal
		n2[1] = n1[0];
		
		return j;
	}
	else
	{
		/* column vectors */
		double* n1 = Q(0);
		double* n2 = Q(1);
		double* n3 = Q(2);
		
		double* m1 = jacobian(0);
		double* m2 = jacobian(1);
		CrossProduct(m1, m2, n3);
		
		double jn = sqrt(n3[0]*n3[0] + n3[1]*n3[1] + n3[2]*n3[2]);
		double j1 = sqrt(m1[0]*m1[0] + m1[1]*m1[1] + m1[2]*m1[2]);

		/* normalize */
		if (jn <= 0.0) throw eBadJacobianDet;
		n3[0] /= jn;
		n3[1] /= jn;
		n3[2] /= jn;
		
		if (j1 <= 0.0) throw eBadJacobianDet;
		n1[0] = m1[0]/j1;
		n1[1] = m1[1]/j1;
		n1[2] = m1[2]/j1;
		
		/* orthonormal, in-plane */
		CrossProduct(n3, n1, n2);
		return jn;
	}
}

/* chain rule jacobian of shapefunctions wrt coordinates that
* are passed in, for all integration points at once */
void ParentDomainT::ComputeDNa(const LocalArrayT& coords,
	ArrayT<dArray2DT>& DNa, dArrayT& det)
{
	/* loop over integration points */
int numIP = fDNa.Length();
	for (int i = 0; i < numIP; i++)	
	{
		/* calculate the Jacobian matrix */
		Jacobian(coords, fDNa[i], fJacobian);
		det[i] = fJacobian.Det();
		
		/* element check */
		if (det[i] <= 0.0) throw eBadJacobianDet;

		dMatrixT& jac_inv = fJacobian.Inverse();
					
		/* calculate the global shape function derivatives */
		if (fNumSD == 2)
		{
			double* pLNax = fDNa[i](0);
			double* pLNay = fDNa[i](1);
			
			double* pNax = DNa[i](0);
			double* pNay = DNa[i](1);
			
			double* pj = jac_inv.Pointer();
		
			for (int j = 0; j < fNumNodes; j++)
			{
				*pNax++ = pj[0]*(*pLNax) + pj[1]*(*pLNay);
				*pNay++ = pj[2]*(*pLNax) + pj[3]*(*pLNay);

				pLNax++;
				pLNay++;
			}
		}
		else if (fNumSD == 3)
		{
			double* pLNax = fDNa[i](0);
			double* pLNay = fDNa[i](1);
			double* pLNaz = fDNa[i](2);
			
			double* pNax = DNa[i](0);
			double* pNay = DNa[i](1);
			double* pNaz = DNa[i](2);
			
			double* pj = jac_inv.Pointer();
		
			for (int j = 0; j < fNumNodes; j++)
			{
				*pNax++ = pj[0]*(*pLNax) + pj[1]*(*pLNay) + pj[2]*(*pLNaz);
				*pNay++ = pj[3]*(*pLNax) + pj[4]*(*pLNay) + pj[5]*(*pLNaz);
				*pNaz++ = pj[6]*(*pLNax) + pj[7]*(*pLNay) + pj[8]*(*pLNaz);
				
				pLNax++;
				pLNay++;
				pLNaz++;
			}
		}
		else
		{
			const dArray2DT& LNax = fDNa[i];
			dArray2DT&        Nax = DNa[i];
			
			Nax = 0.0;
			for (int l = 0; l < fNumSD; l++)
				for (int k = 0; k < fNumSD; k++)
					for (int j = 0; j < fNumNodes; j++)
						Nax(l,j) += jac_inv(k,l)*LNax(k,j);
		}		
	}
}

/* compute nodal values:
*
* ipvalues[numvals] : field values from a single integration pt
* nodalvalues[fNumNodes x numvals] : extrapolated values */
void ParentDomainT::NodalValues(const dArrayT& IPvalues,
	dArray2DT& nodalvalues, int IPnum) const
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (nodalvalues.MajorDim() != fNumNodes ||
		nodalvalues.MinorDim() != IPvalues.Length()) throw eSizeMismatch;
#endif

	int numvals = IPvalues.Length();
	int numIP   = fNodalExtrap.Cols();

	/* single integration point */
	if (numIP == 1)
	{
		double* pip = IPvalues.Pointer();
		double* pnv = nodalvalues.Pointer();
	
		for (int i = 0; i < fNumNodes; i++)
		{									
			double* prep = pip;
		
			/* just overwrite */
			for (int j = 0; j < numvals; j++)
				*pnv++ = *prep++;
		}
	}
/* more than 1 integration point */
	else
	{	
		double* psmooth = fNodalExtrap(IPnum);
		double* pnv     = nodalvalues.Pointer();
		double* pip     = IPvalues.Pointer();
		
		for (int i = 0; i < fNumNodes; i++)
		{
			double* prep = pip;
		
			for (int j = 0; j < numvals; j++)
				*pnv++ += (*psmooth)*(*prep++);
				
			psmooth++;
		}
	}
}	

/* print the shape function values to the output stream */
void ParentDomainT::Print(ostream& out) const
{
	out << "\n Parent domain shape functions:\n";
	fNa.WriteNumbered(out);

	out << "\n Parent domain shape function derivatives:\n";
	for (int i = 0; i < fDNa.Length(); i++)
		fDNa[i].WriteNumbered(out);
}

/* return true if the given point is within the domain */
bool ParentDomainT::PointInDomain(const LocalArrayT& coords, const dArrayT& point) const
{
  int x, y;
  int dim = point.Length();
  int numpoints = coords.Length();
  if (dim == 1) /* 1D case */
  {
    dArrayT temp(2);
    for (int i = 0; i < 2; i++) 
      temp[i]=coords[i];

    temp.SortAscending();
    /* if atom and node coincide, return true */
    if (point[0] >= temp[0] && point[0] <= temp[1])
      return true;
    else
      return false;
  }
  else if (dim == 2)
  {
    /* currently assuming square/rectangular elements */
    if (point[0] >= coords(0,0) && point[0] <= coords(1,0))
      x = 1;
    else
      x = 0;
    if (point[1] >= coords(1,1) && point[1] <= coords(2,1))
      y = 1;
    else
      y = 0;
    if (x == 1 && y == 1)
      return true;
    else
      return false;
  }
  else if (dim == 3)
    int blah2=0;
  else
    int blah3=0;
#pragma unused(coords)
#pragma unused(point)
  return false;
}

/* map domain coordinates into the parent coordinates */
bool ParentDomainT::MapToParentDomain(const LocalArrayT& coords, const dArrayT& point,
	dArrayT& mapped) const
{
  int dim = point.Length();
  int numpoints = coords.Length();
  if (dim == 1) 
    {
      dArrayT temp(2);
      for (int i = 0; i < 2; i++)
	temp[i]=coords(i,0);

      temp.SortAscending();
      mapped[0]=2*point[0]/(temp[1]-temp[0])-(temp[0]+temp[1])/(temp[1]-temp[0]);
      /* mapped[0] = value in psi space, assuming parent coordinates go from -1 to 1 */
    }
  if (dim == 2)
    {
      /* solve iteratively via Newton method */
      dArrayT Na(NumNodes()), Res(NumSD()), Current(NumSD()), Temp(NumSD());
      Res = 0.0, Current = 0.0;
      dArray2DT DNa(NumSD(), NumNodes());
      dMatrixT Jacob(NumSD()), Jacobinv(NumSD());
      EvaluateShapeFunctions(Current, Na, DNa);
      Jacobian(coords, DNa, Jacob);
      Jacobinv.Inverse(Jacob);
      
      /* compute initial residual */
      for (int i = 0; i < NumNodes(); i++)
	{
	  Res[0] += Na[i] * coords(i,0);
	  Res[1] += Na[i] * coords(i,1);
	}
      Res[0] -= point[0];
      Res[1] -= point[1];
      double magres0 = Res.Magnitude();
      /* perform Newton iterations */
      if (magres0 < 10e-14) // if initial guess is exact, so don't divide by 0
	  mapped = Current;
      else
	{
	  int count = 0;
	  while (count++ < 15 && Res.Magnitude()/magres0 > 10e-14)
	    {
	      Res *= -1.0;
	      Jacobinv.Multx(Res,Temp);
	      Current += Temp;
	      EvaluateShapeFunctions(Current, Na);
	      Res = 0.0;
	      for (int i = 0; i < NumNodes(); i++)
		{
		  Res[0] += Na[i] * coords(i,0);
		  Res[1] += Na[i] * coords(i,1);
		}
	      Res[0] -= point[0];
	      Res[1] -= point[1];
	    }
	  mapped = Current;
	}
    }
#pragma unused(coords)
#pragma unused(point)
#pragma unused(mapped)
  return true;
}

/* calculate a characteristic domain size */
double ParentDomainT::AverageRadius(const LocalArrayT& coords, dArrayT& avg) const
{
	/* coordinate averages */
	coords.Average(avg);
	/* find max distance to centroid */
	double radius = 0;
	for (int i = 0; i < coords.NumberOfNodes(); i++) {
		double dist = 0;
		for (int j = 0; j < coords.MinorDim(); j++) {
			double dx = coords(i,j) - avg[j];
			dist += dx*dx;
		}	
		radius = (dist > radius) ? dist : radius;
	}
	return sqrt(radius); // returns largest characteristic domain size
}
