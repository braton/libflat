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
	STRUCT_ALIGN(64);
	AGGREGATE_FLATTEN_STRUCT2(B,pB0);
	AGGREGATE_FLATTEN_STRUCT2(B,pB1);
	AGGREGATE_FLATTEN_STRUCT2(B,pB2);
	AGGREGATE_FLATTEN_STRUCT2(B,pB3);
	AGGREGATE_FLATTEN_STRING2(p);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	printf("sizeof(struct A): %zu\n",sizeof(struct A));
	printf("sizeof(struct B): %zu\n",sizeof(struct B));

	struct B arrB[4] = {{1},{2},{3},{4}};
	struct A T[3] = {{},{0,&arrB[0],&arrB[1],&arrB[2],&arrB[3],"p in struct A"},{}};

	unsigned char* p = (unsigned char*)&T[1]-8;
	FOR_ROOT_POINTER(p,
		FLATTEN_TYPE_ARRAY(unsigned char,p,sizeof(struct A)+16);
	);

	FOR_ROOT_POINTER(&T[1],
		FLATTEN_STRUCT2(A,&T[1]);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
