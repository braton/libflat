#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	long* fib = ROOT_POINTER_SEQ(long*, 0);
	const char* s = ROOT_POINTER_SEQ(const char*, 1);

	int i;
	for (i = 0; i < 40; ++i)
		printf("%ld ", fib[i]);
	printf("\n");
	printf("%s\n", s);

	unflatten_fini();
	fclose(in);

	return 0;
}
