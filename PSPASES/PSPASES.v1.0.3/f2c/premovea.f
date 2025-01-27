
C/*****************************************************************************/
C/*                                                                           */
C/*   (C) Copyright IBM Corporation, 1997                                     */
C/*   (C) Copyright Regents of the University of Minnesota, 1997              */
C/*                                                                           */
C/*   premovea.f                                                              */
C/*                                                                           */
C/*   Written by Mahesh Joshi, U of MN.                                       */
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
C/* $Id: premovea.f,v 1.3 2004-12-30 00:07:22 paklein Exp $ */
C/*****************************************************************************/

      subroutine premovea(N,dd,pp,lgblk,myid,mynnodes,rowdist,order,
     +                  sizes,mybeginleaf,pasize,aptrs,ainds,tainds,
     +                  parent,gorder,temparr2,wrkint,ranmasks,
     +                  whichsnode,maxnzpercol,checksymm,sortinds,comm,
     +                  sendinds,sendsizs,recvinds,recvsizs,
     +                  iwillsend_inds2, iwillreceive_inds2)

      implicit none

      include 'mpif.h'

      integer N,dd,pp,lgblk,myid,mynnodes,pasize,comm
      integer rowdist(0:*),order(0:*),sizes(0:*),ranmasks(5,0:*)
      integer aptrs(2,0:*),ainds(*),tainds(*),wrkint(0:*)
      integer gorder(0:N-1),temparr2(0:*),parent(0:*)
      integer whichsnode(0:*),maxnzpercol,checksymm,sortinds

C     integer, allocatable :: sendinds(:),sendsizs(:)
      integer iwillsend_inds2
      integer sendinds(*),sendsizs(*)
      dimension sendinds(0:iwillsend_inds2-1)
      dimension sendsizs(0:iwillsend_inds2-1)      

C     integer, allocatable :: recvinds(:),recvsizs(:)
      integer iwillreceive_inds2
      integer recvinds(*),recvsizs(*)
      dimension recvinds(0:iwillreceive_inds2-1)
      dimension recvsizs(0:iwillreceive_inds2-1)      

      integer proc,rbits,cbits,pgrsize,pgcsize
      integer tppr,tppc
      integer ierr,level,nseps,sepindex,nodebegin,sepbegin,snodes
      integer sepoffs,gbits,gsize,bmaskr,bmaskc,row,col
      integer mybeginleaf,nodeend
      integer kr,kc,i,j,k,l,m,ptr_r,fptr_r,ptr_c
      integer iwillsend_inds,is1
      integer iwillreceive_inds,ptr_sendinds
      integer lend,m1,ml,mk,kend,nstree
      logical ifound

      integer psci,psdi,prci,prdi,pscs,psds,prcs,prds
      integer ppr,ppc,ppg,psi,pps,psts,psnb

      !non-overlapping wrkint storage
      psci = 0
      psdi = pp
      prci = 2*pp
      prdi = 3*pp
      pscs = 4*pp
      psds = 5*pp
      prcs = 6*pp
      prds = 7*pp
      ppr  = 8*pp
      ppc  = 9*pp
      ppg  = 10*pp

      !overlapping wrkint storage
      psi  = psci
      pps  = psdi
      psts = psdi
      psnb = prdi

      maxnzpercol = 0
      do i=0,mynnodes-1
        k = aptrs(1,i)
        l = aptrs(2,i)
        maxnzpercol = max(maxnzpercol,l)
        do j=0,l-1
          tainds(k+j) = ainds(k+j)
        end do
        if(sortinds.eq.1 .and. checksymm.eq.1) then
          call ikeysortf(l,tainds(k),temparr2)
        end if
      end do

      if(checksymm.eq.1) then
*      check for symmetry of the input matrix.
      
      tppr =  0
      tppc =  mynnodes

      do proc=0,pp-1
        wrkint(psci+proc) = 0
        do i=rowdist(proc),rowdist(proc+1)-1
          temparr2(tppc+i) = proc
        end do
      end do

      do i=0,mynnodes-1
        k = aptrs(1,i)
        l = aptrs(2,i)
        m = 0
        do while (tainds(k+m).le.i+rowdist(myid))
          m = m+1
        end do
        temparr2(tppr+i) = m
        do m=k+m,k+l-1
          proc = temparr2(tppc+tainds(m))
          wrkint(psci+proc) = wrkint(psci+proc)+1
        end do
      end do

      wrkint(psdi) = 0
      iwillsend_inds = wrkint(psci)
      do proc=1,pp-1
        iwillsend_inds = iwillsend_inds+wrkint(psci+proc)
        wrkint(psdi+proc) = wrkint(psdi+proc-1)+wrkint(psci+proc-1)
        wrkint(psci+proc-1) = 0
      end do
      wrkint(psci+pp-1) = 0

C     allocate(sendinds(0:iwillsend_inds-1),stat=is1)
      if(is1.ne.0) then
        print *,'Error in allocate 1'
        call mpi_abort(comm,1,ierr)
      end if
C     allocate(sendsizs(0:iwillsend_inds-1),stat=is1)
      if(is1.ne.0) then
        print *,'Error in allocate 2'
        call mpi_abort(comm,1,ierr)
      end if

      do i=0,mynnodes-1
        k = aptrs(1,i)
        l = aptrs(2,i)
        m = temparr2(tppr+i)
        do m=k+m,k+l-1
          proc = temparr2(tppc+tainds(m))
          ptr_sendinds = wrkint(psdi+proc)+wrkint(psci+proc)
          sendinds(ptr_sendinds) = tainds(m)
          sendsizs(ptr_sendinds) = i+rowdist(myid)
          wrkint(psci+proc) = wrkint(psci+proc)+1
        end do
      end do

      call mpi_alltoall(wrkint(psci),1,MPI_INTEGER,wrkint(prci),
     +                  1,MPI_INTEGER,comm,ierr)

      wrkint(prdi) = 0
      iwillreceive_inds = wrkint(prci)
      do proc=1,pp-1
        iwillreceive_inds = iwillreceive_inds+wrkint(prci+proc)
        wrkint(prdi+proc) = wrkint(prdi+proc-1)+wrkint(prci+proc-1)
      end do

C     allocate(recvinds(0:iwillreceive_inds-1),stat=is1)
      if(is1.ne.0) then
        print *,'Error in allocate 4'
        call mpi_abort(comm,1,ierr)
      end if
C     allocate(recvsizs(0:iwillreceive_inds-1),stat=is1)
      if(is1.ne.0) then
        print *,'Error in allocate 5'
        call mpi_abort(comm,1,ierr)
      end if

      call mpi_alltoallv(sendinds,wrkint(psci),
     +                   wrkint(psdi),MPI_INTEGER,recvinds,
     +                   wrkint(prci),wrkint(prdi),
     +                   MPI_INTEGER,comm,ierr)

      call mpi_alltoallv(sendsizs,wrkint(psci),
     +                   wrkint(psdi),MPI_INTEGER,recvsizs,
     +                   wrkint(prci),wrkint(prdi),
     +                   MPI_INTEGER,comm,ierr)

      do i=0,mynnodes-1
        temparr2(i) = 0
      end do

      do i=0,iwillreceive_inds-1
        j = recvinds(i)-rowdist(myid)
        k = recvsizs(i)
        l = aptrs(1,j)
        m = temparr2(j)
        if(tainds(l+m).ne.k) then
          print *,'Matrix found to be unsymmetric in indices.'
          print *,'Col index ',k,' is not found in row',recvinds(i)
          call mpi_abort(comm,0,ierr)
        end if
        temparr2(j) = temparr2(j)+1
      end do

C     deallocate(sendinds)
C     deallocate(sendsizs)
C     deallocate(recvinds)
C     deallocate(recvsizs)

      end if 

*      collect global order with an allgather operation.

      do proc=0,pp-1
        wrkint(proc) = rowdist(proc+1)-rowdist(proc)
      end do

      call mpi_allgatherv(order,mynnodes,MPI_INTEGER,
     +                    gorder,wrkint,rowdist,MPI_INTEGER,
     +                    comm,ierr)

      do i=0,N-1
        temparr2(gorder(i)) = i
      end do

*      adjust sizes array to ensure non-zero size supernodes.

      do i=0,N-1
        temparr2(N+i) = i
      end do

      do level = 0,dd
        wrkint(psi+level) = 0
      end do
      nseps = ishft(pp,1)-1
      sepindex = 0
      level = dd
      nodebegin = 0

      nstree = 0
      do snodes=0,nseps-1

        sepbegin = nseps-(ishft(1,level+1)-1)
        sepoffs  = wrkint(psi+level)
        sepindex = sepbegin+sepoffs
        nodeend = nodebegin+sizes(sepindex)
        wrkint(psts+sepindex) = nstree+sizes(sepindex)
        nodebegin = nodeend
        wrkint(psi+level) = sepoffs+1
        if(mod(sepoffs+1,2).eq.0) then
          level = level-1
          nstree = wrkint(psts+sepindex)+wrkint(psts+sepindex-1)
        else
          level = dd
          nstree = 0
        end if

      end do

      level = 0
      j = 2*pp-2
      wrkint(psnb+j) = N-1-sizes(j)

      do i=2*pp-2,pp,-1

        if(sizes(i).eq.0) then
          kend = wrkint(psnb+i)
          lend = kend

          sizes(i) = 1
          wrkint(psnb+i) = wrkint(psnb+i)-1

          ifound = .false.

          m = i
          j = 2*pp-1-i 

          do while (.not.ifound .and. m.ge.pp) 

            k = 2*(pp-j)-1 
            l = 2*(pp-j)-2 

            if(wrkint(psts+l).gt.wrkint(psts+k)) then 
              m = l
              lend = lend - wrkint(psts+k)
            else
              m = k
            end if
            wrkint(psts+m) = wrkint(psts+m)-1

            if(sizes(m).gt.0) then
              ifound = .true.
            else
              j = 2*pp-1-m
            end if
          
          end do

          if(m.lt.pp .and. .not.ifound) then
            if(myid.eq.0) then
      print *,'Sorry! Cannot solve this problem on',pp,' processors!'
      print *,'Reason: Insufficient nodes available to'
      print *,'        pull into zero-size separators.'
            end if
            call mpi_barrier(comm,ierr)
            call mpi_abort(comm,0,ierr)
          end if

          ml = temparr2(N+lend)

          do col = lend+1,kend
            temparr2(N+col-1) = temparr2(N+col)
          end do

          temparr2(N+kend) = ml

          sizes(m) = sizes(m)-1

        end if

        j = 2*pp-1-i 
        k = 2*(pp-j)-1 
        l = 2*(pp-j)-2 
        wrkint(psnb+k) = wrkint(psnb+i)-sizes(k)
        wrkint(psnb+l) = wrkint(psnb+i)-wrkint(psts+k)-sizes(l)

      end do

      do i=0,pp-1
	if(sizes(i).le.0) then
	  if(myid.eq.0) then
      print *,'Sorry! Cannot solve this problem on',pp,' processors!'
      print *,'Reason: Zero size partition encountered. Probably the'
      print *,'        matrix is too small, or it has a block that is'
      print *,'        too dense to form required number of partitions.'
	  end if
          call mpi_barrier(comm,ierr)
          call mpi_abort(comm,0,ierr)
	end if
      end do

      do i=0,N-1
        j = temparr2(N+i)
        k = temparr2(j)
        gorder(k) = i
      end do

* adjust order and inds array to reflect new numbering.

      do i=0,mynnodes-1
        order(i) = gorder(i+rowdist(myid))
      end do

      do i=0,mynnodes-1
        k = aptrs(1,i)
        l = aptrs(2,i)
        do j=k,k+l-1
          tainds(j) = gorder(tainds(j))
        end do
        call ikeysortf(l,tainds(k),temparr2)
      end do

* preprocessing for filling up some bookkeeping arrays.

      rbits = ishft(dd,-1)
      cbits = dd-rbits
      pgrsize = ishft(1,rbits)
      pgcsize = ishft(1,cbits)

      do proc = 0,pp-1
        kr = 0
        i = ishft(proc,-1)
        do j = 0,rbits-1
          kr = ior(kr,ishft(iand(i,1),j))
          i = ishft(i,-2)
        end do
        kc = 0
        i = proc
        do j = 0,cbits-1
          kc = ior(kc,ishft(iand(i,1),j))
          i = ishft(i,-2)
        end do
        wrkint(ppr+proc) = kr
        wrkint(ppc+proc) = kc
        wrkint(ppg+kc*pgrsize+kr) = proc
      end do

      do level = 0,dd
        wrkint(psi+level) = 0
      end do

      nseps = ishft(pp,1)-1
      sepindex = 0
      level = dd
      nodebegin = 0

      do snodes=0,nseps-1

        sepbegin = nseps-(ishft(1,level+1)-1)
        sepoffs  = wrkint(psi+level)
        sepindex = sepbegin+sepoffs

        gbits = dd-level
        gsize = ishft(1,gbits)
        rbits = ishft(gbits,-1)
        proc  = sepoffs*gsize
        cbits = gbits-rbits
        bmaskr= ishft(1,rbits)-1
        bmaskc= ishft(1,cbits)-1

        if(level.eq.dd .and. sepoffs.eq.myid) mybeginleaf = nodebegin

        nodeend = nodebegin+sizes(sepindex)

        ranmasks(1,snodes) = nodebegin
        ranmasks(2,snodes) = nodeend-1
        ranmasks(3,snodes) = proc
        ranmasks(4,snodes) = bmaskr
        ranmasks(5,snodes) = bmaskc

        do col = nodebegin,nodeend-1
          parent(col) = col+1
        end do
        nodebegin = nodeend

        wrkint(psi+level) = sepoffs+1

        if(mod(sepoffs+1,2).eq.0) then
          parent(wrkint(pps+level)) = nodeend
          level = level-1
        else
          wrkint(pps+level) = nodeend-1
          level = dd
        end if

      end do
      parent(N-1) = -1

* estimate the size of arrays to be communicated 

      do proc=0,pp-1
        wrkint(psci+proc) = 0
        wrkint(pscs+proc) = 0
      end do

      do i=0,mynnodes-1
        col = order(i)
        j = 0
        do while (col.lt.ranmasks(1,j) .or. col.gt.ranmasks(2,j))
          j = j+1
        end do
        whichsnode(i) = j
        proc   = ranmasks(3,j)
        bmaskr = ranmasks(4,j)
        bmaskc = ranmasks(5,j)

        fptr_r = wrkint(ppr+proc)
        ptr_c  = wrkint(ppc+proc)+iand(ishft(col,-lgblk),bmaskc)

        do k=0,bmaskr
          proc = wrkint(ppg+ptr_c*pgrsize+fptr_r+k)
          wrkint(pscs+proc) = wrkint(pscs+proc)+2
        end do

        do j=aptrs(1,i),aptrs(1,i)+aptrs(2,i)-1
          row = tainds(j)
          ptr_r = fptr_r+iand(ishft(row,-lgblk),bmaskr)
          proc = wrkint(ppg+ptr_c*pgrsize+ptr_r)
          wrkint(psci+proc) = wrkint(psci+proc)+1
        end do
      end do 

      call mpi_alltoall(wrkint(psci),1,MPI_INTEGER,wrkint(prci),
     +                  1,MPI_INTEGER,comm,ierr)

      iwillreceive_inds = wrkint(prci)
      wrkint(prdi) = 0
      do proc=1,pp-1
        iwillreceive_inds = iwillreceive_inds+wrkint(prci+proc)
        wrkint(prdi+proc) = wrkint(prdi+proc-1)+wrkint(prci+proc-1)
      end do

      pasize = iwillreceive_inds

      end
