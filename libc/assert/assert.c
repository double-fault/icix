#include <assert.h>
#include <stdlib.h>

/* Shitty assert */
void assert(int abrt) {
	if (!abrt) {
		printf("Assert fail!\n");
		abort();
	}
}

