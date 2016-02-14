#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	int i;
	const char* s = "Fibonacci would not calculate this as fast";
	long fib[40] = { 0, 1 };
	for (i = 2; i < 40; ++i)
		fib[i] = fib[i - 1] + fib[i - 2];

	FOR_ROOT_POINTER(fib,
		FLATTEN_TYPE_ARRAY(long,fib,40);
	);

	FOR_ROOT_POINTER(s,
		FLATTEN_STRING(s);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
