/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * move.c
 *
 * This file contains functions that move the graph given a partition
 *
 * Started 11/22/96
 * George
 *
 * $Id: move.c,v 1.1.1.1 2004-10-07 16:05:25 paklein Exp $
 *
 */

#include <parmetis.h>

/*************************************************************************
* This function moves the graph, and returns a new graph.
* This routine can be called with or without performing refinement.
* In the latter case it allocates and computes lpwgts itself.
**************************************************************************/
GraphType *MoveGraph(CtrlType *ctrl, GraphType *graph, WorkSpaceType *wspace)
{
  int i, ii, j, jj, k, nvtxs, nparts;
  idxtype *xadj, *vwgt, *adjncy, *adjwgt, *mvtxdist;
  idxtype *where, *newlabel, *lpwgts, *gpwgts;
  idxtype *sgraph, *rgraph;
  KeyValueType *sinfo, *rinfo;
  GraphType *mgraph;

  nparts = ctrl->nparts;
  ASSERT(ctrl, nparts == ctrl->npes);

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  vwgt = graph->vwgt;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;
  where = graph->where;

  mvtxdist = idxmalloc(nparts+1, "MoveGraph: mvtxdist");

  /* Let's do a prefix scan to determine the labeling of the nodes given */
  lpwgts = wspace->pv1;
  gpwgts = wspace->pv2;
  sinfo = wspace->pepairs1;
  rinfo = wspace->pepairs2;
  for (i=0; i<nparts; i++)
    sinfo[i].key = sinfo[i].val = 0;

  /* Here we care about the count and not total weight (diff since graph may be weighted) */
  for (i=0; i<nvtxs; i++) {
    ASSERTP(ctrl, where[i] >= 0 && where[i] < ctrl->npes, (ctrl, "%d %d %d\n", i, where[i], ctrl->npes) );
    sinfo[where[i]].key++;
    sinfo[where[i]].val += xadj[i+1]-xadj[i];
  }
  for (i=0; i<nparts; i++)
    lpwgts[i] = sinfo[i].key;

  MPI_Scan((void *)lpwgts, (void *)gpwgts, nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);
  MPI_Allreduce((void *)lpwgts, (void *)mvtxdist, nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);

/*
  PrintVector(ctrl, nparts, 0, lpwgts, "Lpwgts");
  PrintVector(ctrl, nparts, 0, gpwgts, "Gpwgts");
  PrintVector(ctrl, nparts, 0, mvtxdist, "Mvtxdist");
*/

  MAKECSR(i, nparts, mvtxdist);

  /* gpwgts[i] will store the label of the first vertex for each domain in each processor */
  for (i=0; i<nparts; i++)
    gpwgts[i] = mvtxdist[i] + gpwgts[i] - lpwgts[i];  /* We were interested in an exclusive Scan */

  newlabel = idxmalloc(nvtxs+graph->nrecv, "MoveGraph: newlabel");

  for (i=0; i<nvtxs; i++) 
    newlabel[i] = gpwgts[where[i]]++;

  /* OK, now send the newlabel info to processors storing adjacent interface nodes */
  CommInterfaceData(ctrl, graph, newlabel, wspace->indices, newlabel+nvtxs);

  /* Now lets tell everybody what and from where he will get it. Assume nparts == npes */
  MPI_Alltoall((void *)sinfo, 2, IDX_DATATYPE, (void *)rinfo, 2, IDX_DATATYPE, ctrl->comm);

/*
  PrintPairs(ctrl, nparts, sinfo, "Sinfo");
  PrintPairs(ctrl, nparts, rinfo, "Rinfo");
*/

  /* Use lpwgts and gpwgts as pointers to where data will be received and send */
  lpwgts[0] = 0;  /* Send part */
  gpwgts[0] = 0;  /* Received part */
  for (i=0; i<nparts; i++) {
    lpwgts[i+1] = lpwgts[i] + 2*(sinfo[i].key + sinfo[i].val);
    gpwgts[i+1] = gpwgts[i] + 2*(rinfo[i].key + rinfo[i].val);
  }


  if (lpwgts[nparts]+gpwgts[nparts] > wspace->maxcore) {
    /* Adjust core memory, incase the graph was originally very memory unbalanced */
    free(wspace->core);
    wspace->maxcore = lpwgts[nparts]+4*gpwgts[nparts]; /* In spirit of the 8*nedges */
    wspace->core = idxmalloc(wspace->maxcore, "MoveGraph: wspace->core");
  }

  sgraph = wspace->core;
  rgraph = wspace->core + lpwgts[nparts];

  /* Issue the receives first */
  for (i=0; i<nparts; i++) {
    if (rinfo[i].key > 0) 
      MPI_Irecv((void *)(rgraph+gpwgts[i]), gpwgts[i+1]-gpwgts[i], IDX_DATATYPE, i, 1, ctrl->comm, ctrl->rreq+i);
    else
      ASSERT(ctrl, gpwgts[i+1]-gpwgts[i] == 0);
  }

  /* PrintVector(ctrl, nparts, 0, lpwgts, "Lpwgts"); */

  /* Assemble the graph to be send and send it */
  for (i=0; i<nvtxs; i++) {
    ii = lpwgts[where[i]];
    sgraph[ii++] = xadj[i+1]-xadj[i];
    sgraph[ii++] = vwgt[i]; 
    for (j=xadj[i]; j<xadj[i+1]; j++) {
      sgraph[ii++] = newlabel[adjncy[j]];
      sgraph[ii++] = adjwgt[j];
    }
    lpwgts[where[i]] = ii;
  }
  for (i=nparts; i>0; i--)
    lpwgts[i] = lpwgts[i-1];
  lpwgts[0] = 0;

  /* PrintVector(ctrl, nparts, 0, lpwgts, "Lpwgts"); */

  for (i=0; i<nparts; i++) {
    if (sinfo[i].key > 0)
      MPI_Isend((void *)(sgraph+lpwgts[i]), lpwgts[i+1]-lpwgts[i], IDX_DATATYPE, i, 1, ctrl->comm, ctrl->sreq+i);
    else
      ASSERT(ctrl, lpwgts[i+1]-lpwgts[i] == 0);
  }

#ifdef DMALLOC
  ASSERT(ctrl, dmalloc_verify(NULL) == DMALLOC_VERIFY_NOERROR);
#endif

  /* Wait for the send/recv to finish */
  for (i=0; i<nparts; i++) {
    if (sinfo[i].key > 0) 
      MPI_Wait(ctrl->sreq+i, &ctrl->status);
  }
  for (i=0; i<nparts; i++) {
    if (rinfo[i].key > 0) 
      MPI_Wait(ctrl->rreq+i, &ctrl->status); 
  }

  /* OK, now go and put the graph into GraphType Format */
  mgraph = CreateGraph();
  mgraph->maxvwgt = graph->maxvwgt;
  mgraph->gnvtxs = graph->gnvtxs;
  mgraph->level = 0;
  mgraph->nvtxs = mgraph->nedges = 0;
  for (i=0; i<nparts; i++) {
    mgraph->nvtxs += rinfo[i].key;
    mgraph->nedges += rinfo[i].val;
  }
  nvtxs = mgraph->nvtxs;
  xadj = mgraph->xadj = idxmalloc(nvtxs+1, "MoveGraph: mgraph->xadj");
  vwgt = mgraph->vwgt = idxmalloc(nvtxs, "MoveGraph: mgraph->vwgt");
  adjncy = mgraph->adjncy = idxmalloc(mgraph->nedges, "MoveGraph: mgraph->adjncy");
  adjwgt = mgraph->adjwgt = idxmalloc(mgraph->nedges, "MoveGraph: mgraph->adjwgt");
  mgraph->vtxdist = mvtxdist;

  for (jj=ii=i=0; i<nvtxs; i++) {
    xadj[i] = rgraph[ii++];
    vwgt[i] = rgraph[ii++]; 
    for (j=0; j<xadj[i]; j++) {
      adjncy[jj] = rgraph[ii++];
      adjwgt[jj++] = rgraph[ii++];
    }
  }
  MAKECSR(i, nvtxs, xadj);

  ASSERTP(ctrl, jj == mgraph->nedges, (ctrl, "%d %d\n", jj, mgraph->nedges));
  ASSERTP(ctrl, ii == gpwgts[nparts], (ctrl, "%d %d %d %d %d\n", ii, gpwgts[nparts], jj, mgraph->nedges, nvtxs));

  free(newlabel);

  /* CheckMGraph(ctrl, mgraph); */

  return mgraph;
}



/*************************************************************************
* This function is used to transfer information from the moved graph
* back to the original graph. The information is transfered from array 
* minfo to array info. The routine assumes that graph->where is left intact
* and it is used to get the inverse mapping information.
* The routine assumes that graph->where corresponds to a npes-way partition.
**************************************************************************/
void ProjectInfoBack(CtrlType *ctrl, GraphType *graph, idxtype *info, idxtype *minfo, WorkSpaceType *wspace)
{
  int i, ii, j, jj, k, nvtxs, nparts;
  idxtype *where, *auxinfo, *sinfo, *rinfo;

  nparts = ctrl->npes;

  nvtxs = graph->nvtxs;
  where = graph->where;

  sinfo = wspace->pv1;
  rinfo = wspace->pv2;
  auxinfo = wspace->core;

  /* Find out in rinfo how many entries are received per partition */
  idxset(nparts, 0, rinfo);
  for (i=0; i<nvtxs; i++)
    rinfo[where[i]]++;

  /* The rinfo are transposed and become the sinfo for the back-projection */
  MPI_Alltoall((void *)rinfo, 1, IDX_DATATYPE, (void *)sinfo, 1, IDX_DATATYPE, ctrl->comm);

  MAKECSR(i, nparts, sinfo);
  MAKECSR(i, nparts, rinfo);

  /*-----------------------------------------------------------------
   * Now, go and send back the minfo 
   -----------------------------------------------------------------*/
  for (i=0; i<nparts; i++) {
    if (rinfo[i+1]-rinfo[i] > 0)
      MPI_Irecv((void *)(auxinfo+rinfo[i]), rinfo[i+1]-rinfo[i], IDX_DATATYPE, i, 1, ctrl->comm, ctrl->rreq+i);
  }

  for (i=0; i<nparts; i++) {
    if (sinfo[i+1]-sinfo[i] > 0) 
      MPI_Isend((void *)(minfo+sinfo[i]), sinfo[i+1]-sinfo[i], IDX_DATATYPE, i, 1, ctrl->comm, ctrl->sreq+i);
  }

  /* Wait for the send/recv to finish */
  for (i=0; i<nparts; i++) {
    if (rinfo[i+1]-rinfo[i] > 0)
      MPI_Wait(ctrl->rreq+i, &ctrl->status);
  }
  for (i=0; i<nparts; i++) {
    if (sinfo[i+1]-sinfo[i] > 0)
      MPI_Wait(ctrl->sreq+i, &ctrl->status);
  }

  /* Scatter the info received in auxinfo back to info. */
  for (i=0; i<nvtxs; i++) 
    info[i] = auxinfo[rinfo[where[i]]++];

}






/*************************************************************************
* This function is used to convert a partition vector to a permutation
* vector.
**************************************************************************/
void FindVtxPerm(CtrlType *ctrl, GraphType *graph, idxtype *perm, WorkSpaceType *wspace)
{
  int i, ii, j, jj, k, nvtxs, nparts;
  idxtype *xadj, *vwgt, *adjncy, *adjwgt, *mvtxdist;
  idxtype *where, *lpwgts, *gpwgts;

  nparts = ctrl->nparts;

  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  vwgt = graph->vwgt;
  adjncy = graph->adjncy;
  adjwgt = graph->adjwgt;
  where = graph->where;

  mvtxdist = idxmalloc(nparts+1, "MoveGraph: mvtxdist");

  /* Let's do a prefix scan to determine the labeling of the nodes given */
  lpwgts = wspace->pv1;
  gpwgts = wspace->pv2;

  /* Here we care about the count and not total weight (diff since graph may be weighted */
  idxset(nparts, 0, lpwgts);
  for (i=0; i<nvtxs; i++) 
    lpwgts[where[i]]++;

  MPI_Scan((void *)lpwgts, (void *)gpwgts, nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);
  MPI_Allreduce((void *)lpwgts, (void *)mvtxdist, nparts, IDX_DATATYPE, MPI_SUM, ctrl->comm);

  MAKECSR(i, nparts, mvtxdist);

  for (i=0; i<nparts; i++)
    gpwgts[i] = mvtxdist[i] + gpwgts[i] - lpwgts[i];  /* We were interested in an exclusive Scan */

  for (i=0; i<nvtxs; i++) 
    perm[i] = gpwgts[where[i]]++;

  free(mvtxdist);

}




CheckMGraph(CtrlType *ctrl, GraphType *graph)
{
  int i, j, jj, k, nvtxs, firstvtx, lastvtx;
  idxtype *xadj, *adjncy, *vtxdist;


  nvtxs = graph->nvtxs;
  xadj = graph->xadj;
  adjncy = graph->adjncy;
  vtxdist = graph->vtxdist;

  firstvtx = vtxdist[ctrl->mype];
  lastvtx = vtxdist[ctrl->mype+1];

  for (i=0; i<nvtxs; i++) {
    for (j=xadj[i]; j<xadj[i+1]; j++) {
      ASSERT(ctrl, firstvtx+i != adjncy[j]);
      if (adjncy[j] >= firstvtx && adjncy[j] < lastvtx) {
        k = adjncy[j]-firstvtx;
        for (jj=xadj[k]; jj<xadj[k+1]; jj++) {
          if (adjncy[jj] == firstvtx+i)
            break;
        }
        if (jj == xadj[k+1])
          myprintf(ctrl, "(%d %d) but not (%d %d)\n", firstvtx+i, k, k, firstvtx+i);
      }
    }
  }
}
 



