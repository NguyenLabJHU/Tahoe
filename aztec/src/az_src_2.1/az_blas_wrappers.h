/* Copyright (2001) Sandia Corportation. Under the terms of Contract 
 * DE-AC04-94AL85000, there is a non-exclusive license for use of this 
 * work by or on behalf of the U.S. Government.  Export of this program
 * may require a license from the United States Government. */


/* NOTICE:  The United States Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable worldwide
 * license in ths data to reproduce, prepare derivative works, and
 * perform publicly and display publicly.  Beginning five (5) years from
 * July 25, 2001, the United States Government is granted for itself and
 * others acting on its behalf a paid-up, nonexclusive, irrevocable
 * worldwide license in this data to reproduce, prepare derivative works,
 * distribute copies to the public, perform publicly and display
 * publicly, and to permit others to do so.
 * 
 * NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT
 * OF ENERGY, NOR SANDIA CORPORATION, NOR ANY OF THEIR EMPLOYEES, MAKES
 * ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR
 * RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
 * INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS
 * THAT ITS USE WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS. */

#ifndef _AZ_BLAS_WRAPPERS_H_
#define _AZ_BLAS_WRAPPERS_H_

#include "az_f77func.h"

#if defined(CRAY_T3X)

#define DGEMV_F77   F77_FUNC(sgemv,SGEMV)
#define DGEMM_F77   F77_FUNC(sgemm,SGEMM)
#define DTRSM_F77   F77_FUNC(strsm,STRSM)
#define DTRMM_F77   F77_FUNC(strmm,STRMM)
#define IDAMAX_F77  F77_FUNC(isamax,ISAMAX)
#define DASUM_F77   F77_FUNC(sasum,SASUM)
#define DAXPY_F77   F77_FUNC(saxpy,SAXPY)
#define DCOPY_F77   F77_FUNC(scopy,SCOPY)
#define DDOT_F77    F77_FUNC(sdot,SDOT)
#define DSCAL_F77   F77_FUNC(sscal,SSCAL)

#elif 0

#define DGEMV_F77   F77_FUNC(dgemv,DGEMV)
#define DGEMM_F77   F77_FUNC(dgemm,DGEMM)
#define DTRSM_F77   F77_FUNC(dtrsm,DTRSM)
#define DTRMM_F77   F77_FUNC(dtrmm,DTRMM)
#define IDAMAX_F77  F77_FUNC(idamax,IDAMAX)
#define DASUM_F77   F77_FUNC(dasum,DASUM)
#define DAXPY_F77   F77_FUNC(daxpy,DAXPY)
#define DCOPY_F77   F77_FUNC(dcopy,DCOPY)
#define DDOT_F77    F77_FUNC(ddot,DDOT)
#define DSCAL_F77   F77_FUNC(dscal,DSCAL)

#else

#define DGEMV_F77   dgemv_
#define DGEMM_F77   dgemm_
#define DTRSM_F77   dtrsm_
#define DTRMM_F77   dtrmm_
#define IDAMAX_F77  idamax_
#define DASUM_F77   dasum_
#define DAXPY_F77   daxpy_
#define DCOPY_F77   dcopy_
#define DDOT_F77    ddot_
#define DSCAL_F77   dscal_

#endif



#ifdef __cplusplus
extern "C" {
#include <stdio.h>
#endif

/* Double precision BLAS 1 */
double PREFIX DASUM_F77(int* n, double x[], int* incx);
void PREFIX DAXPY_F77(int* n, double* alpha, double x[], int* incx, double y[], int* incy);
void PREFIX DCOPY_F77(int* n, double *x, int* incx, double *y, int* incy);
double PREFIX DDOT_F77(int* n, double x[], int* incx, double y[], int* incy);
void PREFIX DSCAL_F77(int* n, double* alpha, double *x, int* incx);
int PREFIX IDAMAX_F77(int* n, double *x, int* incx);


/* Double precision BLAS 2 */
void PREFIX DGEMV_F77(az_fcd, int* m, int* n, double* alpha, double A[], int* lda,
		       double x[], int* incx, double* beta, double y[], int* incy);

/* Double precision BLAS 3 */
void PREFIX DGEMM_F77(az_fcd, az_fcd, int *m, int *
		      n, int *k, double *alpha, double *a, int *lda, 
		      double *b, int *ldb, double *beta, double *c, int *ldc);
void PREFIX DTRMM_F77(az_fcd, az_fcd, az_fcd, az_fcd, 
		      int *m, int *n, double *alpha, double *a, int * lda, double *b, int *ldb);
void PREFIX DTRSM_F77(az_fcd, az_fcd, az_fcd, az_fcd, 
		      int *m, int *n, double *alpha, double *a, int *
		      lda, double *b, int *ldb);

/* Single precision BLAS 1 */
float PREFIX SASUM_F77(int* n, float x[], int* incx);
void PREFIX SAXPY_F77(int* n, float* alpha, float x[], int* incx, float y[], int* incy);
void PREFIX SCOPY_F77(int* n, float *x, int* incx, float *y, int* incy);
float PREFIX SDOT_F77(int* n, float x[], int* incx, float y[], int* incy);
void PREFIX DSCAL_F77(int* n, double* alpha, double *x, int* incx);
int PREFIX IDAMAX_F77(int* n, double *x, int* incx);

/* Single precision BLAS 2 */
void PREFIX SGEMV_F77(az_fcd, int* m, int* n, float* alpha, float A[], int* lda,
		       float x[], int* incx, float* beta, float y[], int* incy);

/* Single precision BLAS 3 */
void PREFIX SGEMM_F77(az_fcd, az_fcd, int *m, int *
		      n, int *k, float *alpha, float *a, int *lda, 
		      float *b, int *ldb, float *beta, float *c, int *ldc);
void PREFIX STRMM_F77(az_fcd, az_fcd, az_fcd, az_fcd, 
		      int *m, int *n, float *alpha, float *a, int * lda, float *b, int *ldb);
void PREFIX STRSM_F77(az_fcd, az_fcd, az_fcd, az_fcd, 
		      int *m, int *n, float *alpha, float *a, int *
		      lda, float *b, int *ldb);


#ifdef __cplusplus
}
#endif

#endif /* _AZ_BLAS_WRAPPERS_H_ */
