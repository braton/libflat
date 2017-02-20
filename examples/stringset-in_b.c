#include "libflat.h"
#include "private.h"
#include "stringset.h"
#include <time.h>

static inline const struct string_node* ptr_remove_color(const struct string_node* ptr) {
	return (const struct string_node*)( (uintptr_t)ptr & ~3 );
}

static inline struct flatten_pointer* fptr_add_color(struct flatten_pointer* fptr, const struct string_node* ptr) {
	fptr->offset |= (size_t)((uintptr_t)ptr & 3);
	return fptr;
}

FUNCTION_DEFINE_FLATTEN_STRUCT(string_node,
	STRUCT_ALIGN(4);
	AGGREGATE_FLATTEN_STRUCT_MIXED_POINTER(string_node,node.__rb_parent_color,ptr_remove_color,fptr_add_color);
	AGGREGATE_FLATTEN_STRUCT(string_node,node.rb_right);
	AGGREGATE_FLATTEN_STRUCT(string_node,node.rb_left);
	AGGREGATE_FLATTEN_STRING(s);
);

FUNCTION_DEFINE_FLATTEN_STRUCT(rb_root,
	AGGREGATE_FLATTEN_STRUCT(string_node,rb_node);
);

int main(int argc, char** argv) {

	static const char chars[] = "ABCDEFGHIJ";
	srand (time(0));
	unsigned i,j;
	unsigned long snum;

	assert(argc > 1);
	assert(sscanf(argv[1], "%lu", &snum) == 1);

	for (j=0; j<snum; ++j) {
		char* s = calloc(1,sizeof chars);
		assert(s);
		for (i=0; i<sizeof chars - 1; ++i) {
			s[i] = chars[rand()%(sizeof chars - 1)];
		}
		stringset_insert(s);
		free(s);
	}

	printf("String set size: %zu\n",stringset_count(&stringset_root));

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	FOR_ROOT_POINTER(&stringset_root,
		FLATTEN_STRUCT(rb_root,&stringset_root);
	);

	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);

	stringset_destroy(&stringset_root);

	return 0;
}
