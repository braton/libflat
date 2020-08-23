#include <stdio.h>
#include <assert.h>
#include "libflat.h"

struct list_head;
struct intermediate;
struct task_struct;

FUNCTION_DECLARE_FLATTEN_STRUCT2_ITER(list_head);
FUNCTION_DECLARE_FLATTEN_STRUCT2_ITER(intermediate);
FUNCTION_DECLARE_FLATTEN_STRUCT2_ITER(task_struct);

struct list_head {
	struct list_head* prev;
	struct list_head* next;
};

FUNCTION_DEFINE_FLATTEN_STRUCT2_ITER(list_head,
	AGGREGATE_FLATTEN_STRUCT2_ITER(list_head,prev);
	AGGREGATE_FLATTEN_STRUCT2_ITER(list_head,next);
);
struct intermediate {
	struct list_head* plh;
};

FUNCTION_DEFINE_FLATTEN_STRUCT2_ITER(intermediate,
	AGGREGATE_FLATTEN_STRUCT2_ITER(list_head,plh);
);
struct task_struct {
	int pid;
	struct intermediate* im;
	struct list_head u;
	float w;
};

FUNCTION_DEFINE_FLATTEN_STRUCT2_ITER(task_struct,
	AGGREGATE_FLATTEN_STRUCT2_ITER(intermediate,im);
	AGGREGATE_FLATTEN_STRUCT2_ITER(list_head,u.prev);
	AGGREGATE_FLATTEN_STRUCT2_ITER(list_head,u.next);
);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	struct task_struct T;
	struct intermediate IM = {&T.u};
	T.pid = 123;
	T.im = &IM;
	T.u.prev = T.u.next = &T.u;
	T.w = 1.0;

	FOR_ROOT_POINTER(&T,
		UNDER_ITER_HARNESS2(
			FLATTEN_STRUCT2_ITER(task_struct,&T);
		);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	return 0;
}
