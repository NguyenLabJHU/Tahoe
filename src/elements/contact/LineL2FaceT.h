/* $Id */

#ifndef _LINEL2_FACE_T_H_
#define _LINEL2_FACE_T_H_

/* direct members */
#include "iArray2DT.h"

/* forward declarations */
class VectorT;
class FaceT;

/*  connectivity
 *  1--2
 */

class LineL2FaceT : public FaceT
{
public:

        /* constructor */
        LineL2FaceT
	 (SurfaceT& surface,iArrayT& connectivity, dArrayT& coordinates);

        /* destructor */
        LineL2FaceT(void);

        void ComputeCentroid(Vector& centroid); 
	double ComputeRadius();
        void ComputeNormal(double& local_coordinates, Vector& normal); 
        void ComputeTangents // ?????????
		(double& local_coordinates, Vector& tangent1,Vector& tangent2); 
	double ComputeJacobian (double& local_coordinates);
	void ComputeShapeFunctions
		(double& local_coordinates, double& shape_functions);
	void ComputeShapeFunctions
		(double& local_coordinates, MatrixT& shape_functions);
	void ComputeShapeFunctionDerivatives
		(double& local_coordinates, double& shape_derivatives);
	void ComputeShapeFunctionDerivatives
		(double& local_coordinates, MatrixT& shape_derivatives);
        bool Projection
		(Vector& point, Vector& normal, 
		double& local_coordinates, double gap); 
	/*
        bool Projection
		(Vector& point, 
		double& local_coordinates, double gap); 
	*/
protected:

private:
	/* nodal coordinates */
	double* fx1, fx2;

};

#endif /* _LINEL2_FACE_T_H_ */

