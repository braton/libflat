#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

struct pointer {
	uint32_t low;
	uint32_t high;
};

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	struct pointer* p = ROOT_POINTER_NEXT(struct pointer*);
	(void)p;

	unflatten_fini();
	fclose(in);

	return 0;
}
