#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include "libflat.h"

struct point {
	double x;
	double y;
	unsigned n;
	struct point** other;
};

struct figure {
	const char* name;
	unsigned n;
	struct point* points;
};

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	const struct figure* circle = ROOT_POINTER_NEXT(const struct figure*);

#if 1
	unsigned i, j;
	double length = 0, circumference = 0;
	unsigned edge_number = 0;
	for (i = 0; i < circle->n - 1; ++i) {
		for (j = i; j < circle->n - 1; ++j) {
			if (circle->points[i].other[j]) {

				double path_len = sqrt(  pow(circle->points[i].x-circle->points[i].other[j]->x,2) +
						pow(circle->points[i].y-circle->points[i].other[j]->y,2) );
				length += path_len;

				if (j == i)
					circumference += path_len;
				if ((i == 0) && (j == circle->n - 2))
					circumference += path_len;

				unsigned u;
				for (u = 0; u < circle->n - 1; ++u) {
					if (circle->points[i].other[j]->other[u] == &circle->points[i]) {
						circle->points[i].other[j]->other[u] = 0;
					}
				}
				edge_number++;
			}
		}
	}

	printf("Number of edges/diagonals: %u\n", edge_number);
	printf("Sum of lengths of edges/diagonals: %.17f\n", length);
	printf("Half of the circumference: %.17f\n", circumference / 2);
#endif

	unflatten_fini();
	fclose(in);

	return 0;
}
