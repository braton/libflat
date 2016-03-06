#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct pointer {
	uint32_t low;
	uint32_t high;
	const char* s;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(pointer,
	AGGREGATE_FLATTEN_STRING(s);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	char s[] = "You've done it!";
	struct pointer hide = {};

	hide.s = s;
	hide.low = (uint64_t)s & 0xFFFFFFFFUL;
	hide.high = ((uint64_t)s & 0xFFFFFFFF00000000UL)>>32;

	printf("(%08x:%08x)\n",hide.high,hide.low);

	FOR_ROOT_POINTER(&hide,
		FLATTEN_STRUCT(pointer,&hide);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
