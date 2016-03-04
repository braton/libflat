#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	const char* s = "Very valuable data";

	FOR_ROOT_POINTER(&s,
		FOR_POINTER(const char*,v,&s,
			FLATTEN_STRING(v);
		);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
