#ifndef __LIBFLAT_H__
#define __LIBFLAT_H__

/*
  Library for fast serialization of C/C++ structures
  (C) 2015  Bartosz Zator <braton@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  The following macros are available to describe how to flatten types:

  	FLATTEN_STRUCT(type,pointer)
	AGGREGATE_FLATTEN_STRUCT(type,field)
	FLATTEN_STRUCT_ARRAY(type,pointer,size)
	AGGREGATE_FLATTEN_STRUCT_ARRAY(type,field,size)
	FLATTEN_STRING(pointer)
	AGGREGATE_FLATTEN_STRING(field)
	FLATTEN_TYPE(type,pointer)
	AGGREGATE_FLATTEN_TYPE(type,field)
	FLATTEN_TYPE_ARRAY(type,pointer,size)
	AGGREGATE_FLATTEN_TYPE_ARRAY(type,field,size)
	FOR_POINTER(pointer_type,value_to_use,pointer,code)
	FOREACH_POINTER(pointer_type,value_to_use,pointer,size,code)
	ATTR(field)
	FOR_ROOT_POINTER(root_pointer,code)
	ROOT_POINTER_NEXT(pointer_type)
	ROOT_POINTER_SEQ(pointer_type,pointer_number)
	INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY(type)
	FUNCTION_DEFINE_FLATTEN_STRUCT(type,code)
	FUNCTION_DECLARE_FLATTEN_STRUCT(type)

  See README and docs for more details.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

/* Main interface functions */

void flatten_init();
int flatten_write(FILE* f);
void flatten_fini();
void unflatten_init();
int unflatten_read(FILE* f);
void unflatten_fini();

/* Implementation */

#ifdef __linux__
#define _ALIGNAS(n)	__attribute__((aligned(n)))
#define RB_NODE_ALIGN	(sizeof(long))
#else
#ifdef _WIN32
#define _ALIGNAS(n)	__declspec(align(n))
#ifdef _M_IX86
#define RB_NODE_ALIGN	4
#elif defined _M_X64
#define RB_NODE_ALIGN	8
#endif
#endif	/* _WIN32 */
#endif /* __linux__ */

struct _ALIGNAS(RB_NODE_ALIGN) rb_node {
	uintptr_t  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};
/* The alignment might seem pointless, but allegedly CRIS needs it */

struct interval_tree_node {
	struct rb_node rb;
	uintptr_t start;	/* Start of interval */
	uintptr_t last;	/* Last location _in_ interval */
	uintptr_t __subtree_last;
	struct blstream* storage;
};

struct rb_root {
	struct rb_node *rb_node;
};

struct flatten_pointer {
	struct interval_tree_node* node;
	size_t offset;
};

struct flatten_header {
	size_t memory_size;
	size_t ptr_count;
	size_t root_addr_count;
	uint64_t magic;
};

struct FLCONTROL {
	struct blstream* bhead;
	struct blstream* btail;
	struct rb_root fixup_set_root;
	struct rb_root imap_root;
	struct flatten_header	HDR;
	struct root_addrnode* rhead;
	struct root_addrnode* rtail;
	struct root_addrnode* last_accessed_root;
	int debug_flag;
	void* mem;
};

extern struct FLCONTROL FLCTRL;
extern struct flatten_pointer* flatten_plain_type(const void* _ptr, size_t _sz);
extern int fixup_set_insert(struct interval_tree_node* node, size_t offset, struct flatten_pointer* ptr);
extern struct flatten_pointer* get_pointer_node(const void* _ptr);
extern void root_addr_append(size_t root_addr);
void* root_pointer_next();

extern struct interval_tree_node * interval_tree_iter_first(struct rb_root *root, uintptr_t start, uintptr_t last);
extern struct rb_node* interval_tree_insert(struct interval_tree_node *node, struct rb_root *root);

struct blstream;
struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_insert_front(const void* data, size_t size, struct blstream* where);
struct blstream* binary_stream_append(const void* data, size_t size);
struct rb_node *rb_next(const struct rb_node *node);
struct rb_node *rb_prev(const struct rb_node *node);

static inline struct flatten_pointer* make_flatten_pointer(struct interval_tree_node* node, size_t offset) {
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

#define DBGM1(name,a1)					do { if (FLCTRL.debug_flag>=1) printf(#name "(" #a1 ")\n"); } while(0)
#define DBGM2(name,a1,a2)				do { if (FLCTRL.debug_flag>=1) printf(#name "(" #a1 "," #a2 ")\n"); } while(0)
#define DBGM3(name,a1,a2,a3)			do { if (FLCTRL.debug_flag>=1) printf(#name "(" #a1 "," #a2 "," #a3 ")\n"); } while(0)
#define DBGM4(name,a1,a2,a3,a4)			do { if (FLCTRL.debug_flag>=1) printf(#name "(" #a1 "," #a2 "," #a3 "," #a4 ")\n"); } while(0)

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

#define INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY(FLTYPE)	\
static inline struct flatten_pointer* flatten_struct_##FLTYPE##_array(const struct FLTYPE* _ptr, size_t n) {	\
    size_t _i;	\
	void* _fp_first=0;	\
	for (_i=0; _i<n; ++_i) {	\
		void* _fp = (void*)flatten_struct_##FLTYPE(_ptr+_i);	\
		if (!_fp_first) _fp_first=_fp;	\
		else free(_fp);	\
	}	\
    return _fp_first;	\
}

#define FUNCTION_DEFINE_FLATTEN_STRUCT(FLTYPE,...)	\
/* */		\
			\
struct flatten_pointer* flatten_struct_##FLTYPE(const struct FLTYPE* _ptr) {	\
			\
	typedef struct FLTYPE _container_type;	\
			\
	struct interval_tree_node *__node = interval_tree_iter_first(&FLCTRL.imap_root, (uint64_t)_ptr, (uint64_t)_ptr+sizeof(struct FLTYPE)-1);	\
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
		struct rb_node* rb = interval_tree_insert(__node, &FLCTRL.imap_root);	\
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

#define PTRNODE(PTRV)	(interval_tree_iter_first(&FLCTRL.imap_root, (uintptr_t)(PTRV), (uintptr_t)(PTRV)))
#define ROOT_POINTER_NEXT(PTRTYPE)	((PTRTYPE)(root_pointer_next()))
#define ROOT_POINTER_SEQ(PTRTYPE,n)	((PTRTYPE)(root_pointer_seq(n)))

#define FOR_ROOT_POINTER(p,...)	do {	\
		DBGM1(FOR_ROOT_POINTER,p);	\
		if (p) {	\
			struct flatten_pointer* __fptr = make_flatten_pointer(0,0);	\
			__VA_ARGS__;	\
			free(__fptr);	\
		}	\
		root_addr_append( (uintptr_t)(p) );	\
	} while(0)

#define FLATTEN_MEMORY_START	((unsigned char*)FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(size_t))
#define FLATTEN_MEMORY_END		(FLATTEN_MEMORY_START+FLCTRL.HDR.memory_size)

static inline void libflat_free (void* ptr) {

	if ( (FLCTRL.mem) && ((unsigned char*)ptr>=FLATTEN_MEMORY_START) && ((unsigned char*)ptr<FLATTEN_MEMORY_END) ) {
		/* Trying to free a part of unflatten memory. Do nothing */
	}
	else {
		/* Original free. Make sure "free" is not redefined at this point */
		free(ptr);
	}
}

static inline void* libflat_realloc (void* ptr, size_t size) {

	if ( (FLCTRL.mem) && ((unsigned char*)ptr>=FLATTEN_MEMORY_START) && ((unsigned char*)ptr<FLATTEN_MEMORY_END) ) {
		/* Trying to realloc a part of unflatten memory. Allocate new storage and let the part of unflatten memory fade away */
		void* m = malloc(size);
		if (m) return m; else return ptr;
	}
	else {
		/* Original realloc. Make sure "realloc" is not redefined at this point */
		return realloc(ptr,size);
	}
}

#endif /* __LIBFLAT_H__ */
