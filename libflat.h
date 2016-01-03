#ifndef __LIBFLAT_H__
#define __LIBFLAT_H__

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <assert.h> 
#include "interval_tree.h"

/* Binary stream doubly linked list implementation */

struct blstream {
	struct blstream* next;
	struct blstream* prev;
	void* data;
	size_t size;
	unsigned long index;
};

struct blstream* binary_stream_append(const void* data, size_t size);
struct blstream* binary_stream_append_reserve(size_t size);
struct blstream* binary_stream_insert_front(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_front_reserve(size_t size, struct blstream* where);
struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_back_reserve(size_t size, struct blstream* where);
struct blstream* binary_stream_update(const void* data, size_t size, struct blstream* where);
void binary_stream_calculate_index();
void binary_stream_update_pointers();
void binary_stream_destroy();
void binary_stream_print();
size_t binary_stream_size();
size_t binary_stream_write(FILE* f);





struct interval_nodelist {
	struct interval_nodelist* next;
	struct interval_tree_node* node;
};

struct flatten_pointer {
	struct interval_tree_node* node;
	unsigned long offset;
};

struct flatten_header {
	unsigned long memory_size;
	unsigned long ptr_count;
	unsigned long root_ptr_offset;
};

void flatten_init();
void flatten_save(FILE* f);
void flatten_debug_info();
void flatten_fini();

struct FLCONTROL {
	struct rb_root imap_root;
	struct blstream *bhead
	struct blstream *btail;
	struct rb_root fixup_set_root; /* = RB_ROOT */
	struct rb_root imap_root = RB_ROOT;	/* = RB_ROOT */

	unsigned long root_addr;
	/* List for root pointers */
};

struct fixup_set_node {
	struct rb_node node;
  	/* Storage area and offset where the original address to be fixed is stored */
	struct interval_tree_node* inode;
	unsigned long offset;
	/* Storage area and offset there the original address points to */
	struct flatten_pointer* ptr;
};

struct fixup_set_node* create_fixup_set_node_element(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr);
struct fixup_set_node *fixup_set_search(unsigned long v);
int fixup_set_insert(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr);
void fixup_set_print();
unsigned long fixup_set_count();
void fixup_set_destroy();
size_t fixup_set_write(FILE* f);


static inline struct flatten_pointer* make_flatten_pointer(struct interval_tree_node* node, unsigned long offset) {
	struct flatten_pointer* v = malloc(sizeof(struct flatten_pointer));
	assert(v!=0);
	v->node = node;
	v->offset = offset;
	return v;
}

static inline size_t strmemlen(const char* s) {
	return strlen(s)+1;
}

static inline size_t ptrarrmemlen(const void* const* m) {
	size_t count=1;
	while(*m) {
		count++;
		m++;
	}
	return count;
}

/* #define DEBUG_FLATTEN_FUNCTION */
#ifdef DEBUG_FLATTEN_FUNCTION
#define DBGM1(name,a1)					do { printf(#name "(" #a1 ")\n"); } while(0)
#define DBGM2(name,a1,a2)				do { printf(#name "(" #a1 "," #a2 ")\n"); } while(0)
#define DBGM3(name,a1,a2,a3)			do { printf(#name "(" #a1 "," #a2 "," #a3 ")\n"); } while(0)
#define DBGM4(name,a1,a2,a3,a4)			do { printf(#name "(" #a1 "," #a2 "," #a3 "," #a4 ")\n"); } while(0)
#else
#define DBGM1(name,a1)
#define DBGM2(name,a1,a2)
#define DBGM3(name,a1,a2,a3)
#define DBGM4(name,a1,a2,a3,a4,...)
#endif

#define DBGL(n,...)		do { if (n<=CONFIG_DEBUG_LEVEL)	printf(__VA_ARGS__); } while(0)

#define ATTR(f)	((_ptr)->f)

#define FLATTEN_STRUCT(T,p)	do {	\
		DBGM2(FLATTEN_STRUCT,T,p);	\
		if (p) {   \
			fixup_set_insert(__fptr->node,__fptr->offset,flatten_struct_##T((p)));	\
		}	\
	} while(0)

#define AGGREGATE_FLATTEN_STRUCT(T,f)	do {	\
		DBGM2(AGGREGATE_FLATTEN_STRUCT,T,f);	\
    	if (ATTR(f)) {	\
    		fixup_set_insert(__node,offsetof(_container_type,f),flatten_struct_##T(ATTR(f)));	\
		}	\
	} while(0)


#define INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY(FLTYPE)	\
static inline struct flatten_pointer* flatten_struct_##FLTYPE##_array(struct FLTYPE* _p, int n) {	\
	int _i;	\
	void* _fp_first=0;	\
	for (_i=0; _i<n; ++_i) {	\
		void* _fp = (void*)flatten_struct_##FLTYPE(_p+_i);	\
		if (!_fp_first) _fp_first=_fp;	\
		else free(_fp);	\
	}	\
    return _fp_first;	\
}

#define FLATTEN_STRUCT_ARRAY(T,p,n)	do {	\
		DBGM3(FLATTEN_STRUCT_ARRAY,T,p,n);	\
		if (p) {   \
			fixup_set_insert(__fptr->node,__fptr->offset,flatten_struct_##T##_array((p),(n)));	\
		}	\
	} while(0)

#define AGGREGATE_FLATTEN_STRUCT_ARRAY(T,f,n)	do {	\
		DBGM3(AGGREGATE_FLATTEN_STRUCT_ARRAY,T,f,n);	\
    	if (ATTR(f)) {	\
    		int _i;	\
    		void* _fp_first=0;	\
    		for (_i=0; _i<(n); ++_i) {	\
    			void* _fp = (void*)flatten_struct_##T(ATTR(f)+_i);	\
    			if (!_fp_first) _fp_first=_fp;	\
    			else free(_fp);	\
    		}	\
		    fixup_set_insert(__node,offsetof(_container_type,f),_fp_first);	\
		}	\
	} while(0)

#define FLATTEN_STRING(p)	do {	\
		DBGM1(FLATTEN_STRING,p);	\
		if (p) {   \
			fixup_set_insert(__fptr->node,__fptr->offset,flatten_plain_type((p),strmemlen(p)));	\
		}	\
	} while(0)

#define AGGREGATE_FLATTEN_STRING(f)   do {  \
		DBGM1(AGGREGATE_FLATTEN_STRING,f);	\
        if (ATTR(f)) {   \
        	fixup_set_insert(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),strmemlen(ATTR(f))));	\
        }   \
    } while(0)

struct flatten_pointer* get_pointer_node(const void* _ptr);
struct flatten_pointer* flatten_plain_type(const void* _ptr, size_t _sz);

#define FLATTEN_TYPE(T,p)	do {	\
		DBGM2(FLATTEN_TYPE,T,p);	\
		if (p) {   \
			fixup_set_insert(__fptr->node,__fptr->offset,flatten_plain_type((p),sizeof(T)));	\
		}	\
	} while(0)

#define AGGREGATE_FLATTEN_TYPE(T,f)   do {  \
		DBGM2(AGGREGATE_FLATTEN_TYPE,T,f);	\
        if (ATTR(f)) {   \
            fixup_set_insert(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),sizeof(T)));	\
        }   \
    } while(0)

#define FLATTEN_TYPE_ARRAY(T,p,n)	do {	\
		DBGM3(FLATTEN_TYPE_ARRAY,T,p,n);	\
		if (p) {   \
			fixup_set_insert(__fptr->node,__fptr->offset,flatten_plain_type((p),(n)*sizeof(T)));	\
		}	\
	} while(0)

#define AGGREGATE_FLATTEN_TYPE_ARRAY(T,f,n)   do {  \
		DBGM3(AGGREGATE_FLATTEN_TYPE_ARRAY,T,f,n);	\
        if (ATTR(f)) {   \
            fixup_set_insert(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),(n)*sizeof(T)));	\
        }   \
    } while(0)

#define FOREACH_POINTER(PTRTYPE,v,p,s,...)	do {	\
		DBGM4(FOREACH_POINTER,PTRTYPE,v,p,s);	\
		if (p) {	\
			PTRTYPE const * _m = (PTRTYPE const *)(p);	\
			size_t _i, _sz = (s);	\
			for (_i=0; _i<_sz; ++_i) {	\
				struct flatten_pointer* __fptr = get_pointer_node(_m+_i);	\
				PTRTYPE v = *(_m+_i);	\
				__VA_ARGS__;	\
				free(__fptr);	\
			}	\
		}	\
	} while(0)

#define FOR_POINTER(PTRTYPE,v,p,...)	do {	\
		DBGM3(FOR_POINTER,PTRTYPE,v,p);	\
		if (p) {	\
			PTRTYPE const * _m = (PTRTYPE const *)(p);	\
			struct flatten_pointer* __fptr = get_pointer_node(_m);	\
			PTRTYPE v = *(_m);	\
			__VA_ARGS__;	\
			free(__fptr);	\
		}	\
	} while(0)

#define INSTALL_ROOT_POINTER(p)

#define FOR_ROOT_POINTER(p,...)	do {	\
		DBGM1(FOR_ROOT_POINTER,p);	\
		if (p) {	\
			struct flatten_pointer* __fptr = make_flatten_pointer(0,(unsigned long)(p));	\
			__VA_ARGS__;	\
			free(__fptr);	\
			INSTALL_ROOT_POINTER(p);	\
		}	\
	} while(0)


#define PTRNODE(PTRV)	(interval_tree_iter_first(&imap_root, (uint64_t)(PTRV), (uint64_t)(PTRV)))
#define ROOT_PTR_OFFSET(PTRV)	((assert(PTRNODE(PTRV)!=0),1)?(PTRNODE(PTRV)->storage->index + (((unsigned long)PTRV)-PTRNODE(PTRV)->start)):(0))
#define ROOT_PTR(PTRTYPE,h,m)	((PTRTYPE)((m+h.ptr_count*sizeof(unsigned long)+h.root_ptr_offset)))

#define FUNCTION_DEFINE_FLATTEN_STRUCT(FLTYPE,...)	\
/* */		\
			\
struct flatten_pointer* flatten_struct_##FLTYPE(const struct FLTYPE* _ptr) {	\
			\
	typedef struct FLTYPE _container_type;	\
			\
	struct interval_tree_node *__node = interval_tree_iter_first(&imap_root, (uint64_t)_ptr, (uint64_t)_ptr+sizeof(struct FLTYPE)-1);	\
	if (__node) {	\
		assert(__node->start==(uint64_t)_ptr);	\
		assert(__node->last==(uint64_t)_ptr+sizeof(struct FLTYPE)-1);	\
		return make_flatten_pointer(__node,0);	\
	}	\
	else {	\
		__node = calloc(1,sizeof(struct interval_tree_node));	\
		assert(__node!=0);	\
		__node->start = (uint64_t)_ptr;	\
		__node->last = (uint64_t)_ptr + sizeof(struct FLTYPE)-1;	\
		struct blstream* storage;	\
		struct rb_node* rb = interval_tree_insert(__node, &imap_root);	\
		struct rb_node* prev = rb_prev(rb);	\
		if (prev) {	\
			storage = binary_stream_insert_back(_ptr,sizeof(struct FLTYPE),((struct interval_tree_node*)prev)->storage);	\
		}	\
		else {	\
			struct rb_node* next = rb_next(rb);	\
			if (next) {	\
				storage = binary_stream_insert_front(_ptr,sizeof(struct FLTYPE),((struct interval_tree_node*)next)->storage);	\
			}	\
			else {	\
				storage = binary_stream_append(_ptr,sizeof(struct FLTYPE));	\
			}	\
		}	\
		__node->storage = storage;	\
	}	\
		\
	__VA_ARGS__	\
    return make_flatten_pointer(__node,0);	\
}

#define FUNCTION_DECLARE_FLATTEN_STRUCT(FLTYPE)	\
	extern struct flatten_pointer* flatten_struct_##FLTYPE(const struct FLTYPE*);	\
	INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY(FLTYPE)

#define TIME_MARK_START(start_marker)		\
		struct timeval  tv_mark_##start_marker;	\
		gettimeofday(&tv_mark_##start_marker, 0)
#define TIME_CHECK_ON(start_marker,end_marker)	do {	\
		struct timeval  tv_mark_##start_marker##_##end_marker;	\
		gettimeofday(&tv_mark_##start_marker##_##end_marker, 0);	\
		printf("@Elapsed ("#start_marker" -> "#end_marker"): (%f)[s]\n",	\
		(double) (tv_mark_##start_marker##_##end_marker.tv_usec - tv_mark_##start_marker.tv_usec) / 1000000 +	\
		         (double) (tv_mark_##start_marker##_##end_marker.tv_sec - tv_mark_##start_marker.tv_sec) );	\
	} while(0)

void fix_unflatten_memory(struct flatten_header* hdr, void* memory);

#endif /* __LIBFLAT_H__ */
