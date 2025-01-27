/* $Id: LU_MT_driver_free.c,v 1.2 2005-04-18 05:45:54 paklein Exp $ */
#include "LU_MT_driver_int.h"

/* free data structures needed for multiples solves */
int LU_MT_driver_free(void** ppLU_dat)
{
	/* resolve pointer */
	LU_MT_driver_data* pLU_dat = (LU_MT_driver_data*)(*ppLU_dat);
	if (!pLU_dat) /* skip if NULL */
		return 0;

	/* set flags and dimensions */
	pLU_dat->matrix_type = 0;
	pLU_dat->symmetry_flag = 0;
	pLU_dat->pivoting_flag = 0;
	pLU_dat->rand_seed = 0;
	pLU_dat->num_eq = 0;

	/* free memory */
	if (pLU_dat->frontmtx) {
		FrontMtx_free(pLU_dat->frontmtx);
		pLU_dat->frontmtx = NULL;
	}
	if (pLU_dat->solvemap) {
		SolveMap_free(pLU_dat->solvemap);
		pLU_dat->solvemap = NULL;
	}
	if (pLU_dat->mtxmanager) {
		SubMtxManager_free(pLU_dat->mtxmanager);
		pLU_dat->mtxmanager = NULL;
	}
	if (pLU_dat->frontETree) {
		ETree_free(pLU_dat->frontETree);
		pLU_dat->frontETree = NULL;
	}
	if (pLU_dat->ownersIV) {
		IV_free(pLU_dat->ownersIV);
		pLU_dat->ownersIV = NULL;
	}
	if (pLU_dat->newToOldIV) {
		IV_free(pLU_dat->newToOldIV);
		pLU_dat->newToOldIV = NULL;
	}
	if (pLU_dat->oldToNewIV) {
		IV_free(pLU_dat->oldToNewIV);
		pLU_dat->oldToNewIV = NULL;
	}

	/* free structure */
	free(pLU_dat);
	*ppLU_dat = NULL;

	return 1;
}
