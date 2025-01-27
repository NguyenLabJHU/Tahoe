/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * order.c
 *
 * This file contains the driving routines for the multilevel ordering algorithm
 *
 * Started 5/3/97
 * George
 *
 * $Id: order.c,v 1.1.1.1 2004-10-07 16:05:25 paklein Exp $
 *
 */

#define DEBUG_ORDER_

#include <parmetis.h>

/*************************************************************************
* This is the top level ordering routine
**************************************************************************/
void MultilevelOrder(CtrlType *ctrl, GraphType *graph, idxtype *order, idxtype *sizes, WorkSpaceType *wspace)
{
  int i, j, k, nparts, nvtxs, npes;
  idxtype *perm, *lastnode, *morder, *porder;
  GraphType *mgraph;

  npes = ctrl->npes;
  nvtxs = graph->nvtxs;

  perm = idxmalloc(nvtxs, "MultilevelOrder: perm");
  lastnode = idxsmalloc(4*npes, -1, "MultilevelOrder: lastnode");

  for (i=0; i<nvtxs; i++) 
    perm[i] = i;
  lastnode[2] = graph->gnvtxs;

  idxset(nvtxs, -1, order);

  sizes[0] = 2*npes-1;

  graph->where = idxsmalloc(nvtxs, 0, "MultilevelOrder: graph->where");

  for (nparts=2; nparts<=ctrl->npes; nparts*=2) {
    ctrl->nparts = nparts;

    Order_Partition(ctrl, graph, wspace);

    LabelSeparators(ctrl, graph, lastnode, perm, order, sizes, wspace);

    CompactGraph(ctrl, graph, perm, wspace);

    if (ctrl->CoarsenTo < 100*nparts) {
      ctrl->CoarsenTo = 1.5*ctrl->CoarsenTo;
      graph->maxvwgt = 2*graph->maxvwgt/3;
    }
    ctrl->CoarsenTo = amin(ctrl->CoarsenTo, graph->gnvtxs-1);
  }

  /*-----------------------------------------------------------------
   / Move the graph so that each processor gets its partition 
   -----------------------------------------------------------------*/
  IFSET(ctrl->dbglvl, DBG_TIME, MPI_Barrier(ctrl->comm));
  IFSET(ctrl->dbglvl, DBG_TIME, starttimer(ctrl->MoveTmr));

  SetUp(ctrl, graph, wspace);
  mgraph = MoveGraph(ctrl, graph, wspace);

  /* Fill in the sizes[] array for the local part. Just the vtxdist of the mgraph */
  for (i=0; i<npes; i++)
    sizes[i] = mgraph->vtxdist[i+1]-mgraph->vtxdist[i];

  porder = idxmalloc(graph->nvtxs, "MultilevelOrder: porder");
  morder = idxmalloc(mgraph->nvtxs, "MultilevelOrder: morder");

  IFSET(ctrl->dbglvl, DBG_TIME, MPI_Barrier(ctrl->comm));
  IFSET(ctrl->dbglvl, DBG_TIME, stoptimer(ctrl->MoveTmr));

  /* Find the local ordering */
  LocalNDOrder(ctrl, mgraph, morder, lastnode[2*(ctrl->npes+ctrl->mype)]-mgraph->nvtxs, wspace);

  /* Project the ordering back to the before-move graph */
  ProjectInfoBack(ctrl, graph, porder, morder, wspace);

  /* Copy the ordering from porder to order using perm */
  for (i=0; i<graph->nvtxs; i++) {
    ASSERT(ctrl, order[perm[i]] == -1);
    order[perm[i]] = porder[i];
  }

  FreeGraph(mgraph);
  GKfree(&perm, &lastnode, &porder, &morder, LTERM);

  /* PrintVector(ctrl, 2*npes-1, 0, sizes, "SIZES"); */
}


/*************************************************************************
* This function is used to assign labels to the nodes in the separators
* It uses the appropriate entry in the lastnode array to select label
* boundaries and adjusts it for the next level
**************************************************************************/
void LabelSeparators(CtrlType *ctrl, GraphType *graph, idxtype *lastnode, idxtype *perm, idxtype *order, idxtype *sizes, WorkSpaceType *wspace)
{
  int i, j, k, nvtxs, nparts, firstlabel, sid;
  idxtype *where, *lpwgts, *gpwgts, *sizescan;

  nparts = ctrl->nparts;

  nvtxs = graph->nvtxs;
  where = graph->where;
  lpwgts = graph->lpwgts;
  gpwgts = graph->gpwgts;

  /* Compute the local size of the separator. This is required in case the 
   * graph has vertex weights */
  idxset(2*nparts, 0, lpwgts);
  for (i=0; i<nvtxs; i++) 
    lpwgts[where[i]]++;

  sizescan = idxmalloc(2*nparts, "LabelSeparators: sizescan");

  /* Perform a Prefix scan of the separator sizes to determine the boundaries */
  MPI_Scan((void *)lpwgts, (void *)sizescan, 2*nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);
  MPI_Allreduce((void *)lpwgts, (void *)gpwgts, 2*nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);

#ifdef DEBUG_ORDER
  PrintVector(ctrl, 2*nparts, 0, lpwgts, "Lpwgts");
  PrintVector(ctrl, 2*nparts, 0, sizescan, "SizeScan");
  PrintVector(ctrl, 2*nparts, 0, lastnode, "LastNode");
#endif

  /* Fillin the sizes[] array */
  for (i=nparts-2; i>=0; i-=2) 
    sizes[--sizes[0]] = gpwgts[nparts+i];

  if (ctrl->dbglvl&DBG_INFO) {
    if (ctrl->mype == 0) {
      printf("SepSizes: ");
      for (i=0; i<nparts; i+=2)
        printf(" %d [%d %d]", gpwgts[nparts+i], gpwgts[i], gpwgts[i+1]);
      printf("\n");
    }
    MPI_Barrier(ctrl->comm);
  }

  for (i=0; i<2*nparts; i++)
    sizescan[i] -= lpwgts[i];

  for (i=0; i<nvtxs; i++) {
    if (where[i] >= nparts) {
      sid = where[i];
      sizescan[sid]++;
      ASSERT(ctrl, order[perm[i]] == -1);
      order[perm[i]] = lastnode[sid] - sizescan[sid];
      /* myprintf(ctrl, "order[%d] = %d, %d\n", perm[i], order[perm[i]], sid); */
    }
  }

  /* Update lastnode array */
  idxcopy(2*nparts, lastnode, sizescan);
  for (i=0; i<nparts; i+=2) {
    lastnode[2*nparts+2*i] = sizescan[nparts+i]-gpwgts[nparts+i]-gpwgts[i+1];
    lastnode[2*nparts+2*(i+1)] = sizescan[nparts+i]-gpwgts[nparts+i];
  }

  free(sizescan);
}




/*************************************************************************
* This function compacts a graph by removing the vertex separator
**************************************************************************/
void CompactGraph(CtrlType *ctrl, GraphType *graph, idxtype *perm, WorkSpaceType *wspace)
{
  int i, ii, j, k, l, nvtxs, cnvtxs, cfirstvtx, nparts, npes; 
  idxtype *xadj, *ladjncy, *adjwgt, *vtxdist, *where;
  idxtype *cmap, *cvtxdist, *newwhere;

  nparts = ctrl->nparts;
  npes = ctrl->npes;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  ladjncy = graph->adjncy;
  adjwgt = graph->adjwgt;
  where = graph->where;
  cmap = graph->cmap;

  vtxdist = graph->vtxdist;

  /*************************************************************
  * Construct the cvtxdist of the contracted graph. Uses the fact
  * that lpwgts stores the local non separator vertices.
  **************************************************************/
  cvtxdist = wspace->pv1;
  cnvtxs = cvtxdist[npes] = idxsum(nparts, graph->lpwgts);

  MPI_Allgather((void *)(cvtxdist+npes), 1, IDX_DATATYPE, (void *)cvtxdist, 1, IDX_DATATYPE, ctrl->comm);
  MAKECSR(i, npes, cvtxdist);

#ifdef DEBUG_ORDER
  PrintVector(ctrl, npes+1, 0, cvtxdist, "cvtxdist");
#endif


  /*************************************************************
  * Construct the cmap vector 
  **************************************************************/
  cfirstvtx = cvtxdist[ctrl->mype];

  /* Create the cmap of what you know so far locally */
  cnvtxs = 0;
  for (i=0; i<nvtxs; i++) {
    if (where[i] < nparts) {
      perm[cnvtxs] = perm[i];
      cmap[i] = cfirstvtx + cnvtxs++;
    }
  }

  CommInterfaceData(ctrl, graph, cmap, wspace->indices, cmap+nvtxs);


  /*************************************************************
  * Finally, compact the graph
  **************************************************************/
  newwhere = idxmalloc(cnvtxs, "CompactGraph: newwhere");
  cnvtxs = l = 0;
  for (i=0; i<nvtxs; i++) {
    if (where[i] < nparts) {
      for (j=xadj[i]; j<xadj[i+1]; j++) {
        if (where[i] == where[ladjncy[j]]) {
          ladjncy[l] = cmap[ladjncy[j]];
          adjwgt[l++] = adjwgt[j];
        }
#ifdef DEBUG_ORDER
        else if (where[ladjncy[j]] < nparts)
          printf("It seems that the separation has failed: %d %d\n", where[i], where[ladjncy[j]]);
#endif
      }

      xadj[cnvtxs] = l;
      graph->vwgt[cnvtxs] = graph->vwgt[i];
      newwhere[cnvtxs] = where[i];
      cnvtxs++;
    }
  }
  for (i=cnvtxs; i>0; i--)
    xadj[i] = xadj[i-1];
  xadj[0] = 0;

  GKfree(&graph->match, &graph->cmap, &graph->lperm, &graph->where, &graph->label, &graph->rinfo,
         &graph->nrinfo, &graph->lpwgts, &graph->gpwgts, &graph->sepind, &graph->peind,
         &graph->sendptr, &graph->sendind, &graph->recvptr, &graph->recvind, 
         &graph->imap, &graph->rlens, &graph->slens, &graph->rcand, &graph->pexadj, 
         &graph->peadjncy, &graph->peadjloc, LTERM);
 
  graph->nvtxs = cnvtxs;
  graph->nedges = l;
  graph->gnvtxs = cvtxdist[npes];
  idxcopy(npes+1, cvtxdist, graph->vtxdist);
  graph->where = newwhere;

}


/*************************************************************************
* This function orders the locally stored graph using MMD. 
* The vertices will be ordered from firstnode onwards.
**************************************************************************/
void LocalNDOrder(CtrlType *ctrl, GraphType *graph, idxtype *order, int firstnode, WorkSpaceType *wspace)
{
  int i, j, k, nvtxs, firstvtx, lastvtx, nofsub;
  idxtype *xadj, *adjncy;
  idxtype *perm, *iperm;
  int numflag=0, options[10];

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;

  firstvtx = graph->vtxdist[ctrl->mype];
  lastvtx = graph->vtxdist[ctrl->mype+1];

  /* Relabel the vertices so that they are in local index space */
  for (i=0; i<nvtxs; i++) {
    for (j=xadj[i]; j<xadj[i+1]; j++) {
      ASSERT(ctrl, adjncy[j]>=firstvtx && adjncy[j]<lastvtx);
      adjncy[j] -= firstvtx;
    }
  }

  ASSERT(ctrl, 2*(nvtxs+5) < wspace->maxcore);

  perm = wspace->core;
  iperm = perm + nvtxs + 5;

  options[0] = 0;
  METIS_NodeND(&nvtxs, xadj, adjncy, &numflag, options, perm, iperm);

  for (i=0; i<nvtxs; i++) {
    ASSERT(ctrl, iperm[i]>=0 && iperm[i]<nvtxs);
    order[i] = firstnode+iperm[i];
  }

}
