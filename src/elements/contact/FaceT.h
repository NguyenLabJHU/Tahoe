/* $Id: FaceT.h,v 1.15 2001-05-23 14:45:05 rjones Exp $ */

#ifndef _FACE_T_H_
#define _FACE_T_H_

/* direct members */
#include "iArrayT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "ArrayT.h"
#include "GeometryT.h"


/* forward declarations */
class SurfaceT;
class ContactNodeT;
class iArrayT;
class dArrayT;
class dMatrixT;

class FaceT 
{
public:

        /* constructor */
        FaceT	(SurfaceT& surface, 	
		dArray2DT& surface_coordinates,
		int num_face_nodes,
		int* connectivity);


        /* (virtual) destructor */
        virtual ~FaceT(void);

	/* initialization after construction */
	virtual void Initialize(void)=0;

	/* geometric computation */
        virtual void ComputeCentroid(double* centroid) const =0; 
	virtual double ComputeRadius(void) const =0;
        virtual void ComputeNormal
		(const double* local_coordinates, double* normal) const =0; 
        virtual void ComputeTangent1 
                (const double* local_coordinates, double* tangent1) const =0;
        virtual void ComputeTangent2 
                (const double* local_coordinates, double* tangent2) const =0;
        virtual void NodeNormal(int local_node_number,double* normal) const =0; 
	virtual void CalcFaceNormal(void)=0; 
	virtual void LocalBasis
		(double* normal, double* tangent1, double* tangent2) const=0;
	virtual void ComputeShapeFunctions
	  	(const double* local_coordinates, dArrayT& shape_functions)  
		const=0;
	virtual void ComputeShapeFunctions
	  	(const double* local_coordinates, dMatrixT& shape_functions) 
		const=0;
        virtual void ComputeShapeFunctionDerivatives
                (const double* local_coordinates, dArrayT& shape_derivatives) 
		const =0;
        virtual void ComputeShapeFunctionDerivatives
                (const double* local_coordinates, dMatrixT& shape_derivatives) 
		const =0;
	virtual double Interpolate
                (const double* local_coordinates, dArrayT& nodal_values) 
		const =0;
	virtual void InterpolateVector
                (const double* local_coordinates, dArray2DT& nodal_vectors, 
		double* vector) const=0;
	virtual double ComputeJacobian 
		(const double* local_coordinates) const =0;
        virtual bool Projection 
		(ContactNodeT* node, dArrayT& parameters) const =0; 
	virtual void Quadrature
		(dArray2DT& points, dArrayT& weights) const =0;

	/* access functions */
	inline ArrayT<FaceT*>& AssignNeighbors(void) {return fNeighborFaces;}

	/* look-up functions */
	inline const int NumNodes(void) const 
		{return fConnectivity.Length();}
	inline const ArrayT<FaceT*>& Neighbors(void) const 
		{return fNeighborFaces;}
	inline const GeometryT::CodeT GeometryType(void) const 
		{return fGeometryType;}
//inline const int NumIPs(void) const 
//{return fIntegrationPoints.Length();}
 	inline const iArrayT& Connectivity(void) const {return fConnectivity;} 
 	inline const iArrayT& GlobalConnectivity(void) const 
		{return fGlobalConnectivity;} 
 	inline const int Node(int i) const {return fConnectivity[i];} 
	inline const int NumVertexNodes(void) const {return fNumVertexNodes;}
	inline const int Next(int i) const {return (i + 1)%fNumVertexNodes;}
	inline const int Prev(int i) const {return (i - 1)%fNumVertexNodes;}
	inline const int LocalNodeNumber(int node_num) const
	  {for (int i = 0; i < fConnectivity.Length(); i++) {
		if (node_num == fConnectivity[i]) return i ; } return -1; }

	/* check functions */  
	inline bool CheckLocalCoordinates(double* xi, double tol_xi) const
		{return xi[0] < 1.0 + tol_xi && xi[0] >-1.0 - tol_xi
		   &&   xi[1] < 1.0 + tol_xi && xi[1] >-1.0 - tol_xi ; }
	inline bool CheckLocalCoordinates(double xi, double tol_xi) const
		{return xi < 1.0 + tol_xi && xi >-1.0 - tol_xi;}
	inline bool CheckGap(double gap, double tol_g) const
		{ return gap < tol_g ? 1 : 0 ;}
		

protected:
	/* geometry type */
	GeometryT::CodeT fGeometryType;

	/* number of vertex nodes */
	int fNumVertexNodes;

	/* reference to parent surface */
        const SurfaceT& fSurface;

	/* reference to nodal coordinates of surface */
        const dArray2DT& fSurfaceCoordinates;

	/* connectivity, in node numbers local to surface */
	iArrayT fConnectivity;
	iArrayT fGlobalConnectivity;

	/* adjacent faces */
	ArrayT<FaceT*> fNeighborFaces;

	/* face normal */
	double fnormal[3] ;

//const dArray2DT fIntegrationPoints;

	/* workspace */

private:
};

#endif /* _FACE_T_H_ */

