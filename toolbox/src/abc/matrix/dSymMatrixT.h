/* $Id: dSymMatrixT.h,v 1.8 2002-10-04 01:36:34 thao Exp $ */
/* created: paklein (05/24/1996) */

#ifndef _DSYM_MATRIX_T_H_
#define _DSYM_MATRIX_T_H_

/* base class */
#include "dArrayT.h"

namespace Tahoe {

/* forward declarations */
class dMatrixT;

/* interface for a 1D/2D/3D reduced index symmetric matrix stored as 
 * a reduced index vector */
class dSymMatrixT: public dArrayT
{
public:

	/* constructors */
	dSymMatrixT(void);
	explicit dSymMatrixT(int nsd);
	dSymMatrixT(int nsd, double* array);
	dSymMatrixT(const dSymMatrixT& source);

	/** dimension the matrix for the number of spatial dimensions. No change 
	 * occurs if the array is already the specified length. The previous contents 
	 * of the array is not preserved. */
	void Dimension(int nsd);

	/** \deprecated replaced by dSymMatrixT::Dimension on 02/13/2002 */
	void Allocate(int nsd) { Dimension(nsd); };

	/* set fields */
	void Set(int nsd, double* array);

	/* assignment operators */
	dSymMatrixT& operator=(const dSymMatrixT& RHS);
	dSymMatrixT& operator=(const double value);

	/* accessor */
	double& operator()(int row, int col) const;

	/* returns the number of dimensions in the reduced
	 * index symmetric matrix vectors given the number of spatial
	 * dimensions.  nsd can only be 2 or 3. */
	static int NumValues(int nsd);
	static void ExpandIndex(int nsd, int dex, int& dex_1, int& dex_2);

	/* dimensions */
	int Rows(void) const;
	int Cols(void) const;
	
	/* I/O operators */
	friend ostream& operator<<(ostream& out, const dSymMatrixT& array);
	friend istream& operator>>(istream& in, dSymMatrixT& array);

	/* return eigenvalues and eigenvectors (in columns) */
	void PrincipalValues(dArrayT& val) const; // will get phased out
	void Eigenvalues(dArrayT& val, bool sort_descending) const;
	void Eigensystem(dArrayT& val, dMatrixT& vec, bool sort_descending) const;

	/* returns the scalar inner product of a tensor with itself
	 * define as:
	 *
	 *             T:T = T_ij T_ij
	 */
	double ScalarProduct(void) const;
	/* 
	 *             A:B = A_ij B_ij
	 */
	double ScalarProduct(const dSymMatrixT& matrix) const;
	
	/* the second invariant */
	double Invariant2(void) const;

	/* matrix determinant */
	double Det(void) const;

	/* returns the trace of *this.  For 2D, the out-of-plane
	 * component is assumed to be zero, ie if *this represents stress,
	 * then plane stress is assumed and if *this is strain then plane
	 * strain is assumed. */
	double Trace(void) const;

	/* identity operations */
	void PlusIdentity(double value = 1.0);
	dSymMatrixT& Identity(double value = 1.0);
	
	/* returns the deviatoric part of *this.
	 *
	 * Note: This function should only be called when working in
	 * 3D, ie. a vector length 6, since there's no way to enforce
	 * Trace(Deviatoric{vector)) = 0, in 2D. */
	dSymMatrixT& Deviatoric(const dSymMatrixT& tensor);
	dSymMatrixT& Deviatoric(void);

	/* matrix inverse */
	dSymMatrixT& Inverse(const dSymMatrixT& matrix);
	dSymMatrixT& Inverse(void);

	/* take the symmetric part of the matrix */
	dSymMatrixT& Symmetrize(const dMatrixT& matrix);

	/* reduced index <-> matrix transformations */	
	void ToMatrix(dMatrixT& matrix) const;
	dSymMatrixT& FromMatrix(const dMatrixT& matrix);

	/* 2D <-> 3D translations.  Return references to *this */
	dSymMatrixT& ExpandFrom2D(const dSymMatrixT& vec2D); /* assumed plane strain */
	dSymMatrixT& ReduceFrom3D(const dSymMatrixT& vec3D);

	/* outer product */
	void Outer(const dArrayT& v);

	/* matrix-matrix multiplication */
	void MultAB(const dSymMatrixT& A, const dSymMatrixT& B); /* A and B must commute */
	void MultATA(const dMatrixT& A);
	void MultAAT(const dMatrixT& A);

	/* matrix-matrix-matrix operations, ie. tensor basis transformations */
	void MultQBQT(const dMatrixT& Q, const dSymMatrixT& B);
	void MultQTBQ(const dMatrixT& Q, const dSymMatrixT& B);

	/* matrix-vector multiplication */
	void Multx(const dArrayT& x, dArrayT& b) const;

	/* vector-matrix-vector product */
	double MultmBn(const dArrayT& m, const dArrayT& n) const;

	/* symmetric rank-4 - rank-2 contraction */
	void A_ijkl_B_kl(const dMatrixT& A, const dSymMatrixT& B);
	void A_ijkl_B_ij(const dMatrixT& A, const dSymMatrixT& B);

	/* symmetric rank-(2-4-2) contraction with this */
	double B_ij_A_ijkl_B_kl(const dMatrixT& A) const;

	/** scale off-diagonal value by the given factor */
	void ScaleOffDiagonal(double factor);

private:

	/* iterative eigensystem routines */
	int Eigenvalues3D(dArrayT& evals, bool sort_descending, int max_iterations = 15) const;
	int Eigensystem3D(dArrayT& evals, dMatrixT& evecs, bool sort_descending,
		int max_iterations = 15) const; // append algorithm name
		
	/* closed form eigenvalues - unstable for repeated roots */
	void Eigenvalues3D_Cardano(dArrayT& evals) const;
	
private:

	int fNumSD;	
};

/* inlines */

/* dimensions */
inline int dSymMatrixT::NumValues(int nsd)
{
	if (nsd < 1 || nsd > 3) throw eGeneralFail;
	int map[4] = {0, 1, 3, 6};
	return map[nsd];	
}

/* assigment operators */
inline dSymMatrixT& dSymMatrixT::operator=(const dSymMatrixT& RHS)
{
	/* must be same dimension or nsd must be zero */
	if (fNumSD != 0 && fNumSD != RHS.fNumSD)
		throw eSizeMismatch;
	else
		fNumSD = RHS.fNumSD;

	/* inherited */
	dArrayT::operator=(RHS);
	return *this;
}

inline dSymMatrixT& dSymMatrixT::operator=(const double value)
{
	/* inherited */
	dArrayT::operator=(value);
	return *this;
}

/* dimensions */
inline int dSymMatrixT::Rows(void) const { return fNumSD; }
inline int dSymMatrixT::Cols(void) const { return fNumSD; }
inline dSymMatrixT& dSymMatrixT::Deviatoric(void) { return Deviatoric(*this); }
inline dSymMatrixT& dSymMatrixT::Inverse(void) { return Inverse(*this); }

/* scale off-diagonal value by the given factor */
inline void dSymMatrixT::ScaleOffDiagonal(double factor)
{
	if (fNumSD == 2)
		fArray[2] *= factor;
	else if (fNumSD == 3) {
		fArray[3] *= factor;
		fArray[4] *= factor;	
		fArray[5] *= factor;	
	}
}

} // namespace Tahoe 
#endif /* _DSYM_MATRIX_T_H_ */
