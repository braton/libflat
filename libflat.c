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

void binary_stream_calculate_index() {
	struct blstream* p = bhead;
	unsigned long index=0;
    while(p) {
    	struct blstream* cp = p;
    	p = p->next;
    	cp->index = index;
    	index+=cp->size;
    }
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

static void binary_stream_element_print(struct blstream* p) {
	size_t i;
	printf("(%lu){%lu}[ ",p->index,p->size);
	for (i=0; i<p->size; ++i) {
		printf("%02x ",((unsigned char*)(p->data))[i]);
	}
	printf("]\n");
}

void binary_stream_print() {

	struct blstream* cp = bhead;
	size_t total_size = 0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	binary_stream_element_print(p);
    	total_size+=p->size;
    }
    printf("@ Total size: %lu\n",total_size);
}

void binary_stream_update_pointers() {
	struct fixup_node* p = fhead;
	int count=0;
    while(p) {
    	struct fixup_node* cp = p;
    	p = p->next;
    	void* newptr = (void*)(cp->ptr->node->storage->index+cp->ptr->offset);
    	memcpy(&((unsigned char*)cp->node->storage->data)[cp->offset],&newptr,sizeof(void*));
    	count++;
    }
    printf("@ %d pointers udpated\n",count);
}

struct fixup_node *fhead,*ftail;

struct fixup_node* create_fixup_node_element(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr) {
	struct fixup_node* n = calloc(1,sizeof(struct fixup_node));
	assert(n!=0);
	n->node = node;
	n->offset = offset;
	n->ptr = ptr;
	return n;
}

void fixup_list_append(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr) {
	
	struct fixup_node* nn = create_fixup_node_element(node, offset, ptr);

	if (!fhead) {
        fhead = nn;
        ftail = nn;
    }
    else {
        ftail->next = nn;
        ftail = ftail->next;
    }
}

void fixup_list_destroy() {
	while(fhead) {
    	struct fixup_node* p = fhead;
    	fhead = fhead->next;
    	free(p->ptr);
    	free(p);
    }
}

void fixup_list_print() {
	printf("[ ");
	while(fhead) {
    	struct fixup_node* p = fhead;
    	fhead = fhead->next;
    	printf("(%016lx:%lu)->(%016lx:%lu) ",(unsigned long)p->node,p->offset,(unsigned long)p->ptr->node,p->ptr->offset);
    }
    printf("]\n");
}

void interval_tree_print(struct rb_root *root) {
	struct rb_node * p = rb_first(root);
	size_t total_size=0;
	while(p) {
		struct interval_tree_node* node = (struct interval_tree_node*)p;
		printf("[%016lx:%016lx]{%016lx}\n",node->start,node->last,(unsigned long)node->storage);
		total_size+=node->last-node->start+1;
		p = rb_next(p);	
	};
	printf("@ Total size: %lu\n",total_size);
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
    	free(p->node);
    	free(p);
    }
}

struct flatten_pointer* flatten_plain_type(const void* _ptr, size_t _sz) {

	assert(_sz>0);
	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)_ptr, (uint64_t)_ptr+_sz-1);
	struct flatten_pointer* r = 0;
	if (node) {
		uint64_t p = (uint64_t)_ptr;
		struct interval_tree_node *prev;
		while(node) {
			if (node->start>p) {
				assert(node->storage!=0);
				struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));
				assert(nn!=0);
				nn->start = p;
				nn->last = node->start-1;
				nn->storage = binary_stream_insert_front((void*)p,node->start-p,node->storage);
				interval_tree_insert(nn, &imap_root);
				if (r==0) {
					r = make_flatten_pointer(nn,0);
				}
			}
			else {
				if (r==0) {
					r = make_flatten_pointer(node,p-node->start);
				}
			}
			p = node->last+1;
			prev = node;
			node = interval_tree_iter_next(node, (uint64_t)_ptr, (uint64_t)_ptr+_sz-1);
		}
		if ((uint64_t)_ptr+_sz>p) {
			assert(prev->storage!=0);
			struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));
			assert(nn!=0);
			nn->start = p;
			nn->last = (uint64_t)_ptr+_sz-1;
			nn->storage = binary_stream_insert_back((void*)p,(uint64_t)_ptr+_sz-p,prev->storage);
			interval_tree_insert(nn, &imap_root);
		}
		return r;
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uint64_t)_ptr;
		node->last = (uint64_t)_ptr + _sz-1;
		node->storage = binary_stream_append(_ptr,_sz);
		interval_tree_insert(node, &imap_root);
		return make_flatten_pointer(node,0);
	}
}
