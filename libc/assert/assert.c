#include <assert.h>
#include <stdlib.h>

/* Shitty assert */
void assert(int abrt) {
	if (!abrt) abort();
}

