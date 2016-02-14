#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	const char* argv[] = { "Program name", "Argument 0", "Argument 1",
			"Argument 2", "Argument 3", 0 };

	FOR_ROOT_POINTER(argv,
		FOREACH_POINTER(const char*,arg,argv,
				ptrarrmemlen((const void * const*)argv),
			FLATTEN_TYPE(const char*,arg);
			FLATTEN_STRING(arg);
		);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
