#include <stdio.h>
#include <assert.h>
#include "libflat.h"

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	double magic_number = 3.14159265359;
	double* pointer_to_it = &magic_number;
	double** pointer_to_pointer_to_it = &pointer_to_it;
	double*** ehhh = &pointer_to_pointer_to_it;

	FOR_ROOT_POINTER(ehhh,
		FLATTEN_TYPE(double**,&pointer_to_pointer_to_it);
		FOR_POINTER(double**,p,&pointer_to_pointer_to_it,
			FLATTEN_TYPE(double*,p);
			FOR_POINTER(double*,q,p,
				FLATTEN_TYPE(double,q);
			);
		);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
