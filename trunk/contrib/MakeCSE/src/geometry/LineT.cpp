/*
 * File: LineT.cpp
 */

/*
 * created      : PAK (04/25/99)
 * last modified: PAK (05/06/99)
 */

#include <math.h>

#include "ExceptionCodes.h"

#include "LineT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "iArray2DT.h"
#include "dMatrixT.h"

/* parameters */
const int kLinensd = 1;
const int kNumVertexNodes = 2;

const double sqrt3 = sqrt(3.0);

/* constructor */
LineT::LineT(int numnodes): GeometryBaseT(numnodes, kNumVertexNodes) { }

/* compute local shape functions and derivatives */
void LineT::SetLocalShape(dArray2DT& Na, ArrayT<dArray2DT>& Na_x,
	dArrayT& weights)
{
	/* dimensions */
	int nnd = Na.MinorDim();
	int nip = weights.Length();
	int nsd = Na_x[0].MajorDim();

	/* dimension checks */
	if (nsd != kLinensd) throw eGeneralFail;

	/* initialize */
	Na = 0.0;
	for (int i = 0; i < nip; i++)
		Na_x[i] = 0.0;

	/* 1 point */
	double r1[1] = {0.0};
	
	/* 2 point */
	double x2    = sqrt(1.0/3.0);
    double r2[2] = {-x2, x2};

	/* 3 point */
	double    x3 = sqrt(3.0/5.0);
    double r3[3] = {-x3, 0.0, x3};

	/* 4 point */
	double x41 = sqrt((3.0 - 2.0*sqrt(6.0/5.0))/7.0);
	double x42 = sqrt((3.0 + 2.0*sqrt(6.0/5.0))/7.0);
	double r4[4] = {-x42,-x41, x41, x42};
	
	/* integration coordinates and weights */
	double* xa;
	switch (nip)
	{
		case 1:
		{
			xa = r1;			  
    		weights[0] = 2.0;
    		break;
		}
		case 2:  	
		{
			xa = r2;			   			
       		weights[0] = 1.0;
      		weights[1] = 1.0;
			break;
		}
		case 3:
		{
			xa = r3;
  			double a = 5.0/9.0;
  			double b = 8.0/9.0;  			
       		weights[0] = a;
      		weights[1] = b;
      		weights[2] = a;
			break;
		}
		case 4:
		{
			xa = r4;			    			

			double w1 = (18.0 + sqrt(30.0))/36.0; 
			double w2 = (18.0 - sqrt(30.0))/36.0;
       		weights[0] = w2;
      		weights[1] = w1;
      		weights[2] = w1;
      		weights[3] = w2;
			break;
		}	
		default:		
			
			cout << "\n LineT::SetLocalShape: unsupported number of integration points: " << nip << endl;
			throw eGeneralFail;
	}

	/* set shape functions and derivatives */
	switch (nnd)
	{
		case 2:  	
		{
			for (int i = 0; i < nip; i++)	
			{
				double* na  = Na(i);
				double* nax = Na_x[i](0);

		    	/* Na */
		    	na[0] = 0.5*(1.0 - xa[i]);
		    	na[1] = 0.5*(1.0 + xa[i]);
		
		        /* Na,x */
		    	nax[0] =-0.5;
		    	nax[1] = 0.5;
		    }
			break;
		}
		case 3:
		{
			for (int i = 0; i < nip; i++)	
			{
				double* na  = Na(i);
				double* nax = Na_x[i](0);

		    	/* Na */
		    	na[0] =-xa[i]*0.5*(1.0 - xa[i]);
		    	na[1] = xa[i]*0.5*(1.0 + xa[i]);
		    	na[2] = (1.0 - xa[i])*(1.0 + xa[i]);
		
		        /* Na,x */
		    	nax[0] =-0.5 + xa[i];
		    	nax[1] = 0.5 + xa[i];
		    	nax[2] =-2.0*xa[i];
		    }
			break;
		}
		default:
		
			cout << "\n LineT::SetLocalShape: unsupported number of nodes: " << nnd << endl;
			throw eGeneralFail;
	}
}

/* set the values of the nodal extrapolation matrix */
void LineT::SetExtrapolation(dMatrixT& extrap)
{
	/* dimensions */
	int nnd = extrap.Rows();
	int nip = extrap.Cols();

	/* dimension checks */
	if (nnd != 2 && 
	    nnd != 3) throw eGeneralFail;

	/* initialize */
	extrap = 0.0;
	
	switch (nip)
	{
		case 1:	
			  
    		extrap(0,0) = 1.0;
    		break;

		case 2:	
		{
			double dat_2[3*2] = {
                1.36602540378,
               -0.366025403784,
                0.5,
               -0.366025403784,
                1.36602540378,
                0.5};
			dMatrixT extrap_2(3, 2, dat_2);
			extrap_2.CopyBlock(0, 0, extrap);
    		break;
		}
		case 3:	
		{
			double dat_3[3*3] = {
                1.4788305577,
                0.187836108965,
                0.0,
               -0.666666666667,
               -0.666666666667,
                1.0,
                0.187836108965,
                1.4788305577,
                0.0};
			dMatrixT extrap_3(3, 3, dat_3);
			extrap_3.CopyBlock(0, 0, extrap);
    		break;
		}
		case 4:	
		{
			double dat_4[3*4] = {
                1.52678812546,
               -0.113917196282,
               -0.0923265984407,
               -0.813632449487,
                0.400761520312,
                0.592326598441,
                0.400761520312,
               -0.813632449487,
                0.592326598441,
               -0.113917196282,
                1.52678812546,
               -0.0923265984407};
			dMatrixT extrap_4(3, 4, dat_4);
			extrap_4.CopyBlock(0, 0, extrap);
    		break;
		}
    	default:		
			
			cout << "\n LineT::SetExtrapolation: unsupported number of integration points: " << nip << endl;
			throw eGeneralFail;
	}
}

/* return the local node numbers for each facet of the element 
 * numbered to produce at outward normal in the order: vertex
 * nodes, mid-edge nodes, mid-face nodes */
void LineT::NodesOnFacet(int facet, iArrayT& facetnodes) const
{
#if __option(extended_errorcheck)
	if (facet != 0 && facet != 1) throw eOutOfRange;
#else
#pragma unused (facet)	
#endif

	facetnodes.Allocate(1);
	facetnodes[0] = facet;
}

void LineT::NumNodesOnFacets(iArrayT& num_nodes) const
{
	num_nodes.Allocate(2);
	num_nodes = 1;
}

/* returns the nodes on each facet needed to determine neighbors
 * across facets */
void LineT::NeighborNodeMap(iArray2DT& facetnodes) const
{
	facetnodes.Allocate(2,1);
	facetnodes(0,0) = 0;
	facetnodes(1,0) = 1;
}

/* return geometry and number of nodes on each facet */
void LineT::FacetGeometry(ArrayT<GeometryCode>& facet_geom, iArrayT& facet_nodes) const
{
	facet_geom.Allocate(fNumFacets);
	facet_geom = kPoint;
	
	facet_nodes.Allocate(fNumFacets);
	facet_nodes = 1;
}
