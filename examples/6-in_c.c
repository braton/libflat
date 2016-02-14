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
		FOR_POINTER(const char*,n,&square.name,
			FLATTEN_STRING(n);
		);
		FOR_POINTER(const char*,n,&square.e,
			FLATTEN_TYPE(struct edge*,n);
		);
		struct edge* e = square.e;
		do {
			FOR_POINTER(struct edge*,pe,&e->e,
				FLATTEN_TYPE_ARRAY(struct point,pe->p,2);
				FLATTEN_TYPE(struct edge*,pe);
			);
			e = e->e;
		} while(e!=square.e);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
