/*  SPOOLES headers*/

#include "MT.spoolesMT.h"
#include "misc.h"
#include "SymbFac.h"

/* MT driver provided by in SPOOLES documentation */
int  LU_MT_driver(int msg_lvl, const char* message_file, int matrix_type,
	          int symmetry_flag, int pivoting_flag, int rand_seed, 
                  int num_eq, double* rhs2out, int num_entries, 
                  int* r, int* c, double* v, int n_thread)
{
/*
   ------------------------------------------------------------------
   all-in-one program to solve A X = Y
   using a multithreaded factorization and solve

   (1) read in matrix entries for A and form InpMtx object
   (2) read in right hand side for Y entries and form DenseMtx object
   (3) form Graph object, order matrix and form front tree
   (4) get the permutation, permute the front tree, matrix 
       and right hand side and get the symbolic factorization
   (5) initialize the front matrix object to hold the factor matrices
   (6) get the domain-decomposition map from fronts to threads
   (7) compute the numeric factorization
   (8) post-process the factor matrices
   (9) get the map for the parallel solve
   (10) compute the solution
   (11) permute the solution into the original ordering

   created -- 98jun04, cca
   ------------------------------------------------------------------
*/
/*--------------------------------------------------------------------*/
/* char            *matrixFileName, *rhsFileName ; */
DenseMtx        *mtxY, *mtxX ;
Chv             *rootchv ;
ChvManager      *chvmanager ;
double          droptol = 0.0, tau = 100. ;
double          cpus[11] ;
DV              *cumopsDV ;
ETree           *frontETree ;
FrontMtx        *frontmtx ;
 FILE            /* *inputFile, */ *msgFile ;
Graph           *graph ;
InpMtx          *mtxA ;
int             error, ient, irow, jcol, jrhs, jrow, lookahead=0, 
                msglvl, ncol,  nedges, nent, neqns, nfront, nrhs, nrow, 
                pivotingflag, seed, symmetryflag, type, nthread;
int             *newToOld, *oldToNew ;
int             stats[16] ;
IV              *newToOldIV, *oldToNewIV, *ownersIV ;
IVL             *adjIVL, *symbfacIVL ;
SolveMap        *solvemap ;
SubMtxManager   *mtxmanager  ;
/*--------------------------------------------------------------------*/
/*
   --------------------
   open message file
   --------------------
*/

if ( (msgFile = fopen(message_file, "a")) == NULL )
{
   fprintf(stderr, "\n SPOOLES:LU_MT_driver: unable to open message file %s\n",
		message_file) ;
   return(-1);
}

msglvl = msg_lvl;
type = matrix_type;
symmetryflag = symmetry_flag;
pivotingflag = pivoting_flag;
seed = rand_seed;
nthread = n_thread;

IVzero(16, stats);
DVzero(11, cpus);

if (msglvl > 0) {
  fprintf(msgFile, 
        "\n\n input data"
        "\n msglvl       = %d"
        "\n msgFile      = %s"
        "\n type         = %d"
        "\n symmetryflag = %d"
        "\n pivotingflag = %d"
        "\n seed         = %d"
        "\n num threads  = %d",
        msglvl, message_file, type, symmetryflag, pivotingflag, seed, nthread) ;
fflush(msgFile);
}

/*--------------------------------------------------------------------*/
/*
   --------------------------------------------
   STEP 1: read the entries from the input file 
           and create the InpMtx object
   --------------------------------------------
*/
neqns = num_eq;
nent = num_entries;
mtxA = InpMtx_new();
InpMtx_init(mtxA, INPMTX_BY_ROWS, matrix_type, nent, neqns) ;


if (matrix_type == SPOOLES_REAL)
	/* enter all {row, column, value} triples */
	InpMtx_inputRealTriples(mtxA, num_entries, r, c, v);
else
{
   fprintf(stderr, "\n SPOOLES:LU_MT_driver: real matrices only");
   return -1;
}
InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n input matrix") ;
   InpMtx_writeForHumanEye(mtxA, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   -----------------------------------------
   STEP 2: read the right hand side matrix Y
   -----------------------------------------
*/
mtxY = DenseMtx_new();
nrhs = 1;
DenseMtx_init(mtxY, matrix_type, 0, 0, neqns, nrhs, 1, neqns);
DenseMtx_zero(mtxY) ;
if (matrix_type == SPOOLES_REAL)
{
	for (irow = 0 ; irow < neqns ; irow++) 
		DenseMtx_setRealEntry(mtxY, irow, 0, rhs2out[irow]);
		/* NOTE: there is no function to copy a vector into a column */
}
else
{
   fprintf(stderr, "\n SPOOLES:LU_MT_driver: real matrices only");
   return -1;
}
if ( msglvl > 2 ) {
   fprintf(msgFile, "\n\n rhs matrix in original ordering") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   -------------------------------------------------
   STEP 3 : find a low-fill ordering
   (1) create the Graph object
   (2) order the graph using multiple minimum degree
   -------------------------------------------------
*/
graph = Graph_new() ;
adjIVL = InpMtx_fullAdjacency(mtxA) ;
nedges = IVL_tsize(adjIVL) ;
Graph_init2(graph, 0, neqns, 0, nedges, neqns, nedges, adjIVL,
            NULL, NULL) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n graph of the input matrix") ;
   Graph_writeForHumanEye(graph, msgFile) ;
   fflush(msgFile) ;
}
frontETree = orderViaMMD(graph, seed, msglvl, msgFile) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n front tree from ordering") ;
   ETree_writeForHumanEye(frontETree, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   ----------------------------------------------------
   STEP 4: get the permutation, permute the front tree,
           permute the matrix and right hand side, and
           get the symbolic factorization
   ----------------------------------------------------
*/
oldToNewIV = ETree_oldToNewVtxPerm(frontETree) ;
oldToNew   = IV_entries(oldToNewIV) ;
newToOldIV = ETree_newToOldVtxPerm(frontETree) ;
newToOld   = IV_entries(newToOldIV) ;
ETree_permuteVertices(frontETree, oldToNewIV) ;
InpMtx_permute(mtxA, oldToNew, oldToNew) ;
if (  symmetryflag == SPOOLES_SYMMETRIC
   || symmetryflag == SPOOLES_HERMITIAN ) {
   InpMtx_mapToUpperTriangle(mtxA) ;
}
InpMtx_changeCoordType(mtxA, INPMTX_BY_CHEVRONS) ;
InpMtx_changeStorageMode(mtxA, INPMTX_BY_VECTORS) ;
DenseMtx_permuteRows(mtxY, oldToNewIV) ;
symbfacIVL = SymbFac_initFromInpMtx(frontETree, mtxA) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n old-to-new permutation vector") ;
   IV_writeForHumanEye(oldToNewIV, msgFile) ;
   fprintf(msgFile, "\n\n new-to-old permutation vector") ;
   IV_writeForHumanEye(newToOldIV, msgFile) ;
   fprintf(msgFile, "\n\n front tree after permutation") ;
   ETree_writeForHumanEye(frontETree, msgFile) ;
   fprintf(msgFile, "\n\n input matrix after permutation") ;
   InpMtx_writeForHumanEye(mtxA, msgFile) ;
   fprintf(msgFile, "\n\n right hand side matrix after permutation") ;
   DenseMtx_writeForHumanEye(mtxY, msgFile) ;
   fprintf(msgFile, "\n\n symbolic factorization") ;
   IVL_writeForHumanEye(symbfacIVL, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------
   STEP 5: setup the domain decomposition map
   ------------------------------------------
*/
if ( nthread > (nfront = ETree_nfront(frontETree)) ) {
   nthread = nfront ;
}
cumopsDV = DV_new() ;
DV_init(cumopsDV, nthread, NULL) ;
ownersIV = ETree_ddMap(frontETree, type, symmetryflag,
                       cumopsDV, 1./(2.*nthread)) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n map from fronts to threads") ;
   IV_writeForHumanEye(ownersIV, msgFile) ;
   fprintf(msgFile, "\n\n factor operations for each front") ;
   DV_writeForHumanEye(cumopsDV, msgFile) ;
   fflush(msgFile) ;
}
DV_free(cumopsDV) ;
/*--------------------------------------------------------------------*/
/*
   ------------------------------------------
   STEP 6: initialize the front matrix object
   ------------------------------------------
*/
frontmtx   = FrontMtx_new() ;
mtxmanager = SubMtxManager_new() ;
SubMtxManager_init(mtxmanager, LOCK_IN_PROCESS, 0) ;
FrontMtx_init(frontmtx, frontETree, symbfacIVL, type, symmetryflag, 
              FRONTMTX_DENSE_FRONTS, pivotingflag, LOCK_IN_PROCESS, 
              0, NULL, mtxmanager, msglvl, msgFile) ;
/*--------------------------------------------------------------------*/
/*
   -----------------------------------------------------
   STEP 7: compute the numeric factorization in parallel
   -----------------------------------------------------
*/
chvmanager = ChvManager_new() ;
ChvManager_init(chvmanager, LOCK_IN_PROCESS, 1) ;
DVfill(10, cpus, 0.0) ;
IVfill(20, stats, 0) ;
rootchv = FrontMtx_MT_factorInpMtx(frontmtx, mtxA, tau, droptol,
                                 chvmanager, ownersIV, lookahead, 
                                 &error, cpus, stats, msglvl, msgFile) ;
ChvManager_free(chvmanager) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n factor matrix") ;
   FrontMtx_writeForHumanEye(frontmtx, msgFile) ;
   fflush(msgFile) ;
}
if ( rootchv != NULL ) {
   fprintf(msgFile, "\n\n matrix found to be singular\n") ;
   exit(-1) ;
}
if ( error >= 0 ) {
   fprintf(msgFile, "\n\n fatal error at front %d", error) ;
   exit(-1) ;
}
/*--------------------------------------------------------------------*/
/*
   --------------------------------------
   STEP 8: post-process the factorization
   --------------------------------------
*/
FrontMtx_postProcess(frontmtx, msglvl, msgFile) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n factor matrix after post-processing") ;
   FrontMtx_writeForHumanEye(frontmtx, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   -------------------------------------------------------
   STEP 9: get the solve map object for the parallel solve
   -------------------------------------------------------
*/
solvemap = SolveMap_new() ;
SolveMap_ddMap(solvemap, symmetryflag, FrontMtx_upperBlockIVL(frontmtx),
               FrontMtx_lowerBlockIVL(frontmtx), nthread, ownersIV, 
               FrontMtx_frontTree(frontmtx), seed, msglvl, msgFile) ;
/*--------------------------------------------------------------------*/
/*
   --------------------------------------------
   STEP 10: solve the linear system in parallel
   --------------------------------------------
*/
mtxX = DenseMtx_new() ;
DenseMtx_init(mtxX, type, 0, 0, neqns, nrhs, 1, neqns) ;
DenseMtx_zero(mtxX) ;
FrontMtx_MT_solve(frontmtx, mtxX, mtxY, mtxmanager, solvemap,
                  cpus, msglvl, msgFile) ;
if ( msglvl > 1 ) {
   fprintf(msgFile, "\n\n solution matrix in new ordering") ;
   DenseMtx_writeForHumanEye(mtxX, msgFile) ;
   fflush(msgFile) ;
}
/*--------------------------------------------------------------------*/
/*
   --------------------------------------------------------
   STEP 11: permute the solution into the original ordering
   --------------------------------------------------------
*/
DenseMtx_permuteRows(mtxX, newToOldIV) ;
if ( msglvl > 0 ) {
   fprintf(msgFile, "\n\n solution matrix in original ordering") ;
   DenseMtx_writeForHumanEye(mtxX, msgFile) ;
   fflush(msgFile) ;
}

if (matrix_type == SPOOLES_REAL)
{
	for (irow = 0 ; irow < neqns ; irow++) 
		DenseMtx_realEntry(mtxX, irow, 0, rhs2out + irow);
		/* NOTE: there is no function to copy a column into a vector */
}
else
{
   fprintf(stderr, "\n SPOOLES:LU_MT_driver: real matrices only");
   return -1;
}
/*--------------------------------------------------------------------*/
/*
   -----------
   free memory
   -----------
*/

FrontMtx_free(frontmtx) ;
DenseMtx_free(mtxX) ;
DenseMtx_free(mtxY) ;
IV_free(newToOldIV) ;
IV_free(oldToNewIV) ;
InpMtx_free(mtxA) ;
ETree_free(frontETree) ;
IVL_free(symbfacIVL) ; 
SubMtxManager_free(mtxmanager) ;
Graph_free(graph) ;
SolveMap_free(solvemap) ;
IV_free(ownersIV) ;

/*--------------------------------------------------------------------*/
fclose(msgFile);
return(1) ; }
/*--------------------------------------------------------------------*/




