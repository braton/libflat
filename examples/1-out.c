#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	int* now_allocated = ROOT_POINTER_NEXT(int*);
	printf("%08X\n", *now_allocated);

	unflatten_fini();
	fclose(in);

	return 0;
}
