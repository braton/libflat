#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	int automatic = 0xDABBAD00;

	FOR_ROOT_POINTER(&automatic,
		FLATTEN_TYPE(int,&automatic);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
