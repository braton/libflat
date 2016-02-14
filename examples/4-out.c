#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	const char* start = ROOT_POINTER_SEQ(const char*, 0);
	const char* middle = ROOT_POINTER_SEQ(const char*, 1);

	printf("&start: %p -> %p\n", &start, start);
	printf("&middle: %p -> %p\n", &middle, middle);
	const void* ptr1 = start + strlen(start) + 1;
	printf("*(%p) => %p\n", ptr1, *(void**) ptr1);
	const void* ptr2 = start + strlen(start) + 1 + sizeof(const char*);
	printf("*(%p) => %p\n", ptr2, *(void**) ptr2);
	printf("%s\n", start);
	printf("Remember: %s\n", middle);

	unflatten_fini();
	fclose(in);

	return 0;
}
