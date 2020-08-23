#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct B {
	int i;
};

FUNCTION_DEFINE_FLATTEN_STRUCT2(B,
);

struct A {
	unsigned long ul;
	struct B* pB0;
	struct B* pB1;
	struct B* pB2;
	struct B* pB3;
	char* p;
};

FUNCTION_DEFINE_FLATTEN_STRUCT2(A,
	AGGREGATE_FLATTEN_STRUCT2(B,pB0);
	AGGREGATE_FLATTEN_STRUCT2(B,pB1);
	AGGREGATE_FLATTEN_STRUCT2(B,pB2);
	AGGREGATE_FLATTEN_STRUCT2(B,pB3);
	AGGREGATE_FLATTEN_STRING(p);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	printf("sizeof(struct A): %zu\n",sizeof(struct A));
	printf("sizeof(struct B): %zu\n",sizeof(struct B));

	struct B arrB[4] = {};
	struct A T = {0,&arrB[0],&arrB[1],&arrB[2],&arrB[3],"p in struct A"};

	FOR_ROOT_POINTER(&T,
		FLATTEN_STRUCT2(A,&T);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
