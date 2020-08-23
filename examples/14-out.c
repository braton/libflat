#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

struct B {
	int i;
};

struct A {
	unsigned long ul;
	struct B* pB0;
	struct B* pB1;
	struct B* pB2;
	struct B* pB3;
	char* p;
};

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	unsigned char* p = ROOT_POINTER_NEXT(unsigned char*);
	(void)p;
	struct A* pA = ROOT_POINTER_NEXT(struct A*);

	printf("%d %d %d %d\n",pA->pB0->i,pA->pB1->i,pA->pB2->i,pA->pB3->i);
	printf("%lx\n",(uintptr_t)pA->p);
	printf("%s\n",pA->p);

	unflatten_fini();
	fclose(in);

	return 0;
}
