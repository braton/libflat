#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	const char** argv = ROOT_POINTER_NEXT(const char**);
	while (*argv) {
		printf("%s\n", *argv);
		argv++;
	}

	unflatten_fini();
	fclose(in);

	return 0;
}
