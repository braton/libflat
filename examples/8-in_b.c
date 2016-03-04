#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	const char* st[2] = {
			"Some value here",
			"Other value here",
	};

	FOR_ROOT_POINTER((unsigned char*)&st[0],
		FLATTEN_TYPE(unsigned char,(unsigned char*)&st[0]);
	);

	FOR_ROOT_POINTER(st,
		FOREACH_POINTER(const char*,s,st,ptrarrmemlen((const void* const*)st),
			FLATTEN_STRING(s);
		);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
