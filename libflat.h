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

extern struct rb_root imap_root;

struct interval_nodelist {
	struct interval_nodelist* next;
	struct interval_tree_node* node;
};

struct blstream {
	struct blstream* next;
	struct blstream* prev;
	void* data;
	size_t size;
	unsigned long index;
} bs;

extern struct blstream *bhead,*bptr;
struct blstream* binary_stream_append(const void* data, size_t size);
struct blstream* binary_stream_append_reserve(size_t size);
struct blstream* binary_stream_update(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_front(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where);
void binary_stream_calculate_index();
void binary_stream_update_pointers();
void binary_stream_destroy();
void binary_stream_print();

struct fixup_node {
	struct fixup_node* next;
	/* Storage area and offset where the original address to be fixed is stored */
	struct interval_tree_node* node;
	unsigned long offset;
	/* Storage area and offset there the original address points to */
	struct flatten_pointer* ptr;
};

extern struct fixup_node *fhead,*ftail;
struct fixup_node* create_fixup_node_element(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr);
void fixup_list_append(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr);
void fixup_list_destroy();
void fixup_list_print();

struct flatten_pointer {
	struct interval_tree_node* node;
	unsigned long offset;
};

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

#define ATTR(f)	((_ptr)->f)

#define FLATTEN_STRUCT(T,p)	(p)?((void*)flatten_struct_##T((p))):(0)

#define AGGREGATE_FLATTEN_STRUCT(T,f)	do {	\
    	if (ATTR(f)) {	\
    		fixup_list_append(__node,offsetof(_container_type,f),flatten_struct_##T(ATTR(f)));	\
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

#define FLATTEN_STRUCT_ARRAY(T,p,n)	(p)?((void*)flatten_struct_##T##_array((p),(n))):(0)

#define AGGREGATE_FLATTEN_STRUCT_ARRAY(T,f,n)	do {	\
    	if (ATTR(f)) {	\
    		int _i;	\
    		void* _fp_first=0;	\
    		for (_i=0; _i<(n); ++_i) {	\
    			void* _fp = (void*)flatten_struct_##T(ATTR(f)+_i);	\
    			if (!_fp_first) _fp_first=_fp;	\
    			else free(_fp);	\
    		}	\
		    fixup_list_append(__node,offsetof(_container_type,f),_fp_first);	\
		}	\
	} while(0)

#define FLATTEN_STRING(p)	(p)?((void*)flatten_plain_type((p),strmemlen(p))):(0)

#define AGGREGATE_FLATTEN_STRING(f)   do {  \
        if (ATTR(f)) {   \
        	fixup_list_append(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),strmemlen(ATTR(f))));	\
        }   \
    } while(0)

struct flatten_pointer* flatten_plain_type(const void* _ptr, size_t _sz);

#define FLATTEN_TYPE(T,p)	(p)?((void*)flatten_plain_type((p),sizeof(T))):(0)

#define AGGREGATE_FLATTEN_TYPE(T,f)   do {  \
        if (ATTR(f)) {   \
            fixup_list_append(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),sizeof(T)));	\
        }   \
    } while(0)

#define FLATTEN_TYPE_ARRAY(T,p,n)	(p)?((void*)flatten_plain_type((p),(n)*sizeof(T))):(0)

#define AGGREGATE_FLATTEN_TYPE_ARRAY(T,f,n)   do {  \
        if (ATTR(f)) {   \
            fixup_list_append(__node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),(n)*sizeof(T)));	\
        }   \
    } while(0)

#define FOREACH_POINTER(PTRTYPE,v,p,s,...)	do {	\
		PTRTYPE const * _m = (PTRTYPE const *)(p);	\
		size_t _i, _sz = (s);	\
		for (_i=0; _i<_sz; ++_i) {	\
			PTRTYPE v = (_m+_i)?(*(_m+_i)):(0);	\
			
			__VA_ARGS__	\
		}	\
	} while(0)

#define FOR_POINTER(PTRTYPE,v,p,...)	do {	\
		PTRTYPE const * _m = (PTRTYPE const *)(p);	\
		PTRTYPE v = (_m)?(*(_m)):(0);	\
		__VA_ARGS__;	\
	} while(0)

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
		__node->storage = binary_stream_append_reserve(sizeof(struct FLTYPE));	\
		interval_tree_insert(__node, &imap_root);	\
	}	\
		\
	__VA_ARGS__	\
	binary_stream_update(_ptr,sizeof(struct FLTYPE),__node->storage);	\
    return make_flatten_pointer(__node,0);	\
}

#define FUNCTION_DECLARE_FLATTEN_STRUCT(FLTYPE)	\
	extern struct flatten_pointer* flatten_struct_##FLTYPE(const struct FLTYPE*);	\
	INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY(FLTYPE)

#endif /* __LIBFLAT_H__ */
