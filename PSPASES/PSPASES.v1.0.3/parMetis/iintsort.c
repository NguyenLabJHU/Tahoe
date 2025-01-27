#include <parmetis.h>


/* Byte-wise swap two items of size SIZE. */
#define QSSWAP(a, b, stmp) do { stmp = (a); (a) = (b); (b) = stmp; } while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 20

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
  int *lo;
  int *hi;
} stack_node;


/* The next 4 #defines implement a very fast in-line stack abstraction. */
#define STACK_SIZE	(8 * sizeof(unsigned long int))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)


void iintsort(int total_elems, int *pbase)
{
  int pivot, stmp;

  if (total_elems == 0)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;

  if (total_elems > MAX_THRESH) {
    int *lo = pbase;
    int *hi = &lo[total_elems - 1];
    stack_node stack[STACK_SIZE]; /* Largest size needed for 32-bit int!!! */
    stack_node *top = stack + 1;

    while (STACK_NOT_EMPTY) {
      int *left_ptr;
      int *right_ptr;

      /* Select median value from among LO, MID, and HI. Rearrange
	 LO and HI so the three values are sorted. This lowers the
	 probability of picking a pathological pivot value and
	 skips a comparison for both the LEFT_PTR and RIGHT_PTR. */

      int *mid = lo + ((hi - lo) >> 1);

      if (*mid < *lo) 
        QSSWAP(*mid, *lo, stmp);
      if (*hi < *mid)
        QSSWAP(*mid, *hi, stmp);
      else
        goto jump_over;
      if (*mid < *lo)
        QSSWAP(*mid, *lo, stmp);

jump_over:;
      pivot = *mid;
      left_ptr  = lo + 1;
      right_ptr = hi - 1;

      /* Here's the famous ``collapse the walls'' section of quicksort.
	 Gotta like those tight inner loops!  They are the main reason
	 that this algorithm runs much faster than others. */
      do {
	while (*left_ptr < pivot)
	  left_ptr++;

	while (pivot < *right_ptr)
	  right_ptr--;

	if (left_ptr < right_ptr) {
	  QSSWAP (*left_ptr, *right_ptr, stmp);
	  left_ptr++;
	  right_ptr--;
	}
	else if (left_ptr == right_ptr) {
	  left_ptr++;
	  right_ptr--;
	  break;
	}
      } while (left_ptr <= right_ptr);

      /* Set up pointers for next iteration.  First determine whether
         left and right partitions are below the threshold size.  If so,
         ignore one or both.  Otherwise, push the larger partition's
         bounds on the stack and continue sorting the smaller one. */

      if ((size_t) (right_ptr - lo) <= MAX_THRESH) {
        if ((size_t) (hi - left_ptr) <= MAX_THRESH)
	  /* Ignore both small partitions. */
          POP (lo, hi);
        else
	  /* Ignore small left partition. */
          lo = left_ptr;
      }
      else if ((size_t) (hi - left_ptr) <= MAX_THRESH)
	/* Ignore small right partition. */
        hi = right_ptr;
      else if ((right_ptr - lo) > (hi - left_ptr)) {
       /* Push larger left partition indices. */
       PUSH (lo, right_ptr);
       lo = left_ptr;
      }
      else {
	/* Push larger right partition indices. */
        PUSH (left_ptr, hi);
        hi = right_ptr;
      }
    }
  }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */

  {
    int *end_ptr = &pbase[total_elems - 1];
    int *tmp_ptr = pbase;
    int *thresh = (end_ptr < pbase + MAX_THRESH ? end_ptr : pbase + MAX_THRESH);
    register int *run_ptr;

    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */


    for (run_ptr = tmp_ptr + 1; run_ptr <= thresh; run_ptr++)
      if (*run_ptr < *tmp_ptr)
        tmp_ptr = run_ptr;

    if (tmp_ptr != pbase)
      QSSWAP(*tmp_ptr, *pbase, stmp);

    /* Insertion sort, running from left-hand-side up to right-hand-side.  */
    run_ptr = pbase + 1;
    while (++run_ptr <= end_ptr) {
      tmp_ptr = run_ptr - 1;
      while (*run_ptr < *tmp_ptr)
        tmp_ptr--;

      tmp_ptr++;
      if (tmp_ptr != run_ptr) {
        int elmnt = *run_ptr;
        int *mptr;

        for (mptr=run_ptr; mptr>tmp_ptr; mptr--)
          *mptr = *(mptr-1);
        *mptr = elmnt;
      }
    }
  }
}
