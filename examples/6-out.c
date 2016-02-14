#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
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

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	const struct figure* square = ROOT_POINTER_NEXT(const struct figure*);

	struct edge* e = square->e;
	double circumference = 0;
	do {
		circumference += sqrt(
				pow(e->p[0].x - e->p[1].x, 2) + pow(e->p[0].y - e->p[1].y, 2));
		e = e->e;
	} while (e != square->e);

	printf("%s circumference: %f\n", square->name, circumference);

	unflatten_fini();
	fclose(in);

	return 0;
}
