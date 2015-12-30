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
} bs;

extern struct blstream *bhead,*bptr;
struct blstream* binary_stream_append(const void* data, size_t size);
struct blstream* binary_stream_append_reserve(size_t size);
struct blstream* binary_stream_update(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_front(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where);
void binary_stream_destroy();
void binary_stream_print();

struct field_offset {
	struct field_offset* next;
	unsigned long offset;
};

struct field_offset* create_field_offset_element(unsigned long offset);
void field_offset_destroy(struct field_offset* head);
void field_offset_print(struct field_offset* head);

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

struct address_node {
	struct rb_node rb;
	unsigned long address;
};

typedef size_t (*calc_fun_char)(const char*);

static inline size_t strmemlen(const char* s) {
	return strlen(s)+1;
}

#define APPEND_FIELD_OFFSET(f) do {	\
		struct field_offset* _v = create_field_offset_element(offsetof(_container_type,f));	\
		if (!node->poffs_head) {	\
	        node->poffs_head = _v;	\
	        node->poffs = _v;	\
	    }	\
	    else {	\
	        node->poffs->next = _v;	\
	        node->poffs = node->poffs->next;	\
	    }	\
	} while(0)

#define AGGREGATE_FLATTEN_STRUCT(T,f)	do {	\
    	if ((_ptr)->f) {	\
		    (_ptr)->f = (void*)flatten_struct_##T((_ptr)->f);	\
		    APPEND_FIELD_OFFSET(f);	\
		}	\
	} while(0)

#define FLATTEN_STRUCT(T,p)	(p)?((void*)flatten_struct_##T((p))):(0)

#define AGGREGATE_FLATTEN_STRUCT_ARRAY_SIZE(T,f,n)	do {	\
    	if ((_ptr)->f) {	\
    		int _i;	\
    		void* _fp_first=0;	\
    		for (_i=0; _i<(n); ++_i) {	\
    			void* _fp = (void*)flatten_struct_##T((_ptr)->f+_i);	\
    			if (!_fp_first) _fp_first=_fp;	\
    		}	\
		    (_ptr)->f = _fp_first;	\
		    APPEND_FIELD_OFFSET(f);	\
		}	\
	} while(0)

#define INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY_SIZE(FLTYPE)	\
static inline struct flatten_pointer* flatten_struct_##FLTYPE##_array_size(struct FLTYPE* _p, int n) {	\
	int _i;	\
	void* _fp_first=0;	\
	for (_i=0; _i<n; ++_i) {	\
		void* _fp = (void*)flatten_struct_##FLTYPE(_p+_i);	\
		if (!_fp_first) _fp_first=_fp;	\
	}	\
    return _fp_first;	\
}

#define FLATTEN_STRUCT_ARRAY_SIZE(T,p,n)	(p)?((void*)flatten_struct_##T##_array_size((p),(n))):(0)

#define FLATTEN_STRING(p)	(p)?((void*)flatten_plain_char_calc((p),strmemlen)):(0)

#define AGGREGATE_FLATTEN_STRING(f)   do {  \
        if ((_ptr)->f) {   \
            (_ptr)->f = (void*)flatten_plain_char_calc((_ptr)->f,strmemlen); \
            APPEND_FIELD_OFFSET(f);	\
        }   \
    } while(0)

#define FLATTEN_TYPE_ARRAY_SIZE(T,p,n)	(p)?((void*)flatten_plain_##T##_size((p),(n)*sizeof(T))):(0)

#define AGGREGATE_FLATTEN_TYPE_ARRAY_SIZE(T,f,n)   do {  \
        if ((_ptr)->f) {   \
            (_ptr)->f = (void*)flatten_plain_##T##_size((_ptr)->f,(n)*sizeof(T)); \
            APPEND_FIELD_OFFSET(f);	\
        }   \
    } while(0)

#define FUNCTION_DEFINE_FLATTEN_STRUCT(FLTYPE,...)	\
/* */		\
			\
struct flatten_pointer* flatten_struct_##FLTYPE(struct FLTYPE* _ptr) {	\
			\
	typedef struct FLTYPE _container_type;	\
			\
	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)_ptr, (uint64_t)_ptr+sizeof(struct FLTYPE)-1);	\
	if (node) {	\
		assert(node->start==(uint64_t)_ptr);	\
		assert(node->last==(uint64_t)_ptr+sizeof(struct FLTYPE)-1);	\
		return make_flatten_pointer(node,0);	\
	}	\
	else {	\
		node = calloc(1,sizeof(struct interval_tree_node));	\
		assert(node!=0);	\
		node->start = (uint64_t)_ptr;	\
		node->last = (uint64_t)_ptr + sizeof(struct FLTYPE)-1;	\
		node->storage = binary_stream_append_reserve(sizeof(struct FLTYPE));	\
		interval_tree_insert(node, &imap_root);	\
	}	\
		\
	__VA_ARGS__	\
	binary_stream_update(_ptr,sizeof(struct FLTYPE),node->storage);	\
    return make_flatten_pointer(node,0);	\
}

#define FUNCTION_DECLARE_FLATTEN_STRUCT(FLTYPE)	\
	extern struct flatten_pointer* flatten_struct_##FLTYPE(struct FLTYPE*)

#define FUNCTION_DEFINE_FLATTEN_PLAIN_TYPE(FLTYPE)	\
/* */		\
			\
struct flatten_pointer* flatten_plain_##FLTYPE##_size(const FLTYPE* _ptr, size_t _sz) {	\
			\
	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)_ptr, (uint64_t)_ptr+_sz-1);	\
	struct flatten_pointer* r = 0;	\
	if (node) {	\
		uint64_t p = (uint64_t)_ptr;	\
		struct interval_tree_node *prev;	\
		while(node) {	\
			if (node->start>p) {	\
				assert(node->storage!=0);	\
				struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));	\
				assert(nn!=0);	\
				nn->start = p;	\
				nn->last = node->start-1;	\
				nn->storage = binary_stream_insert_front((void*)p,node->start-p,node->storage);	\
				interval_tree_insert(nn, &imap_root);	\
				if (r==0) {	\
					r = make_flatten_pointer(nn,0);	\
				}	\
			}	\
			else {	\
				if (r==0) {	\
					r = make_flatten_pointer(node,p-node->start);	\
				}	\
			}	\
			p = node->last+1;	\
			prev = node;	\
			node = interval_tree_iter_next(node, (uint64_t)_ptr, (uint64_t)_ptr+_sz-1);	\
		}	\
		if ((uint64_t)_ptr+_sz>p) {	\
			assert(prev->storage!=0);	\
			struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));	\
			assert(nn!=0);	\
			nn->start = p;	\
			nn->last = (uint64_t)_ptr+_sz-1;	\
			nn->storage = binary_stream_insert_back((void*)p,(uint64_t)_ptr+_sz-p,prev->storage);	\
			interval_tree_insert(nn, &imap_root);	\
		}	\
		return r;	\
	}	\
	else {	\
		node = calloc(1,sizeof(struct interval_tree_node));	\
		assert(node!=0);	\
		node->start = (uint64_t)_ptr;	\
		node->last = (uint64_t)_ptr + _sz-1;	\
		node->storage = binary_stream_append(_ptr,_sz);	\
		interval_tree_insert(node, &imap_root);	\
		return make_flatten_pointer(node,0);	\
	}	\
}

#define FUNCTION_DECLARE_FLATTEN_PLAIN_TYPE(FLTYPE)	\
	struct flatten_pointer* flatten_plain_##FLTYPE##_size(const FLTYPE* _ptr, size_t _sz);

FUNCTION_DECLARE_FLATTEN_PLAIN_TYPE(char);
FUNCTION_DECLARE_FLATTEN_PLAIN_TYPE(int);

static inline struct flatten_pointer* flatten_plain_char_calc(const char* v, calc_fun_char f) {
	
	return flatten_plain_char_size(v,f(v));
}

#endif /* __LIBFLAT_H__ */
