/*
 * File: QuadT.cpp
 *
 */

/*
 * created      : PAK (07/03/96)
 * last modified: PAK (03/02/99)
 */

#include <math.h>

#include "ExceptionCodes.h"

#include "QuadT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "iArray2DT.h"
#include "dArray2DT.h"
#include "dMatrixT.h"

/* parameters */
const int kQuadnsd          = 2;
const int kNumVertexNodes	= 4;

const double sqrt3 = sqrt(3.0);

/* constructor */
QuadT::QuadT(int numnodes): GeometryBaseT(numnodes, kNumVertexNodes) { }

/* compute local shape functions and derivatives */
void QuadT::SetLocalShape(dArray2DT& Na, ArrayT<dArray2DT>& Na_x,
	dArrayT& weights)
{
	/* dimensions */
	int numnodes  = Na.MinorDim();
	int numint    = weights.Length();
	int nsd       = Na_x[0].MajorDim();

	/* dimension checks */
	if (numnodes < 4 || numnodes > 8) throw eGeneralFail;
	if (numint != 1 && 
	    numint != 4 && 
	    numint != 9 &&
	    numint != 16) throw eGeneralFail;
	if (nsd != kQuadnsd) throw eGeneralFail;

	/* initialize */
	Na = 0.0;
	for (int i = 0; i < Na_x.Length(); i++)
		Na_x[i] = 0.0;

	/* integration point coordinates */
	double	ra[9] = {-1.0, 1.0, 1.0,-1.0, 0.0, 1.0, 0.0,-1.0, 0.0};
	double  sa[9] = {-1.0,-1.0, 1.0, 1.0,-1.0, 0.0, 1.0, 0.0, 0.0};

	double xa_16[16], ya_16[16];
	double *xa, *ya;
	double 	g;
	
	/* integration weights */
	switch (numint)
	{
		case 1:	

			xa = ra;			  
			ya = sa;			  
    		g = 0.0;

    		weights[0] = 4.0;
    		break;
    
  		case 4:
  	
			xa = ra;			  
			ya = sa;			  
  			g = 1.0/sqrt3;
  			
       		weights[0] = 1.0;
      		weights[1] = 1.0;
      		weights[2] = 1.0;
      		weights[3] = 1.0;
			break;

		case 9:
		{
			xa = ra;			  
			ya = sa;			  
  			g = sqrt(3.0/5.0);
  			
  			double a = 25.0/81.0;
  			double b = 40.0/81.0;
  			double c = 64.0/81.0;
  			
       		weights[0] = a;
      		weights[1] = a;
      		weights[2] = a;
      		weights[3] = a;
       		weights[4] = b;
      		weights[5] = b;
      		weights[6] = b;
      		weights[7] = b;
      		weights[8] = c;
			break;
		}
		
		case 16:
		{
			/* coordinates */
			double b1 = sqrt((3.0 - 2.0*sqrt(6.0/5.0))/7.0);
			double b2 = sqrt((3.0 + 2.0*sqrt(6.0/5.0))/7.0);
			double b_1D[4] = {-b2,-b1, b1, b2};

			/* weights */					
			double w1 = (18.0 + sqrt(30.0))/36.0; 
			double w2 = (18.0 - sqrt(30.0))/36.0;
			double w_1D[4] = {w2, w1, w1, w2};

			int x_i = 0;
			int y_i = 0;
			for (int i = 0; i < 16; i++)
			{
				xa_16[i]   = b_1D[x_i];
				ya_16[i]   = b_1D[y_i];
				weights[i] = w_1D[x_i]*w_1D[y_i];
							
				if (++x_i == 4)
				{
					y_i++;
					x_i = 0;
				}
			}						
			
			xa = xa_16;
			ya = ya_16;
			g  = 1.0;		
			break;
		}	
		default:
		
			throw eGeneralFail;
	}
				
	/* corner nodes */
	for (int in = 0; in < numint; in++)	
	{
		double* na  = Na(in);
		double* nax = Na_x[in](0);
		double* nay = Na_x[in](1);

      	double r = g*xa[in];
      	double s = g*ya[in];
      	
		for (int lnd = 0; lnd < kNumVertexNodes; lnd++)	/* corner nodes */
		{
      		double tempr1 = 1.0 + ra[lnd]*r;
      		double temps1 = 1.0 + sa[lnd]*s;
      		
      		*na  = 0.25*tempr1*temps1;
      		*nax = 0.25*ra[lnd]*temps1;
      		*nay = 0.25*tempr1*sa[lnd];
      		
      		na++;
      		nax++;
      		nay++;
       	}
 	}
 	
 	/* mid-side nodes */
 	if (numnodes > kNumVertexNodes)
 	{
 		for (int in = 0; in < numint; in++)
 		{
			/* pointers to the first mid-side node */
			double* na  = Na(in)  + kNumVertexNodes;
			double* nax = Na_x[in](0) + kNumVertexNodes;
			double* nay = Na_x[in](1) + kNumVertexNodes;

 			double r = g*xa[in];
 			double s = g*ya[in];
	
 			for (int lnd = kNumVertexNodes; lnd < numnodes; lnd++)
 			{
 				int off1 =-kNumVertexNodes;
 				int off2 = off1 + 1;
 				if (lnd == 7) /* the 8th node is the 0th */
 					off2 =-7;
 				
 				double fac = 1.0;
 				if (lnd > 5) /* the 7th and 8th nodes */
 					fac = -1.0;	
 					 				
  				double tempr1 = 1.0 + r*sa[lnd]; 
 				double temps1 = 1.0 + s*ra[lnd];
 				double tempr2 = 1.0 + r*fac;
 				double temps2 = 1.0 - s*fac;
 				
 				double shape = 0.5*tempr1*tempr2*temps1*temps2;
 				double shapex, shapey;
 				
  				if (fmod(lnd+1,2) == 0) 
 				{
 					/* even numbered mid-side nodes */
 					shapex = 0.5*ra[lnd]*temps1*temps2;
 					shapey =-s*tempr2;
 				}
 				else
 				{
 					/* odd numbered mid-side nodes */
 					shapex =-r*temps2;
 					shapey = 0.5*sa[lnd]*tempr1*tempr2;
 				}
 				
 				/* set mid-side node functions */
 				*na  = shape;
 				*nax = shapex;
 				*nay = shapey;
 				
 				/* correct the corner node functions */
				shapex *= 0.5;
				shapey *= 0.5;
				shape  *= 0.5;

				*(na + off1)  -= shape;
 				*(na + off2)  -= shape;
 				*(nax + off1) -= shapex;
 				*(nax + off2) -= shapex;
 				*(nay + off1) -= shapey;
 				*(nay + off2) -= shapey;

				na++;
      			nax++;
      			nay++;
 			}
 		}
 	}	
}

/* set the values of the nodal extrapolation matrix */
void QuadT::SetExtrapolation(dMatrixT& extrap)
{
	/* dimensions */
	int numnodes = extrap.Rows();
	int numint   = extrap.Cols();

	/* dimension checks */
	if (numnodes < 4 || numnodes > 8) throw eGeneralFail;

	/* initialize */
	extrap = 0.0;
	
	switch (numint)
	{
		case 1:	
			  
    		extrap(0,0) = 1.0;
    		break;
    
  		case 4:
		{
			double a = 1.0 + sqrt3/2.0;
			double b =-1.0/2.0;
			double c = 1.0 - sqrt3/2.0;
			
			/* vertex nodes */
			extrap(0,0) = a;
			extrap(0,1) = b;
			extrap(0,2) = c;
			extrap(0,3) = b;
						
			extrap(1,0) = b;
			extrap(1,1) = a;
			extrap(1,2) = b;
			extrap(1,3) = c;
			
			extrap(2,0) = c;
			extrap(2,1) = b;
			extrap(2,2) = a;
			extrap(2,3) = b;
			
			extrap(3,0) = b;
			extrap(3,1) = c;
			extrap(3,2) = b;
			extrap(3,3) = a;
			
			/* midside nodes */
			if (numnodes > kNumVertexNodes)
			{
				double d = (1.0 + sqrt3)/4.0;
				double e = (1.0 - sqrt3)/4.0;
			
				double smooth[4][4] = {{d,d,e,e},
				                       {e,d,d,e},
				                       {e,e,d,d},
				                       {d,e,e,d}};
			
				for (int i = kNumVertexNodes; i < numnodes; i++)
					for (int j = 0; j < numint; j++)
						extrap(i,j) = smooth[i-kNumVertexNodes][j];
			}
			  	
			break;
		}
		
		case 9:
		{
			/* constants */
			double a = 5.0/18.0;
			double b = (20.0 + 5.0*sqrt(15.0))/18.0;
			double c = (20.0 - 5.0*sqrt(15.0))/18.0;
					
			double d = (-10.0 + 2.0*sqrt(15.0))/18.0;
			double e = (-10.0 - 2.0*sqrt(15.0))/18.0;		
			
			/* vertex nodes */
			extrap(0,0) = b;
			extrap(0,1) = a;
			extrap(0,2) = c;
			extrap(0,3) = a;
			extrap(0,4) = e;
			extrap(0,5) = d;
			extrap(0,6) = d;
			extrap(0,7) = e;
			extrap(0,8) = 4.0/9.0;

			extrap(1,0) = a;
			extrap(1,1) = b;
			extrap(1,2) = a;
			extrap(1,3) = c;
			extrap(1,4) = e;
			extrap(1,5) = e;
			extrap(1,6) = d;
			extrap(1,7) = d;
			extrap(1,8) = 4.0/9.0;

			extrap(2,0) = c;
			extrap(2,1) = a;
			extrap(2,2) = b;
			extrap(2,3) = a;
			extrap(2,4) = d;
			extrap(2,5) = e;
			extrap(2,6) = e;
			extrap(2,7) = d;
			extrap(2,8) = 4.0/9.0;

			extrap(3,0) = a;
			extrap(3,1) = c;
			extrap(3,2) = a;
			extrap(3,3) = b;
			extrap(3,4) = d;
			extrap(3,5) = d;
			extrap(3,6) = e;
			extrap(3,7) = e;
			extrap(3,8) = 4.0/9.0;
			
			/* midside nodes */
			if (numnodes > kNumVertexNodes)
			{
				double f = (5.0 + sqrt(15.0))/6.0;
				double g = (5.0 - sqrt(15.0))/6.0;
				double h =-2.0/3.0;
			
				double smooth[4][9] = {{0,0,0,0,f,0,g,0,h},
				                       {0,0,0,0,0,f,0,g,h},
				                       {0,0,0,0,g,0,f,0,h},
				                       {0,0,0,0,0,g,0,f,h}};
			
				for (int i = kNumVertexNodes; i < numnodes; i++)
					for (int j = 0; j < numint; j++)
						extrap(i,j) = smooth[i-kNumVertexNodes][j];
			}

			break;
		}

		case 16:
		{
			/* data for the smoothing  matrix */
			double smooth_16[8*16] = {
 2.3310819800373e+00, -1.7392742256873e-01,  1.2977127608750e-02, -1.7392742256873e-01, 
-1.4096315416317e-01,  1.0517587236621e-02,  1.0517587236621e-02, -1.4096315416317e-01, 
-1.2422443623633e+00,  9.2686727449598e-02, -4.5653628771612e-02,  6.1187793035203e-01, 
 7.5119916442126e-02, -6.7476185377616e-02, -3.7000947956309e-02,  9.0435721689180e-01, 
 6.1187793035203e-01, -4.5653628771610e-02,  9.2686727449599e-02, -1.2422443623634e+00, 
-3.7000947956310e-02, -6.7476185377616e-02,  7.5119916442126e-02,  9.0435721689180e-01, 
-1.7392742256873e-01,  1.2977127608750e-02, -1.7392742256873e-01,  2.3310819800373e+00, 
 1.0517587236621e-02,  1.0517587236621e-02, -1.4096315416317e-01, -1.4096315416317e-01, 
-1.2422443623634e+00,  6.1187793035203e-01, -4.5653628771615e-02,  9.2686727449597e-02, 
 9.0435721689180e-01, -3.7000947956310e-02, -6.7476185377616e-02,  7.5119916442126e-02, 
 6.6199776285810e-01, -3.2607257743128e-01,  1.6060979616250e-01, -3.2607257743128e-01, 
-4.8193614118559e-01,  2.3738170811213e-01,  2.3738170811213e-01, -4.8193614118559e-01, 
-3.2607257743127e-01,  1.6060979616250e-01, -3.2607257743127e-01,  6.6199776285809e-01, 
 2.3738170811213e-01,  2.3738170811213e-01, -4.8193614118559e-01, -4.8193614118559e-01, 
 9.2686727449598e-02, -4.5653628771611e-02,  6.1187793035203e-01, -1.2422443623633e+00, 
-6.7476185377616e-02, -3.7000947956309e-02,  9.0435721689180e-01,  7.5119916442126e-02, 
 6.1187793035203e-01, -1.2422443623634e+00,  9.2686727449600e-02, -4.5653628771612e-02, 
 9.0435721689180e-01,  7.5119916442126e-02, -6.7476185377617e-02, -3.7000947956310e-02, 
-3.2607257743128e-01,  6.6199776285809e-01, -3.2607257743127e-01,  1.6060979616251e-01, 
-4.8193614118559e-01, -4.8193614118559e-01,  2.3738170811214e-01,  2.3738170811214e-01, 
 1.6060979616250e-01, -3.2607257743127e-01,  6.6199776285810e-01, -3.2607257743127e-01, 
 2.3738170811214e-01, -4.8193614118559e-01, -4.8193614118559e-01,  2.3738170811214e-01, 
-4.5653628771611e-02,  9.2686727449597e-02, -1.2422443623634e+00,  6.1187793035203e-01, 
-6.7476185377616e-02,  7.5119916442125e-02,  9.0435721689180e-01, -3.7000947956310e-02, 
-1.7392742256873e-01,  2.3310819800373e+00, -1.7392742256873e-01,  1.2977127608749e-02, 
-1.4096315416317e-01, -1.4096315416317e-01,  1.0517587236621e-02,  1.0517587236621e-02, 
 9.2686727449598e-02, -1.2422443623633e+00,  6.1187793035203e-01, -4.5653628771610e-02, 
 7.5119916442126e-02,  9.0435721689180e-01, -3.7000947956308e-02, -6.7476185377616e-02, 
-4.5653628771610e-02,  6.1187793035203e-01, -1.2422443623634e+00,  9.2686727449597e-02, 
-3.7000947956310e-02,  9.0435721689180e-01,  7.5119916442125e-02, -6.7476185377616e-02, 
 1.2977127608750e-02, -1.7392742256873e-01,  2.3310819800373e+00, -1.7392742256873e-01, 
 1.0517587236621e-02, -1.4096315416317e-01, -1.4096315416317e-01,  1.0517587236621e-02};
		
				/* copy in */		
				dMatrixT smooth(8,16, smooth_16);
				for (int i = 0; i < numnodes; i++)
					for (int j = 0; j < numint; j++)
						extrap(i,j) = smooth(i,j);
		
			break;
		}
		default:
		
			cout << "\n QuadT::SetExtrapolation: no nodal extrapolation with Gauss rule: ";
			cout << numint << endl;			
	}
}

/* return the local node numbers for each facet of the element 
 * numbered to produce at outward normal in the order: vertex
 * nodes, mid-edge nodes, mid-face nodes */
void QuadT::NodesOnFacet(int facet, iArrayT& facetnodes) const
{
// TEMP: not implemented with midside nodes
	if (fNumNodes != 4 && fNumNodes != 8)
	{
		cout << "\n QuadT::NodesOnFacet: only implemented for 4 and 8 element nodes" << endl;
		throw eGeneralFail;
	}

#if __option(extended_errorcheck)
	if (facet < 0 || facet > 3) throw eOutOfRange;
#endif

	/* nodes-facet data */
	int dat4[] = {0,1,1,2,2,3,3,0};
	int dat8[] = {0,1,4,1,2,5,2,3,6,3,0,7};

	/* collect facet data */		
	iArrayT tmp;
	if (fNumNodes == 4)
		tmp.Set(2, dat4 + facet*2);
	else
		tmp.Set(3, dat8 + facet*3);
	
	/* (allocate and) copy in */
	facetnodes = tmp;
}

void QuadT::NumNodesOnFacets(iArrayT& num_nodes) const
{
// TEMP: not implemented with midside nodes
	if (fNumNodes != 4 && fNumNodes != 8)
	{
		cout << "\n QuadT::NumNodesOnFacets: only implemented for 4 and 8 element nodes" << endl;
		throw eGeneralFail;
	}

	num_nodes.Allocate(4);
	if (fNumNodes == 4)
		num_nodes = 2;
	else
		num_nodes = 3;
}

/* returns the nodes on each facet needed to determine neighbors
 * across facets */
void QuadT::NeighborNodeMap(iArray2DT& facetnodes) const
{
	int dat4[] = {0,1,1,2,2,3,3,0};
	iArray2DT temp(4, 2, dat4);

	facetnodes = temp;
}

/* return geometry and number of nodes on each facet */
void QuadT::FacetGeometry(ArrayT<GeometryCode>& facet_geom, iArrayT& facet_nodes) const
{
	facet_geom.Allocate(fNumFacets);
	facet_geom = kLine;
	
	facet_nodes.Allocate(fNumFacets);
	facet_nodes = 2;
	for (int i = 0; i < (fNumNodes - kNumVertexNodes); i++)
		facet_nodes[i] = 3;
}