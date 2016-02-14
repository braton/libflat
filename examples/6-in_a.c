#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct point {
	double x;
	double y;
};

struct edge {
	struct point p[2];
	struct edge* e;
};

struct figure {
	const char* name;
	struct edge* e;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(figure);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	struct point sqv[4] = {{1.0,1.0},{1.0,-1.0},{-1.0,-1.0},{-1.0,1.0}};
	struct edge sqe[4] = {
			{ {sqv[0],sqv[1]}, &sqe[1] },
			{ {sqv[1],sqv[2]}, &sqe[2] },
			{ {sqv[2],sqv[3]}, &sqe[3] },
			{ {sqv[3],sqv[0]}, &sqe[0] }
	};

	struct figure square = { "square", sqe };

	printf("sizeof(struct figure): %zu\n", sizeof(struct figure));

	FOR_ROOT_POINTER(&square,
		FLATTEN_STRUCT(figure,&square);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
