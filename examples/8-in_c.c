#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct internal_type {
	int u:4;
	char c;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(internal_type);

struct type {
	const char* name;
	unsigned long v;
	struct type* next;
	struct internal_type* internal[4];
};

FUNCTION_DEFINE_FLATTEN_STRUCT(type,
	AGGREGATE_FLATTEN_STRING(name);
	AGGREGATE_FLATTEN_STRUCT(type,next);
	AGGREGATE_FLATTEN_TYPE_ARRAY(struct internal_type*,internal,4);
	FOREACH_POINTER(struct internal_type*,p,ATTR(internal),4,
		FLATTEN_STRUCT(internal_type,p);
	);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	static struct internal_type u = { .u=1, .c='#' };

	struct type type_arr = { .name="type1", .v=666, &type_arr, {&u,&u,0,0}  };

	FOR_ROOT_POINTER(&type_arr,
		FLATTEN_STRUCT(type,&type_arr);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
