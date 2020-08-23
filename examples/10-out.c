#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "libflat.h"

struct list_head {
	struct list_head* prev;
	struct list_head* next;
};

struct intermediate {
	struct list_head* plh;
};

struct task_struct {
	int pid;
	struct intermediate* im;
	struct list_head u;
	float w;
};

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	struct task_struct *T = ROOT_POINTER_NEXT(struct task_struct*);

	printf("pid: %d\n",T->pid);
	printf("T: %lx\n",(uintptr_t)T);
	printf("T->im->plh: %lx\n",(uintptr_t)T->im->plh);
	printf("T->u.prev: %lx\n",(uintptr_t)T->u.prev);
	printf("T->u.next: %lx\n",(uintptr_t)T->u.next);
	printf("w: %f\n",T->w);

	unflatten_fini();
	fclose(in);

	return 0;
}
