#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	double*** ehhh = ROOT_POINTER_SEQ(double***, 0);
	printf("The magic answer to the ultimate question of life?: %f\n", ***ehhh);

	unflatten_fini();
	fclose(in);

	return 0;
}
