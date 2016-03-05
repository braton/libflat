#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct type {
	const char* name;
	unsigned long v;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(type,
	AGGREGATE_FLATTEN_STRING(name);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	struct type s = { "Some value here", 0 };

	FOR_ROOT_POINTER((unsigned short*)&s.v,
		FLATTEN_TYPE(unsigned short,(unsigned short*)&s.v);
	);

	FOR_ROOT_POINTER(&s,
		FLATTEN_STRUCT(type,&s);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
