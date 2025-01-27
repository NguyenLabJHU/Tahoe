/* $Id: LU_MPI_driver_solve.c,v 1.3 2005-04-05 16:02:35 paklein Exp $ */
#include "LU_MPI_driver_int.h"

#if 0
int LU_MPI_driver(int msg_lvl, const char* message_file, int matrix_type,
	int symmetry_flag, int pivoting_flag, int rand_seed, int num_eq, int num_row,
	int* r_rhs, double* rhs2out, int num_entries, int* r, int* c, double* v, 
	MPI_Comm* comm)
#endif
/* solve linear system */
int LU_MPI_driver_solve(int msg_lvl, const char* message_file,
	int* r_rhs, double* rhs2out, void** ppLU_dat,
	MPI_Comm* comm)
{
/*
   ------------------------------------------------------------
   all-in-one MPI program for each process

   order, factor and solve A X = Y

   ( 1) read in matrix entries and form InpMtx object for A
   ( 2) order the system using minimum degree
   ( 3) permute the front tree
   ( 4) create the owners map IV object
   ( 5) permute the matrix A and redistribute
   ( 6) compute the symbolic factorization 
   ( 7) compute the numeric factorization
   ( 8) split the factors into submatrices
   ( 9) create the submatrix map and redistribute
   (10) read in right hand side entries 
        and form dense matrix DenseMtx object for Y
   (11) permute and redistribute Y
   (12) solve the linear system
   (13) gather X on processor 0

   created -- 98jun13, cca
   ------------------------------------------------------------
*/

/*--------------------------------------------------------------------*/
/* resolve pointer to the solver data */
LU_MPI_driver_data* pLU_dat = (LU_MPI_driver_data*)(*ppLU_dat);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
char            buffer[128] ;
/* Chv             *rootchv ; */
/* ChvManager      *chvmanager ; */
DenseMtx        *mtxX, *mtxY, *newY, *mtxX_tmp;
SubMtxManager   /* *mtxmanager, */ *solvemanager;
FrontMtx        *frontmtx ;
/* InpMtx          *mtxA, *newA ; */
double          cutoff, droptol = 0.0, minops, tau = 100. ;
double          cpus[20] ;
double          *opcounts ;
DV              *cumopsDV ;
ETree           *frontETree ;
FILE            *inputFile, *msgFile ;
Graph           *graph ;
int             error, firsttag, ient, irow, jcol, lookahead = 0, 
                msglvl, myid, nedges, nent, neqns, nmycol, nproc, nrhs,
                nrow, pivotingflag, root, seed, symmetryflag, matrixtype ;
int             stats[20] ;
int             *rowind ;
IV              *oldToNewIV, *ownedColumnsIV, *ownersIV, 
                *newToOldIV, *vtxmapIV ;
/* IVL             *adjIVL, *symbfacIVL ; */
SolveMap        *solvemap ;

/* PAK - needed to return solution to local processors */
int* row_counts;
int* disp_vec;
int* row_map;
int* pmap;
int MPI_return, eq_sum, i, j;
/* PAK - end */

/*--------------------------------------------------------------------*/
/*
   ---------------------------------------------------------------
   find out the identity of this process and the number of process
   ---------------------------------------------------------------
*/
/* MPI_Init(&argc, &argv) ; */
MPI_Comm_rank(*comm, &myid) ;
MPI_Comm_size(*comm, &nproc) ;
/*--------------------------------------------------------------------*/
/*
   --------------------
   get input parameters
   --------------------
*/

/* resolve arguments */
if ( (msgFile = fopen(message_file, "a")) == NULL ) {
	fprintf(stderr, "\n SPOOLES:LU_MPI_driver: unable to open message file %s\n",
		message_file) ;
	return(-1);
   }
msglvl = msg_lvl;
#if 0
matrixtype = matrix_type;
symmetryflag = symmetry_flag;
pivotingflag = pivoting_flag;
seed = rand_seed;
#endif

if (!pLU_dat) {
	fprintf(stderr, "\n SPOOLES:LU_MPI_driver_solve: pLU_dat is NULL\n",
	message_file) ;
   	return -1;
}
else {

	/* data structures computed during factorization */
	frontmtx = pLU_dat->frontmtx;
	solvemap = pLU_dat->solvemap;
	vtxmapIV = pLU_dat->vtxmapIV;
	frontETree = pLU_dat->frontETree;
	oldToNewIV = pLU_dat->oldToNewIV;
	newToOldIV = pLU_dat->newToOldIV;
	ownersIV = pLU_dat->ownersIV;

	/* flags and dimensions */
	matrixtype = pLU_dat->matrix_type;
	symmetryflag = pLU_dat->symmetry_flag;
	pivotingflag = pLU_dat->pivoting_flag;
	seed = pLU_dat->rand_seed;
}

IVzero(20, stats) ;
DVzero(20, cpus) ;
if (msglvl > 0) {
fprintf(msgFile, 
        "\n\n input data"
        "\n msglvl       = %d"
        "\n msgFile      = %s"
        "\n type         = %d"
        "\n symmetryflag = %d"
        "\n pivotingflag = %d"
        "\n seed         = %d",
        msglvl, message_file, matrixtype, symmetryflag, pivotingflag, seed) ;
fflush(msgFile);
}
MPI_Barrier(*comm);
/*--------------------------------------------------------------------*/
/*
   --------------------------------------------
   STEP 1: read the entries from the input file 
           and create the InpMtx object
   --------------------------------------------
*/
/* sprintf(buffer, "haggar.mtx.%d.input", myid) ;*/
#if 0
sprintf(buffer, "matrix.%d.input", myid);
inputFile = fopen(buffer, "r") ;
fscanf(inputFile, "%d %d %d", &neqns, &neqns, &nent) ;
MPI_Barrier(*comm) ;
#endif

neqns = pLU_dat->num_eq;

#if 0
nent = num_entries;
mtxA = InpMtx_new() ;
InpMtx_init(mtxA, INPMTX_BY_ROWS, matrixtype, nent, 0) ;
#endif

#if 0
if ( matrixtype == SPOOLES_REAL ) {
   double   value ;
   for ( ient = 0 ; ient < nent ; ient++ ) {
      fscanf(inputFile, "%d %d %le", &irow, &jcol, &value) ;
      InpMtx_inputRealEntry(mtxA, irow, jcol, value) ;
   }
} else if ( matrixtype == SPOOLES_COMPLEX ) {
   double   imag, real ;
   for ( ient = 0 ; ient < nent ; ient++ ) {
      fscanf(inputFile, "%d %d %le %le", &irow, &jcol, &real, &imag) ;
      InpMtx_inputComplexEntry(mtxA, irow, jcol, real, imag) ;
   }
}
fclose(inputFile) ;
#endif

#if 0
if (matrixtype == SPOOLES_REAL)
	/* enter all {row, column, value} triples */
	InpMtx_inputRealTriples(mtxA, num_entries, r, c, v);
else
{
   fprintf(stderr, "\n SPOOLES:LU_MPI_driver: real matrices only");
   return -1;
}
InpMtx_sortAndCompress(mtxA) ;
InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n input matrix") ;
   InpMtx_writeForHumanEye(mtxA, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   ----------------------------------------------------
   STEP 2: read the rhs entries from the rhs input file 
   and create the DenseMtx object for Y
   ----------------------------------------------------
*/
#if 0
/* sprintf(buffer, "haggar.rhs.%d.input", myid) ; */
sprintf(buffer, "rhs.%d.input", myid) ;
inputFile = fopen(buffer, "r") ;
fscanf(inputFile, "%d %d", &nrow, &nrhs) ;
#endif
nrow = pLU_dat->num_row;
nrhs = 1;

mtxY = DenseMtx_new() ;
DenseMtx_init(mtxY, matrixtype, 0, 0, nrow, nrhs, 1, nrow);
DenseMtx_rowIndices(mtxY, &nrow, &rowind) ;

#if 0
if ( matrixtype == SPOOLES_REAL ) {
   double   value ;
   for ( irow = 0 ; irow < nrow ; irow++ ) {
      fscanf(inputFile, "%d", rowind + irow) ;
      for ( jcol = 0 ; jcol < nrhs ; jcol++ ) {
         fscanf(inputFile, "%le", &value) ;
         DenseMtx_setRealEntry(mtxY, irow, jcol, value) ;
      }
   }
} if ( matrixtype == SPOOLES_COMPLEX ) {
   double   imag, real ;
   for ( irow = 0 ; irow < nrow ; irow++ ) {
      fscanf(inputFile, "%d", rowind + irow) ;
      for ( jcol = 0 ; jcol < nrhs ; jcol++ ) {
         fscanf(inputFile, "%le %le", &real, &imag) ;
         DenseMtx_setComplexEntry(mtxY, irow, jcol, real, imag) ;
      }
   }
}
fclose(inputFile) ;
#endif

if (matrixtype == SPOOLES_REAL)
{
	for (irow = 0 ; irow < nrow ; irow++)
	{
		rowind[irow] = r_rhs[irow];
		DenseMtx_setRealEntry(mtxY, irow, 0, rhs2out[irow]);
		/* NOTE: there is no function to copy a vector into a column */
	}
}
else
{
   fprintf(stderr, "\n SPOOLES:LU_serial_driver: real matrices only");
   return -1;
}

if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n rhs matrix in original ordering") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   -------------------------------------------------------
   STEP 2 : find a low-fill ordering
   (1) create the Graph object
   (2) order the graph using multiple minimum degree
   (3) find out who has the best ordering w.r.t. op count,
       and broadcast that front tree object
   -------------------------------------------------------
*/
#if 0
graph = Graph_new() ;
adjIVL = InpMtx_MPI_fullAdjacency(mtxA, stats, 
                                  msglvl, msgFile, *comm) ;
nedges = IVL_tsize(adjIVL) ;
Graph_init2(graph, 0, neqns, 0, nedges, neqns, nedges, adjIVL,
            NULL, NULL) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n graph of the input matrix") ;
   Graph_writeForHumanEye(graph, msgFile) ;
   fflush(msgFile) ;
}
frontETree = orderViaMMD(graph, seed + myid, msglvl, msgFile) ;
Graph_free(graph) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n front tree from ordering") ;
   ETree_writeForHumanEye(frontETree, msgFile) ;
   fflush(msgFile) ;
}
opcounts = DVinit(nproc, 0.0) ;
opcounts[myid] = ETree_nFactorOps(frontETree, matrixtype, symmetryflag) ;
MPI_Allgather((void *) &opcounts[myid], 1, MPI_DOUBLE,
              (void *) opcounts, 1, MPI_DOUBLE, *comm) ;
minops = DVmin(nproc, opcounts, &root) ;
DVfree(opcounts) ;
frontETree = ETree_MPI_Bcast(frontETree, root, 
                             msglvl, msgFile, *comm) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n best front tree") ;
   ETree_writeForHumanEye(frontETree, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   -------------------------------------------------------
   STEP 3: get the permutations, permute the front tree,
           permute the matrix and right hand side.
   -------------------------------------------------------
*/
#if 0
oldToNewIV = ETree_oldToNewVtxPerm(frontETree) ;
newToOldIV = ETree_newToOldVtxPerm(frontETree) ;
ETree_permuteVertices(frontETree, oldToNewIV) ;
InpMtx_permute(mtxA, IV_entries(oldToNewIV), IV_entries(oldToNewIV)) ;
if (  symmetryflag == SPOOLES_SYMMETRIC 
   || symmetryflag == SPOOLES_HERMITIAN ) { 
   InpMtx_mapToUpperTriangle(mtxA) ;
}
InpMtx_changeCoordType(mtxA, INPMTX_BY_CHEVRONS) ;
InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;
#endif
DenseMtx_permuteRows(mtxY, oldToNewIV) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n rhs matrix in new ordering") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   -------------------------------------------
   STEP 4: generate the owners map IV object
           and the map from vertices to owners
   -------------------------------------------
*/
#if 0
cutoff   = 1./(2*nproc) ;
cumopsDV = DV_new() ;
DV_init(cumopsDV, nproc, NULL) ;
ownersIV = ETree_ddMap(frontETree, 
                       matrixtype, symmetryflag, cumopsDV, cutoff) ;
DV_free(cumopsDV) ;
vtxmapIV = IV_new() ;
IV_init(vtxmapIV, neqns, NULL) ;
IVgather(neqns, IV_entries(vtxmapIV), 
         IV_entries(ownersIV), ETree_vtxToFront(frontETree)) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n map from fronts to owning processes") ;
   IV_writeForHumanEye(ownersIV, msgFile) ;
   fprintf(msgFile, "\n\n map from vertices to owning processes") ;
   IV_writeForHumanEye(vtxmapIV, msgFile) ;
   fflush(msgFile) ;
}
#endif
#if 0
if ( myid == 0 ) {
   IV_writeToFile(ownersIV, "../../Tree/drivers/haggar.ivf") ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   ---------------------------------------------------
   STEP 5: redistribute the matrix and right hand side
   ---------------------------------------------------
*/
firsttag = 0 ;
#if 0
newA = InpMtx_MPI_split(mtxA, vtxmapIV, stats, 
                        msglvl, msgFile, firsttag, *comm) ;
#endif
firsttag++ ;
#if 0
InpMtx_free(mtxA) ;
mtxA = newA ;
InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n split InpMtx") ;
   InpMtx_writeForHumanEye(mtxA, msgFile) ;
   fflush(msgFile) ;
}
#endif
newY = DenseMtx_MPI_splitByRows(mtxY, vtxmapIV, stats, msglvl, 
                                msgFile, firsttag, *comm) ;
DenseMtx_free(mtxY) ;
mtxY = newY;
firsttag += nproc ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n split DenseMtx Y") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------
   STEP 6: compute the symbolic factorization
   ------------------------------------------
*/
#if 0
symbfacIVL = SymbFac_MPI_initFromInpMtx(frontETree, ownersIV, mtxA,
                     stats, msglvl, msgFile, firsttag, *comm) ;
#endif
firsttag += frontETree->nfront ;
#if 0
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n local symbolic factorization") ;
   IVL_writeForHumanEye(symbfacIVL, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   -----------------------------------
   STEP 7: initialize the front matrix
   -----------------------------------
*/
#if 0
mtxmanager = SubMtxManager_new() ;
SubMtxManager_init(mtxmanager, NO_LOCK, 0) ;
frontmtx = FrontMtx_new() ;
FrontMtx_init(frontmtx, frontETree, symbfacIVL, matrixtype, symmetryflag,
              FRONTMTX_DENSE_FRONTS, pivotingflag, NO_LOCK, myid,
              ownersIV, mtxmanager, msglvl, msgFile) ;
#endif
/*--------------------------------------------------------------------*/
/*
   ---------------------------------
   STEP 8: compute the factorization
   ---------------------------------
*/
#if 0
chvmanager = ChvManager_new() ;
ChvManager_init(chvmanager, NO_LOCK, 0) ;
rootchv = FrontMtx_MPI_factorInpMtx(frontmtx, mtxA, tau, droptol,
                     chvmanager, ownersIV, lookahead, &error, cpus, 
                     stats, msglvl, msgFile, firsttag, *comm) ;
ChvManager_free(chvmanager) ;
#endif
firsttag += 3*frontETree->nfront + 2 ;
#if 0
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n numeric factorization") ;
   FrontMtx_writeForHumanEye(frontmtx, msgFile) ;
   fflush(msgFile) ;
}
if ( error >= 0 ) {
   fprintf(stderr, 
          "\n proc %d : factorization error at front %d", myid, error) ;
   /* MPI_Finalize() ; */
   return -1;
}
#endif
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------------
   STEP 9: post-process the factorization and split 
   the factor matrices into submatrices 
   ------------------------------------------------
*/
#if 0
FrontMtx_MPI_postProcess(frontmtx, ownersIV, stats, msglvl,
                         msgFile, firsttag, *comm) ;
#endif
firsttag += 5*nproc ;
#if 0
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n numeric factorization after post-processing");
   FrontMtx_writeForHumanEye(frontmtx, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   -----------------------------------
   STEP 10: create the solve map object
   -----------------------------------
*/
#if 0
solvemap = SolveMap_new() ;
SolveMap_ddMap(solvemap, frontmtx->symmetryflag, 
               FrontMtx_upperBlockIVL(frontmtx),
               FrontMtx_lowerBlockIVL(frontmtx),
               nproc, ownersIV, FrontMtx_frontTree(frontmtx), 
               seed, msglvl, msgFile);
if ( msglvl > 2 ) {
   SolveMap_writeForHumanEye(solvemap, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   ----------------------------------------------------
   STEP 11: redistribute the submatrices of the factors
   ----------------------------------------------------
*/
#if 0
FrontMtx_MPI_split(frontmtx, solvemap, 
                   stats, msglvl, msgFile, firsttag, *comm) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n numeric factorization after split") ;
   FrontMtx_writeForHumanEye(frontmtx, msgFile) ;
   fflush(msgFile) ;
}
#endif
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------------
   STEP 13: permute and redistribute Y if necessary
   ------------------------------------------------
*/
if ( FRONTMTX_IS_PIVOTING(frontmtx) ) {
   IV   *rowmapIV ;
/*
   ----------------------------------------------------------
   pivoting has taken place, redistribute the right hand side
   to match the final rows and columns in the fronts
   ----------------------------------------------------------
*/
   rowmapIV = FrontMtx_MPI_rowmapIV(frontmtx, ownersIV, msglvl,
                                    msgFile, *comm) ;
   newY = DenseMtx_MPI_splitByRows(mtxY, rowmapIV, stats, msglvl, 
                                   msgFile, firsttag, *comm) ;
   DenseMtx_free(mtxY) ;
   mtxY = newY ;
   IV_free(rowmapIV) ;
}
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n rhs matrix after split") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------
   STEP 14: create a solution DenseMtx object
   ------------------------------------------
*/
ownedColumnsIV = FrontMtx_ownedColumnsIV(frontmtx, myid, ownersIV,
                                         msglvl, msgFile) ;
nmycol = IV_size(ownedColumnsIV) ;
mtxX = DenseMtx_new() ;
if ( nmycol > 0 ) {
   DenseMtx_init(mtxX, matrixtype, 0, 0, nmycol, nrhs, 1, nmycol) ;
   DenseMtx_rowIndices(mtxX, &nrow, &rowind) ;
   IVcopy(nmycol, rowind, IV_entries(ownedColumnsIV)) ;
}
/*--------------------------------------------------------------------*/
/*
   --------------------------------
   STEP 15: solve the linear system
   --------------------------------
*/
solvemanager = SubMtxManager_new() ;
SubMtxManager_init(solvemanager, NO_LOCK, 0) ;
FrontMtx_MPI_solve(frontmtx, mtxX, mtxY, solvemanager, solvemap, cpus, 
                   stats, msglvl, msgFile, firsttag, *comm) ;
SubMtxManager_free(solvemanager) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n solution in new ordering") ;
   DenseMtx_writeForHumanEye(mtxX, msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   --------------------------------------------------------
   STEP 15: permute the solution into the original ordering
            and assemble the solution onto processor zero
   --------------------------------------------------------
*/
DenseMtx_permuteRows(mtxX, newToOldIV) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n solution in old ordering") ;
   DenseMtx_writeForHumanEye(mtxX, msgFile) ;
   fflush(msgFile) ;
}

/* PAK - collect map of originating processors */
row_counts = (int *) malloc(nproc*sizeof(int));
disp_vec = (int *) malloc(nproc*sizeof(int));
row_map = (int *) malloc(pLU_dat->num_eq*sizeof(int));
if (!row_counts || !disp_vec || !row_map) return -1;
MPI_return = MPI_Allgather(&(pLU_dat->num_row), 1, MPI_INT,
	row_counts, 1, MPI_INT, *comm);
if (msglvl > 2) {
   fprintf(msgFile, "\n\n MPI_ERROR = %d", MPI_return);
   fflush(msgFile) ;
}
eq_sum = 0;
for (i = 0; i < nproc; i++)
{
	disp_vec[i] = eq_sum;
	eq_sum += row_counts[i];
}
if (eq_sum != pLU_dat->num_eq) {
	fprintf(msgFile, "\n\n sum of equations from proc's %d does not equal total %d",
		eq_sum, pLU_dat->num_eq);
	return -1;
}
/* collect rows-by-processor */
MPI_return = MPI_Allgatherv(r_rhs, pLU_dat->num_row, MPI_INT, row_map,
	row_counts, disp_vec, MPI_INT, *comm);
if (msglvl > 2) {
   fprintf(msgFile, "\n\n MPI_ERROR = %d", MPI_return);
   fflush(msgFile) ;
}
/* set row-to-processor map */
pmap = IV_entries(vtxmapIV);
for (i = 0; i < pLU_dat->num_eq; i++)
	pmap[i] = -1;
for (i = 0; i < nproc; i++)
{
	int*rows = row_map + disp_vec[i];
	for (j = 0; j < row_counts[i]; j++)
	{
		int* n = pmap + rows[j];
		if (*n != -1)
		{
   			fprintf(msgFile, "\n\n Attempt by proc %d to reassign row %d from proc %d",
   				i, rows[j], *n);
   			fflush(msgFile);
			return -1;
		}
		else
			*n = i;
	}
}
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n row-to-processor map") ;
   IV_writeForHumanEye(vtxmapIV, msgFile) ;
}
free(row_counts);
free(disp_vec);
free(row_map);
/* PAK - end */

/* redistribute */
/* IV_fill(vtxmapIV, 0) ; */
firsttag++ ;
mtxX_tmp = mtxX;
mtxX = DenseMtx_MPI_splitByRows(mtxX_tmp, vtxmapIV, stats, msglvl, msgFile,
	                                firsttag, *comm);
DenseMtx_free(mtxX_tmp);
if (msglvl > 0) {
   fprintf(msgFile, "\n\n complete solution in old ordering") ;
   DenseMtx_writeForHumanEye(mtxX, msgFile) ;
   fflush(msgFile) ;
}

/* PAK - extract data */
if (matrixtype == SPOOLES_REAL)
{
	for (irow = 0 ; irow < pLU_dat->num_row; irow++) 
		if (DenseMtx_realEntry(mtxX, irow, 0, rhs2out + irow) != 1)
		/* NOTE: there is no function to copy a column into a vector */
		{
			fprintf(stderr, "\n proc %d : factorization error redistributing", myid);
			return -1;
		}
}
else
{
   fprintf(stderr, "\n SPOOLES:LU_MPI_driver: real matrices only");
   return -1;
}
/* PAK - end */

/* PAK - these structures were not freed above */
/* Chv_free(rootchv); */
DenseMtx_free(mtxX);
DenseMtx_free(mtxY);
/* SubMtxManager_free(mtxmanager); */
/* FrontMtx_free(frontmtx); */
/* InpMtx_free(mtxA); */
/* ETree_free(frontETree); */
/* IV_free(oldToNewIV); */
IV_free(ownedColumnsIV);
/* IV_free(ownersIV); */
/* IV_free(newToOldIV); */
/* IV_free(vtxmapIV); */
/* IVL_free(symbfacIVL); */
/* SolveMap_free(solvemap); */
/* PAK - end */
/*--------------------------------------------------------------------*/
/* MPI_Finalize() ; */

/* close message stream */
fclose(msgFile);

return 1;}
/*--------------------------------------------------------------------*/
