C/*****************************************************************************/
C/*                                                                           */
C/*   (C) Copyright IBM Corporation, 1997                                     */
C/*   (C) Copyright Regents of the University of Minnesota, 1997              */
C/*                                                                           */
C/*   parelimh1i.f                                                            */
C/*                                                                           */
C/*   Written by Anshul Gupta, IBM Corp.                                      */
C/*   Modified by Mahesh Joshi, U of MN.                                      */
C/*                                                                           */
C/*****************************************************************************/
C/*                                                                           */
C/* This code is meant to be used solely for educational, research, and       */
C/* benchmarking purposes by non-profit institutions and US government        */
C/* agencies only.  Use by any other organization requires prior written      */
C/* permission from both IBM Corporation and the University of Minnesota.     */
C/* The software may not be sold or redistributed.  One may make copies       */
C/* of the software or modify it for their use provided that the copies,      */
C/* modified or otherwise, are not sold or distributed, are used under the    */
C/* same terms and conditions, and this notice and any part of the source     */
C/* code that follows this notice are not separated.                          */
C/*                                                                           */
C/* As unestablished research software, this code is provided on an           */
C/* ``as is'' basis without warranty of any kind, either expressed or         */
C/* implied, including but not limited to implied warranties of               */
C/* merchantability and fitness for a particular purpose.  IBM does not       */
C/* warrant that the functions contained in this software will meet the       */
C/* user's requirements or that the operation of its routines will be         */
C/* uninterrupted or error-free.  Acceptance and use of this program          */
C/* constitutes the user's understanding that he/she will have no recourse    */
C/* to IBM for any actual or consequential damages, including, but not        */
C/* limited to, lost profits or savings, arising out of the use or inability  */
C/* to use these libraries.  Even if the user informs IBM of the possibility  */
C/* of such damages, IBM expects the user to accept the risk of any such      */
C/* harm, or the user shall not attempt to use these libraries for any        */
C/* purpose.                                                                  */
C/*                                                                           */
C/* The downloading, compiling, or executing any part of this software        */
C/* constitutes an implicit agreement to these terms.  These terms and        */
C/* conditions are subject to change at any time without prior notice.        */
C/*                                                                           */
C/*****************************************************************************/
C/* $Id: parelimh1i.f,v 1.1 2004-12-10 20:28:27 paklein Exp $ */
C/*****************************************************************************/

      subroutine PARELIMH1(wmem,vbuf_s,hbuf_s,vbuf_r,hbuf_r,supinds,
     1           tptrs,tinds,cinfo,stak,nstak,avals,aptrs,
     2           ainds,lvals,lptrs,linds,sup,stakptr,nptr,mask1,
     3           fptr,ldf,nrows,ncols,rsuptr,csuptr,myid,
     4           myleft,myright,myup,mydown,diagproc,bufsize,
     5           wmemsize,locinds,prof,dfopts,ifopts,comm)

      integer supinds(*),tptrs(3,0:*),tinds(*),stak(3,*),nstak(*)
      integer cinfo(0:*)
      double precision lvals(*),avals(*),dfopts(7)
      integer aptrs(2,0:*),ainds(*),lptrs(3,0:*),linds(*),sup(*)
      integer stakptr,nptr,mask1,fptr,ldf,nrows,ncols,comm
      integer rsuptr,csuptr,myid,myleft,myright,myup,mydown
      integer diagproc,bufsize,wmemsize,locinds(*)
      integer prof(4,*),ifopts(5)
      double precision wmem(*),vbuf_s(*),hbuf_s(*) 
      double precision vbuf_r(*),hbuf_r(*)

      integer rank, uptr, bottom, rsuptr_u, csuptr_u, buflen

      include 'mpif.h'
      integer mpistat(MPI_STATUS_SIZE),req(4),ierr
      integer statall(MPI_STATUS_SIZE,4)

      do ireq=1,4
        req(ireq) = MPI_REQUEST_NULL
      end do

      buflen = ishft(bufsize,3)
      node = nstak(nptr)
      nclim = csuptr + ncols
      nrlim = rsuptr + nrows
      newsup = 1

1000  rank = 0

      bottom = node
      j = tptrs(3,node)
      if (j .ne. 0) then
        if (sup(j) .eq. node) then
          if (sup(j+1) .eq. 1) then
            nptr = nptr - 1
            rank = 1
            if (nptr .gt. 0) node = nstak(nptr)
          end if
        else
          nptr = nptr - 1
          rank = 1
          if (nptr .gt. 0) node = nstak(nptr)
        end if
      end if
      if (rank .eq. 0) then
        index = iand(bottom,mask1)
        nptr = nptr - 1
        rank = 1
        if (nptr .gt. 0) then
          node = nstak(nptr)
          do while (iand(mask1,node) .eq. index)
            if (tptrs(3,node) .ne. 0) then
              nptr = nptr - 1
              rank = rank + 1
              if (nptr .gt. 0) node = nstak(nptr)
              goto 10
            end if
            nptr = nptr - 1
            rank = rank + 1
            node = nstak(nptr)
          end do
        end if
      end if

10    if (newsup .eq. 1) then
        newsup = 0
        if (cinfo(bottom) .ne. 1) then
          nprof = 0
          if (diagproc .ne. 1) then
            jj = 1
            newlocf = fptr
            i = csuptr
            j = rsuptr
            do while (i .lt. nclim)
              nprof = nprof + 1
              locf = newlocf
              prof(1,nprof) = newlocf
              newrank = 0
              inode = supinds(j)
              do while (i+newrank .lt. nclim)
                if (supinds(i+newrank) .gt. inode) goto 135
                newrank = newrank + 1
              end do
135           i = i + newrank
              prof(2,nprof) = newrank
              newlocf = newlocf + ldf * newrank
              if (i .lt. nclim) then
                jnode = supinds(i)
                k = 0
                do while (supinds(j+k) .lt. jnode)
                  k = k + 1
                end do
              else
                k = nrlim - j
              end if
              prof(3,nprof) = k
              prof(4,nprof) = jj
              j = j + k
              newlocf = newlocf + k
              jj = jj + newrank
            end do
          end if
        end if
      end if

      if (cinfo(bottom) .eq. 1) then

        locf = fptr
        j = rsuptr
        do 40 i = csuptr, csuptr + min0(rank,ncols) - 1
          jnode = supinds(i)
          jj = locf - j
          ii = j
          do 30 k = aptrs(1,jnode), aptrs(1,jnode)+aptrs(2,jnode)-1
            inode = ainds(k)
            do while (supinds(ii) .lt. inode)
              ii = ii + 1
            end do
            wmem(jj+ii) = wmem(jj+ii) + avals(k)
30        continue
          locf = locf + ldf + diagproc
          j = j + diagproc
40      continue

20      if (diagproc .eq. 1) then

          call dpack(wmem(fptr),vbuf_s,rank,ldf,rank)
          call dpotrf('l',rank,vbuf_s,rank,info)
        if (info.gt.0) goto 1
          if (myid .ne. mydown) then
            call mpi_send(vbuf_s,ishft(rank*rank,3),MPI_BYTE,mydown,
     +                    myid,comm,ierr)
          end if
          call dtrsm('R','L','T','N',nrows-rank,rank,1.d0,vbuf_s,rank,
     1         wmem(fptr+rank),ldf)

          locf = fptr + rank
          uptr = 1
          jj = 1
          do 50 i = 0, rank - 1
            jnode = supinds(i+csuptr)
            call mydc(rank-i,vbuf_s(uptr),lvals(lptrs(1,jnode)))
            call myddc(nrows-rank,wmem(locf),
     1           lvals(lptrs(1,jnode)+rank-i),hbuf_s(jj))
            locf = locf + ldf 
            jj = jj + nrows - rank
            uptr = uptr + rank + 1
50        continue
          call mpi_isend(hbuf_s,ishft(rank*(nrows-rank),3),MPI_BYTE,
     1         myright,myid,comm,req(1),ierr)

          i = i + csuptr
          j = rsuptr + rank
          jnode = supinds(i)
          if (i .lt. nclim) then
            do while (supinds(j) .lt. jnode)
              j = j + 1
            end do
          else
            j = nrlim
          end if
          newlocf = locf + j - rsuptr - rank
          nrows = nrlim - j 
          uptr = fptr + j - rsuptr
          rsuptr = j
          fptr = newlocf
          csuptr = csuptr + rank
          ncols = ncols - rank

          call mpi_isend(hbuf_s,ishft(rank*nrows,3),MPI_BYTE,mydown,
     1         myid,comm,req(2),ierr)

          call mpi_waitall(4,req,statall,ierr)
          if (nrows .gt. 0)
     1      call dsyrk('L','N',nrows,rank,-1.d0,wmem(uptr),ldf,
     2           1.d0,wmem(fptr),ldf)
        else

          msgtype = MPI_ANY_TAG
          call mpi_recv(vbuf_r,buflen,MPI_BYTE,myup,msgtype,
     +                  comm,mpistat,ierr)
          call mpi_get_count(mpistat,MPI_BYTE,nbytes,ierr)
          msgtype = mpistat(MPI_TAG)
          if (msgtype .ne. mydown) then
            call mpi_send(vbuf_r,nbytes,MPI_BYTE,mydown,msgtype,
     +                    comm,ierr)
          end if
          if (ncols .gt. 0) then
            call dtrsm('R','L','T','N',nrows,rank,1.d0,vbuf_r,rank,
     1           wmem(fptr),ldf)
          end if
          call dpack(wmem(fptr),hbuf_s,nrows,ldf,rank)
          call mpi_isend(hbuf_s,ishft(nrows*rank,3),MPI_BYTE,myright,
     +                   myid,comm,req(1),ierr)
          msgtype1 = MPI_ANY_TAG
          call mpi_irecv(vbuf_r,buflen,MPI_BYTE,myup,msgtype1,
     +                   comm,req(2),ierr)
          npending = 2
          nsent1 = 0
          do while (npending .gt. 0)
            call mpi_waitany(4,req,msgid,mpistat,ierr)
            call mpi_get_count(mpistat,MPI_BYTE,nbytes,ierr)
            if (msgid .eq. 2 .and. nsent1 .eq. 0) then
              ldb = ishft(nbytes/rank,-3)
              msgtype1 = mpistat(MPI_TAG)
              if (msgtype1 .ne. mydown) then
                call mpi_isend(vbuf_r,nbytes,MPI_BYTE,mydown,msgtype1,
     +                         comm,req(3),ierr)
                npending = npending + 1
              end if
            nsent1 = 1
            end if
            npending = npending - 1
          end do

          locf = fptr
          k = lptrs(2,supinds(csuptr))
          do 60 i = csuptr, csuptr + min0(ncols,rank) - 1
            call mydc(k,wmem(locf),lvals(lptrs(1,supinds(i))))
            locf = locf + ldf
60        continue

          uptr = fptr
          newlocf = locf
          csuptr = i
          if (rank .lt. ncols) then
            jj = 1
            jnode = supinds(i)
            j = rsuptr
            do while (supinds(j) .lt. jnode)
              j = j + 1
            end do
            newlocf = newlocf + j - rsuptr
            uptr = uptr + j - rsuptr
            fptr = newlocf
          end if
          nrows = nrlim - j
          rsuptr = j
          ncols = ncols - rank
          nprof = 0

          do while (i .lt. nclim)
            nprof = nprof + 1
            locf = newlocf
            prof(1,nprof) = newlocf
            newrank = 0
            inode = supinds(j)
            do while (i+newrank .lt. nclim)
              if (supinds(i+newrank) .gt. inode) goto 90
              newrank = newrank + 1
            end do
90          i = i + newrank 
            prof(2,nprof) = newrank
            newlocf = newlocf + ldf * newrank
            if (i .lt. nclim) then
              jnode = supinds(i)
              k = 0
              do while (supinds(j+k) .lt. jnode)
                k = k + 1
              end do
            else
              k = nrlim - j
            end if
            prof(3,nprof) = k
            prof(4,nprof) = jj
            call dgemm('N','T',nrlim-j,newrank,rank,-1.d0,
     1           wmem(uptr),ldf,vbuf_r(jj),ldb,1.d0,wmem(locf),ldf)
            j = j + k
            newlocf = newlocf + k
            jj = jj + newrank
            uptr = uptr + k
          end do
        end if
        if (ncols .lt. 0) then
          ncols = 0
          csuptr = nclim
        end if
        if (nrows .lt. 0 .or. ncols .eq. 0) then
          nrows = 0
          rsuptr = nrlim
        end if
      else

        if (diagproc .eq. 1) then

          msgtype = MPI_ANY_TAG 
          call mpi_recv(hbuf_r,buflen,MPI_BYTE,myleft,msgtype,
     +                  comm,mpistat,ierr)
          call mpi_get_count(mpistat,MPI_BYTE,nbytes,ierr)
          msgtype = mpistat(MPI_TAG)
          ldb = ishft(nbytes/rank,-3)

          uptr = 1 + ldb - nrows
          if (myid .ne. mydown) then
            if (ldb .eq. nrows) then
              call mpi_isend(hbuf_r,nbytes,MPI_BYTE,mydown,myid,
     +                       comm,req(1),ierr)
            else
              call dpack(hbuf_r(uptr),vbuf_s,nrows,ldb,rank)
              call mpi_isend(vbuf_s,ishft(nrows*rank,3),MPI_BYTE,mydown,
     1             myid,comm,req(1),ierr)
            end if
          end if
          if (myright .ne. msgtype) then
            call mpi_isend(hbuf_r,nbytes,MPI_BYTE,myright,msgtype,
     +                     comm,req(2),ierr)
          end if
          call mpi_waitall(4,req,statall,ierr)
         
          if (nrows .gt. 0)
     1      call dsyrk('L','N',nrows,rank,-1.d0,hbuf_r(uptr),ldb,
     2           1.d0,wmem(fptr),ldf)
        else

          msgtype1 = MPI_ANY_TAG
          msgtype2 = MPI_ANY_TAG
          call mpi_irecv(hbuf_r,buflen,MPI_BYTE,myleft,msgtype1,
     +                   comm,req(1),ierr)
          call mpi_irecv(vbuf_r,buflen,MPI_BYTE,myup,msgtype2,
     +                   comm,req(2),ierr)
          npending = 2
        nsent1 = 0
        nsent2 = 0
          do while (npending .ne. 0)
            call mpi_waitany(4,req,msgid,mpistat,ierr)
            call mpi_get_count(mpistat,MPI_BYTE,nbytes,ierr)
            npending = npending - 1
            if (msgid .eq. 1 .and. nsent1 .eq. 0) then
              lda = ishft(nbytes/rank,-3)
              msgtype1 = mpistat(MPI_TAG)

              if (msgtype1 .ne. myright) then
                call mpi_isend(hbuf_r,nbytes,MPI_BYTE,myright,msgtype1,
     +                         comm,req(3),ierr)
                npending = npending + 1
              end if
            nsent1 = 1
            end if
            if (msgid .eq. 2 .and. nsent2 .eq. 0) then
              ldb = ishft(nbytes/rank,-3)
              msgtype2 = mpistat(MPI_TAG)
              if (msgtype2 .ne. mydown) then
                call mpi_isend(vbuf_r,nbytes,MPI_BYTE,mydown,msgtype2,
     +                         comm,req(4),ierr)
                npending = npending + 1
              end if
              nsent2 = 1
            end if
          end do

          uptr = 1 + lda - nrows
          j = nrows
          do 130 i = 1, nprof
            call dgemm('N','T',j,prof(2,i),rank,-1.d0,
     1           hbuf_r(uptr),lda,vbuf_r(prof(4,i)),ldb,1.d0,
     2           wmem(prof(1,i)),ldf)
            j = j - prof(3,i)
            uptr = uptr + prof(3,i)
130       continue

        end if
      end if
   
      if (tptrs(3,node) .eq. 0) goto 1000 
      if (nptr .eq. 0) return 
      if (sup(tptrs(3,node)) .ne. node) goto 1000
      if (tptrs(2,node) .eq. 1) then
        newsup = 1
        uptr = fptr
        ncols_u = nclim
        csuptr_u = csuptr
        rsuptr_u = rsuptr
        stakptr = stakptr - 1
        ncols = stak(2,stakptr)
        nrows = stak(3,stakptr)
        i = tptrs(3,node) + 2
        csuptr = sup(i)
        rsuptr = csuptr + sup(i+1)
        fptr = wmemsize - ldf * (ncols - 1) - nrows + 1

        i = rsuptr_u
        j = nrlim
        k = rsuptr
        l = rsuptr + nrows

        locptr = -1
        do while (i .lt. j .and. k .lt. l)
          locptr = locptr + 2
          do while (i .lt. j .and. supinds(i) .eq. supinds(k))
            i = i + 1
            k = k + 1
          end do
          locinds(locptr) = k - 1
          if (i .lt. j) then
            do while (supinds(i) .ne. supinds(k))
              k = k + 1
            end do
            locinds(locptr+1) = k - 1
          else
            locinds(locptr+1) = l - 1
          end if
        end do

        locf = fptr
        nclim = ncols + csuptr
        nrlim = nrows + rsuptr
        jj = csuptr
        ii = rsuptr
        do while (jj .lt. nclim .and. csuptr_u .lt. ncols_u)
          do while (supinds(jj) .lt. supinds(csuptr_u))
            do while (supinds(ii) .lt. supinds(jj))
              ii = ii + 1
              locf = locf + 1
            end do
            do 220 i = 0, nrlim - ii - 1
              wmem(locf+i) = 0.d0
220         continue
            jj = jj + 1
            locf = locf + ldf
          end do
          do while (supinds(ii) .lt. supinds(jj))
            ii = ii + 1
            locf = locf + 1
          end do
          if (locf .eq. uptr) goto 1000
          i = ii
          locf = locf - ii
          j = uptr
          do 230 l = 1, locptr, 2
            j = j - i
            if (locf .eq. j) goto 310
            do 240 k = i, locinds(l)
              wmem(locf+k) = wmem(j+k)
240         continue
            j = j + k
            do 250 i = k, locinds(l+1)
              wmem(locf+i) = 0.d0
250         continue
230       continue
310       locf = locf + ldf + ii
          uptr = uptr + ldf
          csuptr_u = csuptr_u + 1
          jj = jj + 1
          do while (supinds(rsuptr_u) .lt. supinds(csuptr_u))
            rsuptr_u = rsuptr_u + 1
            uptr = uptr + 1
          end do
        end do
        do while (jj .lt. nclim)
          do while (supinds(ii) .lt. supinds(jj))
            ii = ii + 1
            locf = locf + 1
          end do
          do 210 i = 0, nrlim - ii - 1
            wmem(locf+i) = 0.d0
210       continue
          jj = jj + 1
          locf = locf + ldf
        end do
        goto 1000
      end if

      return
1     print *,myid,': Non positive definite matrix found at',
     1        bottom
      call mpi_abort(comm,27,ierr)
      end 
