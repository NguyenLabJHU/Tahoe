/* $Id: superlu.h,v 1.2 2002-07-02 19:56:46 cjkimme Exp $ */
/* created: rbridson (07/06/2000)                                         */
/* Header file for calling SuperLU                                        */

#ifndef _SUPERLU_H_
#define _SUPERLU_H_

#include "supermatrix.h"

#ifdef __cplusplus

namespace Tahoe {

extern "C" {
} // namespace Tahoe 
#endif

/****************** functions provided by SuperLU */

extern
void sp_preorder (char *refact, SuperMatrix *A, int *perm_c, int *etree,
SuperMatrix *AC);

extern
void dgstrf (char *refact, SuperMatrix *A, double diag_pivot_thresh,
double drop_tol, int relax, int panel_size, int *etree,
void *work, int lwork, int *perm_r, int *perm_c,
SuperMatrix *L, SuperMatrix *U, int *info);

extern
void dgstrs (char *trans, SuperMatrix *L, SuperMatrix *U,
int *perm_r, int *perm_c, SuperMatrix *B, int *info);

extern
void Destroy_CompCol_Matrix (SuperMatrix *M);

extern
void Destroy_SuperNode_Matrix (SuperMatrix *M);

extern
void StatInit(int panel_size, int relax);

extern
void StatFree();

extern
void get_colamd (const int m, const int n, const int nnz, int *colptr,
int *rowind, int *perm_c);

#ifdef __cplusplus
}  /* extern "C" */
} // namespace Tahoe 
#endif

} // namespace Tahoe 
#endif /* _SUPERLU_H_ */
