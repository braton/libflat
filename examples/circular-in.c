#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "libflat.h"

struct point {
	double x;
	double y;
	unsigned n;
	struct point** other;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(point,
	AGGREGATE_FLATTEN_TYPE_ARRAY(struct point*,other,ATTR(n));
	FOREACH_POINTER(struct point*,p,ATTR(other),ATTR(n),
			FLATTEN_STRUCT(point,p);
	);
);

struct figure {
	const char* name;
	unsigned n;
	struct point* points;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(figure,
	AGGREGATE_FLATTEN_STRING(name);
	AGGREGATE_FLATTEN_STRUCT_ARRAY(point,points,ATTR(n));
);

int main(int argc, char** argv) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	assert(argc > 1);
	struct figure circle = { "circle" };
	assert(sscanf(argv[1], "%u", &circle.n) == 1);
	assert(circle.n > 1);
	circle.points = calloc(circle.n, sizeof(struct point));
	assert(circle.points);
	double radius = 1.0, angle = 2 * M_PI / circle.n;

#define MAKE_POINT(p,i,N)	\
	p.x = cos(angle*(radius*(i)));	\
	p.y = sin(angle*(radius*(i)));	\
	p.n = (N);	\
	p.other = calloc((N),sizeof*p.other);	\
	assert(p.other);

	unsigned i, j;
	for (i = 0; i < circle.n; ++i) {
		MAKE_POINT(circle.points[i], i, circle.n - 1);
	}
	for (i = 0; i < circle.n; ++i) {
		unsigned u = 0;
		for (j = 0; j < circle.n; ++j) {
			if (i == j)
				continue;
			circle.points[i].other[u++] = &circle.points[j];
		}
	}

	FOR_ROOT_POINTER(&circle,
		FLATTEN_STRUCT(figure,&circle);
	);

	for (i = 0; i < circle.n; ++i) {
		free(circle.points[i].other);
	}
	free(circle.points);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
