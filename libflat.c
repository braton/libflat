#include "libflat.h"

struct blstream *bhead,*bptr;

static struct blstream* create_binary_stream_element(size_t size) {
	struct blstream* n = calloc(1,sizeof(struct blstream));
	assert(n!=0);
	void* m = calloc(1,size);
	assert(m!=0);
	n->data = m;
	n->size = size;
	return n;
}

struct blstream* binary_stream_append(const void* data, size_t size) {
	struct blstream* v = create_binary_stream_element(size);
	memcpy(v->data,data,size);
    if (!bhead) {
        bhead = v;
        bptr = v;
    }
    else {
        v->prev = bptr;
        bptr->next = v;
        bptr = bptr->next;
    }
    return v;
}


struct blstream* binary_stream_append_reserve(size_t size) {
	struct blstream* v = calloc(1,sizeof(struct blstream));
	assert(v!=0);
	v->data = 0;
	v->size = size;
	if (!bhead) {
        bhead = v;
        bptr = v;
    }
    else {
        v->prev = bptr;
        bptr->next = v;
        bptr = bptr->next;
    }
    return v;
}

struct blstream* binary_stream_update(const void* data, size_t size, struct blstream* where) {
	void* m = calloc(1,size);
	assert(m!=0);
	where->data = m;
	memcpy(where->data,data,size);
	return where;
}


struct blstream* binary_stream_insert_front(const void* data, size_t size, struct blstream* where) {
	struct blstream* v = create_binary_stream_element(size);
	memcpy(v->data,data,size);
	v->next = where;
	v->prev = where->prev;
	if (where->prev) {
		where->prev->next = v;
	}
	else {
		bhead = v;
	}
	where->prev = v;
	return v;
}

struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where) {
	struct blstream* v = create_binary_stream_element(size);
	memcpy(v->data,data,size);
	v->next = where->next;
	v->prev = where;
	if (where->next) {
		where->next->prev = v;
	}
	else {
		bptr = v;
	}
	where->next = v;
	return v;
}

void binary_stream_destroy() {
	bptr = bhead;
    while(bptr) {
    	struct blstream* p = bptr;
    	bptr = bptr->next;
    	free(p->data);
    	free(p);
    }
}

void binary_stream_print() {

	struct blstream* cp = bhead;
    while(cp) {
    	struct blstream* p = cp;
    	size_t i;
    	cp = cp->next;
    	printf("{%lu}[ ",p->size);
    	for (i=0; i<p->size; ++i) {
    		printf("%02x ",((unsigned char*)(p->data))[i]);
    	}
    	printf("]\n");
    }
}

struct field_offset* create_field_offset_element(unsigned long offset) {
	struct field_offset* n = calloc(1,sizeof(struct field_offset));
	assert(n!=0);
	n->offset = offset;
	return n;
}

void field_offset_destroy(struct field_offset* head) {
	while(head) {
    	struct field_offset* p = head;
    	head = head->next;
    	free(p);
    }
}

void field_offset_print(struct field_offset* head) {
	printf("[ ");
	while(head) {
    	struct field_offset* p = head;
    	head = head->next;
    	printf("%lu ",p->offset);
    }
    printf("]\n");
}

void interval_tree_print(struct rb_root *root) {
	struct rb_node * p = rb_first(root);
	while(p) {
		struct interval_tree_node* node = (struct interval_tree_node*)p;
		printf("[%016lx:%016lx]{%016lx}",node->start,node->last,(unsigned long)node->storage);
		field_offset_print(node->poffs_head);
		p = rb_next(p);	
	};
}

void interval_tree_destroy(struct rb_root *root) {
	struct interval_nodelist *h = 0, *i = 0;
	struct rb_node * p = rb_first(root);
	while(p) {
		struct interval_tree_node* node = (struct interval_tree_node*)p;
		interval_tree_remove(node,&imap_root);
		struct interval_nodelist* v = calloc(1,sizeof(struct interval_nodelist));
		assert(v!=0);
	    v->node = node;
	    if (!h) {
	        h = v;
	        i = v;
	    }
	    else {
	        i->next = v;
	        i = i->next;
	    }
		p = rb_next(p);
	};
	while(h) {
    	struct interval_nodelist* p = h;
    	h = h->next;
    	field_offset_destroy(p->node->poffs_head);
    	free(p->node);
    	free(p);
    }
}

FUNCTION_DEFINE_FLATTEN_PLAIN_TYPE(char);
FUNCTION_DEFINE_FLATTEN_PLAIN_TYPE(int);
