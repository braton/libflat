#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "libflat.h"

struct internal_type {
	int u:4;
	char c;
};

FUNCTION_DEFINE_FLATTEN_STRUCT(internal_type);

struct type {
	const char* name;
	unsigned long v;
	struct type* next;
	struct internal_type* internal[4];
};

FUNCTION_DEFINE_FLATTEN_STRUCT(type,
	AGGREGATE_FLATTEN_STRING(name);
	AGGREGATE_FLATTEN_STRUCT(type,next);
	FOREACH_POINTER(struct internal_type*,p,ATTR(internal),4,
		FLATTEN_STRUCT(internal_type,p);
	);
);

void flatten_debug_info();
void flatten_set_debug_flag(int flag);

int main(void) {

	FILE* out = fopen("/tmp/memory.img", "w");
	assert(out);
	flatten_init();

	char* st[] = {
	    "string0",
	    "string1",
	    "string2",
	    0
	};

	static struct internal_type u = { .u=1, .c='#' };

	struct internal_type w = { .u=2, .c='@' };

	struct type* alloc_type = calloc(1,sizeof(struct type));
		alloc_type->name = st[2]+2;
		alloc_type->next = alloc_type;
		alloc_type->internal[0] = &u;
		alloc_type->internal[1] = &w;

	struct type type_arr[2] = {
			{ .name="type1", .v=UINT_MAX, &type_arr[1], {&u,&u,0,0}  },
			{  st[1], .v=0, alloc_type, {0} }
	};

	struct type* root_ptr = &type_arr[0];

	printf("(automatic storage)\n\n");
	printf("st: 		[    %p    ][    %p    ][    %p    ][ 0 ]\n"
	"			^				  ^					^				  ^\n"
	"			%p	  %p	%p	  %p\n",st[0],st[1],st[2],&st[0],&st[1],&st[2],&st[3]);

	printf("w:			[ %zuB ]\n"
	"			^\n"
	"			%p\n",sizeof w,&w);

	printf("type_arr: 	[ ...  %zuB  ... ][ ...  %zuB  ... ]\n"
	"			^				 ^\n"
	"			%p	 %p\n\n",sizeof(struct type),sizeof(struct type),&type_arr[0],&type_arr[1]);
	printf("(allocated storage)\n\n");
	printf("alloc_type:	[ ...  %zuB  ... ]\n"
	"			^\n"
	"			%p\n\n",sizeof(struct type),alloc_type);
	printf("(static storage)\n\n");
	unsigned i;
	printf("st[0]:		[");for (i=0; i<strlen(st[0]); ++i) printf("\'%c\'|",st[0][i]); printf("0]\n");
	printf("			^\n"
	"			%p\n\n",st[0]);
	printf("st[1]:		[");for (i=0; i<strlen(st[1]); ++i) printf("\'%c\'|",st[1][i]); printf("0]\n");
	printf("			^\n"
	"			%p\n\n",st[1]);
	printf("st[2]:		[");for (i=0; i<strlen(st[2]); ++i) printf("\'%c\'|",st[2][i]); printf("0]\n");
	printf("			^\n"
	"			%p\n\n",st[2]);
	printf("u:			[ %zuB ]\n"
	"			^\n"
	"			%p\n\n",sizeof u,&u);
	printf("type_arr[0].name: 	[");for (i=0; i<strlen(type_arr[0].name); ++i) printf("\'%c\'|",type_arr[0].name[i]); printf("0]\n");
	printf("					^\n"
	"					%p\n\n",type_arr[0].name);


	flatten_set_debug_flag(1);

	FOR_ROOT_POINTER(root_ptr,
		FLATTEN_STRUCT(type,root_ptr);
	);

	FOR_ROOT_POINTER(st,
		FOREACH_POINTER(const char*,s,st,ptrarrmemlen((const void* const*)st),
			FLATTEN_STRING(s);
		);
	);

	flatten_debug_info();
	assert(flatten_write(out) == 0);

	flatten_fini();
	fclose(out);
	free(alloc_type);

	return 0;
}
