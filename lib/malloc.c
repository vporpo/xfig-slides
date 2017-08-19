/* Provide a replacement function for malloc, if ./configure finds
 * that the original malloc() is not portable.
 * See info autoconf, section "Particular Function Checks".
 */

#undef malloc

/* #include <sys/types.h> */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h> /* info autoconf: On Darwin, stdio.h is a prerequisite. */

void *malloc();

/* Allocate an N-byte block of memory from the heap.
   If N is zero, allocate a 1-byte block.  */

void *
rpl_malloc(size_t n)
{
	if (n == 0)
		n = 1;
	return malloc(n);
}
