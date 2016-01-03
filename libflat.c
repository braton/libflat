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

static size_t binary_stream_element_write(struct blstream* p, FILE* f) {
	return fwrite((unsigned char*)(p->data),1,p->size,f);
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

size_t binary_stream_write(FILE* f) {
	struct blstream* cp = bhead;
	size_t written=0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	written+=binary_stream_element_write(p,f);
    }
    return written;
}

size_t binary_stream_size() {
	
	struct blstream* cp = bhead;
	size_t total_size = 0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	total_size+=p->size;
    }
    return total_size;
}

void binary_stream_update_pointers() {
	struct rb_node * p = rb_first(&fixup_set_root);
	int count=0;
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	void* newptr = (void*)node->ptr->node->storage->index+node->ptr->offset;
    	memcpy(&((unsigned char*)node->inode->storage->data)[node->offset],&newptr,sizeof(void*));
    	p = rb_next(p);
    	count++;
    }
}

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

struct rb_root fixup_set_root = RB_ROOT;

#define ADDR_KEY(p)	((unsigned long)(p)->inode + (p)->offset)

struct fixup_set_node* create_fixup_set_node_element(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr) {
	struct fixup_set_node* n = calloc(1,sizeof(struct fixup_set_node));
	assert(n!=0);
	n->inode = node;
	n->offset = offset;
	n->ptr = ptr;
	return n;
}

struct fixup_set_node *fixup_set_search(unsigned long v) {
	struct rb_node *node = fixup_set_root.rb_node;

	while (node) {
		struct fixup_set_node *data = container_of(node, struct fixup_set_node, node);

		if (v < ADDR_KEY(data)) {
			node = node->rb_left;
		}
		else if (v > ADDR_KEY(data)) {
			node = node->rb_right;
		}
		else
			return data;
	}

	return 0;
}

int fixup_set_insert(struct interval_tree_node* node, unsigned long offset, struct flatten_pointer* ptr) {

	struct fixup_set_node* inode = fixup_set_search((unsigned long)node+offset);

	if (inode) {
		assert(((unsigned long)inode->ptr->node+inode->ptr->offset)==((unsigned long)ptr->node+ptr->offset));
		free(ptr);
		return 0;
	}

	struct fixup_set_node *data = create_fixup_set_node_element(node,offset,ptr);
	struct rb_node **new = &(fixup_set_root.rb_node), *parent = 0;

	/* Figure out where to put new node */
	while (*new) {
		struct fixup_set_node *this = container_of(*new, struct fixup_set_node, node);

		parent = *new;
		if (ADDR_KEY(data) < ADDR_KEY(this))
			new = &((*new)->rb_left);
		else if (ADDR_KEY(data) > ADDR_KEY(this))
			new = &((*new)->rb_right);
		else
			return 0;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, &fixup_set_root);

	return 1;
}

void fixup_set_print() {
	struct rb_node * p = rb_first(&fixup_set_root);
	printf("[\n");
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	unsigned long newptr = node->ptr->node->storage->index+node->ptr->offset;
    	unsigned long origptr = node->inode->storage->index+node->offset;
    	printf(" %8lu: (%016lx:%lu)->(%016lx:%lu) | %8lu <- %8lu\n",node->inode->storage->index,(unsigned long)node->inode,node->offset,(unsigned long)node->ptr->node,node->ptr->offset,origptr,newptr);
    	p = rb_next(p);
    }
    printf("]\n");
}

size_t fixup_set_write(FILE* f) {
	size_t written=0;
	struct rb_node * p = rb_first(&fixup_set_root);
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	unsigned long origptr = node->inode->storage->index+node->offset;
    	written+=fwrite(&origptr,sizeof(unsigned long),1,f);
    	p = rb_next(p);
    }
    return written;
}

unsigned long fixup_set_count() {
	struct rb_node * p = rb_first(&fixup_set_root);
	unsigned long count=0;
	while(p) {
		count++;
    	p = rb_next(p);
    }
    return count;
}

void fixup_set_destroy() {
	struct rb_node * p = rb_first(&fixup_set_root);
	while(p) {
		struct fixup_set_node* node = (struct fixup_set_node*)p;
		rb_erase(p, &fixup_set_root);
		p = rb_next(p);
		free(node->ptr);
		free(node);
	};
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

struct flatten_pointer* get_pointer_node(const void* _ptr) {

	assert(_ptr!=0);
	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)_ptr, (uint64_t)_ptr+sizeof(void*)-1);
	if (node) {
		uint64_t p = (uint64_t)_ptr;
		assert(node->start<=p);
		assert(node->last>=p+sizeof(void*)-1);
		return make_flatten_pointer(node,p-node->start);
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uint64_t)_ptr;
		node->last = (uint64_t)_ptr + sizeof(void*)-1;
		node->storage = binary_stream_append(_ptr,sizeof(void*));
		interval_tree_insert(node, &imap_root);
		return make_flatten_pointer(node,0);
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

void fix_unflatten_memory(struct flatten_header* hdr, void* memory) {
	size_t i;
	void* mem = memory+hdr->ptr_count*sizeof(unsigned long);

	/*int j;
	printf("(%016lx)[ ",(unsigned long)mem);
	for (j=0; j<hdr->memory_size; ++j) {
		if ((j>0)&&(j%16==0)) printf("\n");
		printf("%02x ",((unsigned char*)(mem))[j]);
	}
	printf("]\n");
	int u=5;*/

	for (i=0; i<hdr->ptr_count; ++i) {
		unsigned long fix_loc = *((unsigned long*)memory+i);
		unsigned long ptr = *((unsigned long*)(mem+fix_loc));
		//printf("@ fix_loc(%lu) : ptr(%lu)\n",fix_loc,ptr);
		/* Make the fix */
		*((void**)(mem+fix_loc)) = mem + ptr;

		/*if (i<=u) {
			printf("(%016lx)[ ",(unsigned long)mem);
			for (j=0; j<hdr->memory_size; ++j) {
				if ((j>0)&&(j%16==0)) printf("\n");
				printf("%02x ",((unsigned char*)(mem))[j]);
			}
			printf("]\n");
		}*/
	}

	/*printf("(%016lx)[ ",(unsigned long)mem);
	for (j=0; j<hdr->memory_size; ++j) {
		if ((j>0)&&(j%16==0)) printf("\n");
		printf("%02x ",((unsigned char*)(mem))[j]);
	}
	printf("]\n");*/

}
