/*====================================================================
 * ------------------------
 * | CVS File Information |
 * ------------------------
 *
 * $RCSfile: md_wrap_scalar_c.c,v $
 *
 * $Author: paklein $
 *
 * $Date: 2003-02-28 01:19:20 $
 *
 * $Revision: 1.1 $
 *
 * $Name: not supported by cvs2svn $
 *====================================================================*/

/*******************************************************************************
 * Copyright 1995, Sandia Corporation.  The United States Government retains a *
 * nonexclusive license in this software as prescribed in AL 88-1 and AL 91-7. *
 * Export of this program may require a license from the United States         *
 * Government.                                                                 *
 ******************************************************************************/

#include "az_aztec_defs.h"
#ifdef AZTEC_MPI
#include <mpi.h>
#else
#define MPI_Request int
#endif

#include <stdlib.h>
#include <stdio.h>

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
extern void get_parallel_info(int *proc, int *nprocs, int *dim);
extern int md_read(char *buf, int bytes, int *source, int *type, int *flag);
extern int md_write(char *buf, int bytes, int dest, int type, int *flag);
extern int md_wrap_iread(void *buf, int bytes, int *source, int *type,
                  MPI_Request *request);
extern int md_wrap_write(void *buf, int bytes, int dest, int type, int *flag);
extern int md_wrap_wait(void *buf, int bytes, int *source, int *type, int *flag,
                 MPI_Request *request);
extern int md_wrap_iwrite(void *buf, int bytes, int dest, int type, int *flag,
                  int *request);

void get_parallel_info(int *proc, int *nprocs, int *dim)

{
  *proc   = 0;
  *nprocs = 1;
  *dim    = 0;

} /* get_parallel_info */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*ARGSUSED*/

int md_read(char *buf, int bytes, int *source, int *type, int *flag)

{
  return bytes;

} /* md_read */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*ARGSUSED*/

int md_write(char *buf, int bytes, int dest, int type, int *flag)

{
  return 0;

} /* md_write */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*ARGSUSED*/

int md_wrap_iread(void *buf, int bytes, int *source, int *type,
                  MPI_Request *request)

/*******************************************************************************

  Machine dependent wrapped message-reading communication routine for the
  Intel.  This routine is a simple no-op but is used in order to provide
  compatibility with the MPI communication routine order.

  Author:          Scott A. Hutchinson, SNL, 9221
  =======

  Return code:     int
  ============

  Parameter list:
  ===============

  buf:             Beginning address of data to be sent.

  bytes:           Length of message in bytes.

  source:          Source processor number.

  type:            Message type

*******************************************************************************/

{

  return 0;

} /* md_wrap_iread */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*ARGSUSED*/

int md_wrap_write(void *buf, int bytes, int dest, int type, int *flag)

/*******************************************************************************

  Machine dependent wrapped message-sending communication routine for the
  Intel.  This routine is exactly the same as md_write.

  Author:          Scott A. Hutchinson, SNL, 9221
  =======

  Return code:     int
  ============

  Parameter list:
  ===============

  buf:             Beginning address of data to be sent.

  bytes:           Length of message in bytes.

  dest:            Destination processor number.

  type:            Message type

  flag:

*******************************************************************************/

{

  return 0;

} /* md_wrap_write */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*ARGSUSED*/

int md_wrap_wait(void *buf, int bytes, int *source, int *type, int *flag,
                 MPI_Request *request)

/*******************************************************************************

  Machine dependent wrapped message-wait communication routine for the Intel.
  This routine is identical to md_read but is put here in order to be compatible
  with the order required to do MPI communication.

  Author:          Scott A. Hutchinson, SNL, 9221
  =======

  Return code:     int
  ============

  Parameter list:
  ===============

  buf:             Beginning address of data to be sent.

  bytes:           Length of message in bytes.
  dest:            Destination processor number.

  type:            Message type

  flag:

*******************************************************************************/

{

  return bytes;

} /* md_wrap_wait */

int md_wrap_iwrite(void *buf, int bytes, int dest, int type, int *flag,
                  int *request)
{
int ret_info;
 
ret_info = md_wrap_write(buf, bytes, dest, type, flag);
return(ret_info); 
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

int md_wrap_request_free(MPI_Request *request)

/*******************************************************************************

  Machine dependent wrapped request object deletion routine. 
  (Trivial function except for MPI version).

  Author:          Michael A. Heroux, SNL, 9214
  =======

  Return code:     int
  ============

  Parameter list:
  ===============

  request:           Pointer to an existing request object that will be freed.

*******************************************************************************/
{

  int err = 0;
  return err;

} /* md_wrap_request_free */

