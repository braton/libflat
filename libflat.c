#include "libflat.h"
#include "private.h"

struct FLCONTROL FLCTRL = {
		.bhead = 0,
		.btail = 0,
		.fixup_set_root = RB_ROOT,
		.imap_root = RB_ROOT,
		.rhead = 0,
		.rtail = 0,
		.mem = 0,
		.last_accessed_root=0,
		.debug_flag=0
};

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
	assert(v!=0);
	memcpy(v->data,data,size);
    if (!FLCTRL.bhead) {
        FLCTRL.bhead = v;
        FLCTRL.btail = v;
    }
    else {
        v->prev = FLCTRL.btail;
        FLCTRL.btail->next = v;
        FLCTRL.btail = FLCTRL.btail->next;
    }
    return v;
}


struct blstream* binary_stream_append_reserve(size_t size) {
	struct blstream* v = calloc(1,sizeof(struct blstream));
	assert(v!=0);
	v->data = 0;
	v->size = size;
	if (!FLCTRL.bhead) {
        FLCTRL.bhead = v;
        FLCTRL.btail = v;
    }
    else {
        v->prev = FLCTRL.btail;
        FLCTRL.btail->next = v;
        FLCTRL.btail = FLCTRL.btail->next;
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
	assert(v!=0);
	memcpy(v->data,data,size);
	v->next = where;
	v->prev = where->prev;
	if (where->prev) {
		where->prev->next = v;
	}
	else {
		FLCTRL.bhead = v;
	}
	where->prev = v;
	return v;
}

struct blstream* binary_stream_insert_front_reserve(size_t size, struct blstream* where) {
	struct blstream* v = calloc(1,sizeof(struct blstream));
	assert(v!=0);
	v->data = 0;
	v->size = size;
	v->next = where;
	v->prev = where->prev;
	if (where->prev) {
		where->prev->next = v;
	}
	else {
		FLCTRL.bhead = v;
	}
	where->prev = v;
	return v;
}

struct blstream* binary_stream_insert_back(const void* data, size_t size, struct blstream* where) {
	struct blstream* v = create_binary_stream_element(size);
	assert(v!=0);
	memcpy(v->data,data,size);
	v->next = where->next;
	v->prev = where;
	if (where->next) {
		where->next->prev = v;
	}
	else {
		FLCTRL.btail = v;
	}
	where->next = v;
	return v;
}

struct blstream* binary_stream_insert_back_reserve(size_t size, struct blstream* where) {
	struct blstream* v = calloc(1,sizeof(struct blstream));
	assert(v!=0);
	v->data = 0;
	v->size = size;
	v->next = where->next;
	v->prev = where;
	if (where->next) {
		where->next->prev = v;
	}
	else {
		FLCTRL.btail = v;
	}
	where->next = v;
	return v;
}

void binary_stream_calculate_index() {
	struct blstream* p = FLCTRL.bhead;
	size_t index=0;
    while(p) {
    	struct blstream* cp = p;
    	p = p->next;
    	cp->index = index;
    	index+=cp->size;
    }
}

void binary_stream_destroy() {
	FLCTRL.btail = FLCTRL.bhead;
    while(FLCTRL.btail) {
    	struct blstream* p = FLCTRL.btail;
    	FLCTRL.btail = FLCTRL.btail->next;
    	free(p->data);
    	free(p);
    }
}

static void binary_stream_element_print(struct blstream* p) {
	size_t i;
	printf("(%zu){%zu}{%p}[ ",p->index,p->size,(void*)p);
	for (i=0; i<p->size; ++i) {
		printf("%02x ",((unsigned char*)(p->data))[i]);
	}
	printf("]\n");
}

static size_t binary_stream_element_write(struct blstream* p, FILE* f) {
	return fwrite((unsigned char*)(p->data),1,p->size,f);
}

void binary_stream_print() {

	struct blstream* cp = FLCTRL.bhead;
	size_t total_size = 0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	binary_stream_element_print(p);
    	total_size+=p->size;
    }
    printf("@ Total size: %zu\n",total_size);
}

size_t binary_stream_write(FILE* f) {
	struct blstream* cp = FLCTRL.bhead;
	size_t written=0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	written+=binary_stream_element_write(p,f);
    }
    return written;
}

size_t binary_stream_size() {
	
	struct blstream* cp = FLCTRL.bhead;
	size_t total_size = 0;
    while(cp) {
    	struct blstream* p = cp;
    	cp = cp->next;
    	total_size+=p->size;
    }
    return total_size;
}

void binary_stream_update_pointers() {
	struct rb_node * p = rb_first(&FLCTRL.fixup_set_root);
	int count=0;
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	void* newptr = (unsigned char*)node->ptr->node->storage->index+node->ptr->offset;
    	memcpy(&((unsigned char*)node->inode->storage->data)[node->offset],&newptr,sizeof(void*));
    	p = rb_next(p);
    	count++;
    }
}

#define ADDR_KEY(p)	((p)->inode->start + (p)->offset)

static struct fixup_set_node* create_fixup_set_node_element(struct interval_tree_node* node, size_t offset, struct flatten_pointer* ptr) {
	struct fixup_set_node* n = calloc(1,sizeof(struct fixup_set_node));
	assert(n!=0);
	n->inode = node;
	n->offset = offset;
	n->ptr = ptr;
	return n;
}

struct fixup_set_node *fixup_set_search(uintptr_t v) {
	struct rb_node *node = FLCTRL.fixup_set_root.rb_node;

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

int fixup_set_insert(struct interval_tree_node* node, size_t offset, struct flatten_pointer* ptr) {

	if (node==0) {
		free(ptr);
		return 0;
	}

	struct fixup_set_node* inode = fixup_set_search(node->start+offset);

	if (inode) {
		assert((inode->ptr->node->start+inode->ptr->offset)==(ptr->node->start+ptr->offset) && "Multiple pointer mismatch for the same storage");
		free(ptr);
		return 0;
	}

	struct fixup_set_node *data = create_fixup_set_node_element(node,offset,ptr);
	struct rb_node **new = &(FLCTRL.fixup_set_root.rb_node), *parent = 0;

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
	rb_insert_color(&data->node, &FLCTRL.fixup_set_root);

	return 1;
}

void fixup_set_print() {
	struct rb_node * p = rb_first(&FLCTRL.fixup_set_root);
	printf("[\n");
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	uintptr_t newptr = node->ptr->node->storage->index+node->ptr->offset;
    	uintptr_t origptr = node->inode->storage->index+node->offset;
    	printf(" %zu: (%p:%zu)->(%p:%zu) | %zu <- %zu\n",
    			node->inode->storage->index,
				node->inode,node->offset,
				node->ptr->node,node->ptr->offset,
				origptr,newptr);
    	p = rb_next(p);
    }
    printf("]\n");
}

size_t fixup_set_write(FILE* f) {
	size_t written=0;
	struct rb_node * p = rb_first(&FLCTRL.fixup_set_root);
	while(p) {
    	struct fixup_set_node* node = (struct fixup_set_node*)p;
    	size_t origptr = node->inode->storage->index+node->offset;
		size_t wr = fwrite(&origptr,sizeof(size_t),1,f);
		written+=wr*sizeof(size_t);
    	p = rb_next(p);
    }
    return written;
}

size_t fixup_set_count() {
	struct rb_node * p = rb_first(&FLCTRL.fixup_set_root);
	size_t count=0;
	while(p) {
		count++;
    	p = rb_next(p);
    }
    return count;
}

void fixup_set_destroy() {
	struct rb_node * p = rb_first(&FLCTRL.fixup_set_root);
	while(p) {
		struct fixup_set_node* node = (struct fixup_set_node*)p;
		rb_erase(p, &FLCTRL.fixup_set_root);
		p = rb_next(p);
		free(node->ptr);
		free(node);
	};
}

void root_addr_append(uintptr_t root_addr) {
    struct root_addrnode* v = calloc(1,sizeof(struct root_addrnode));
    assert(v!=0);
    v->root_addr = root_addr;
    if (!FLCTRL.rhead) {
        FLCTRL.rhead = v;
        FLCTRL.rtail = v;
    }
    else {
        FLCTRL.rtail->next = v;
        FLCTRL.rtail = FLCTRL.rtail->next;
    }
}

size_t root_addr_count() {
	struct root_addrnode* p = FLCTRL.rhead;
	size_t count = 0;
    while(p) {
    	count++;
    	p = p->next;
    }
    return count;
}

void interval_tree_print(struct rb_root *root) {
	struct rb_node * p = rb_first(root);
	size_t total_size=0;
	while(p) {
		struct interval_tree_node* node = (struct interval_tree_node*)p;
		printf("[%p:%p](%zu){%p}\n",(void*)node->start,(void*)node->last,node->last-node->start+1,(void*)node->storage);
		total_size+=node->last-node->start+1;
		p = rb_next(p);
	};
	printf("@ Total size: %zu\n",total_size);
}

void interval_tree_destroy(struct rb_root *root) {
	struct interval_nodelist *h = 0, *i = 0;
	struct rb_node * p = rb_first(root);
	while(p) {
		struct interval_tree_node* node = (struct interval_tree_node*)p;
		interval_tree_remove(node,&FLCTRL.imap_root);
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
	struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root, (uintptr_t)_ptr, (uintptr_t)_ptr+sizeof(void*)-1);
	if (node) {
		uintptr_t p = (uintptr_t)_ptr;
		assert(node->start<=p && "Invalid pointer address");
		assert(node->last>=p+sizeof(void*)-1 && "Invalid pointer address");
		return make_flatten_pointer(node,p-node->start);
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uintptr_t)_ptr;
		node->last = (uintptr_t)_ptr + sizeof(void*)-1;
		struct rb_node* rb = interval_tree_insert(node, &FLCTRL.imap_root);
		struct rb_node* prev = rb_prev(rb);
		struct blstream* storage;
		if (prev) {
			storage = binary_stream_insert_back(_ptr,sizeof(void*),((struct interval_tree_node*)prev)->storage);
		}
		else {
			struct rb_node* next = rb_next(rb);
			if (next) {
				storage = binary_stream_insert_front(_ptr,sizeof(void*),((struct interval_tree_node*)next)->storage);
			}
			else {
				storage = binary_stream_append(_ptr,sizeof(void*));
			}
		}
		node->storage = storage;		
		return make_flatten_pointer(node,0);
	}
}

struct flatten_pointer* flatten_plain_type(const void* _ptr, size_t _sz) {

	assert(_sz>0);
	struct interval_tree_node *node = interval_tree_iter_first(&FLCTRL.imap_root, (uintptr_t)_ptr, (uintptr_t)_ptr+_sz-1);
	struct flatten_pointer* r = 0;
	if (node) {
		uintptr_t p = (uintptr_t)_ptr;
		struct interval_tree_node *prev;
		while(node) {
			if (node->start>p) {
				assert(node->storage!=0);
				struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));
				assert(nn!=0);
				nn->start = p;
				nn->last = node->start-1;
				nn->storage = binary_stream_insert_front((void*)p,node->start-p,node->storage);
				interval_tree_insert(nn, &FLCTRL.imap_root);
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
			node = interval_tree_iter_next(node, (uintptr_t)_ptr, (uintptr_t)_ptr+_sz-1);
		}
		if ((uintptr_t)_ptr+_sz>p) {
			assert(prev->storage!=0);
			struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));
			assert(nn!=0);
			nn->start = p;
			nn->last = (uintptr_t)_ptr+_sz-1;
			nn->storage = binary_stream_insert_back((void*)p,(uintptr_t)_ptr+_sz-p,prev->storage);
			interval_tree_insert(nn, &FLCTRL.imap_root);
		}
		return r;
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uintptr_t)_ptr;
		node->last = (uintptr_t)_ptr + _sz-1;
		struct blstream* storage;
		struct rb_node* rb = interval_tree_insert(node, &FLCTRL.imap_root);
		struct rb_node* prev = rb_prev(rb);
		if (prev) {
			storage = binary_stream_insert_back(_ptr,_sz,((struct interval_tree_node*)prev)->storage);
		}
		else {
			struct rb_node* next = rb_next(rb);
			if (next) {
				storage = binary_stream_insert_front(_ptr,_sz,((struct interval_tree_node*)next)->storage);
			}
			else {
				storage = binary_stream_append(_ptr,_sz);
			}
		}
		node->storage = storage;		
		return make_flatten_pointer(node,0);
	}
}

void fix_unflatten_memory(struct flatten_header* hdr, void* memory) {
	size_t i;
	void* mem = (unsigned char*)memory+hdr->ptr_count*sizeof(size_t);
	for (i=0; i<hdr->ptr_count; ++i) {
		size_t fix_loc = *((size_t*)memory+i);
		uintptr_t ptr = (uintptr_t)( *((void**)((unsigned char*)mem+fix_loc)) );
		/* Make the fix */
		*((void**)((unsigned char*)mem+fix_loc)) = (unsigned char*)mem + ptr;
	}
}

void flatten_init() {
}

int flatten_write(FILE* ff) {
	size_t written = 0;
	binary_stream_calculate_index();
    binary_stream_update_pointers();
    DBGC(0,flatten_debug_info());
    FLCTRL.HDR.memory_size = binary_stream_size();
    FLCTRL.HDR.ptr_count = fixup_set_count();
    FLCTRL.HDR.root_addr_count = root_addr_count();
    FLCTRL.HDR.magic = FLATTEN_MAGIC;
    size_t wr = fwrite(&FLCTRL.HDR,sizeof(struct flatten_header),1,ff);
    if (wr!=1) return -1; written+=sizeof(struct flatten_header);
    struct root_addrnode* p = FLCTRL.rhead;
    while(p) {
    	size_t root_addr_offset;
    	if (p->root_addr) {
    		struct interval_tree_node *node = PTRNODE(p->root_addr);
			assert(node!=0);
			root_addr_offset = node->storage->index + (p->root_addr-node->start);
    	}
    	else {
    		root_addr_offset = (size_t)-1;
    	}
    	size_t wr = fwrite(&root_addr_offset,sizeof(size_t),1,ff);
    	if (wr!=1) return -1; else written+=wr*sizeof(size_t);
    	p = p->next;
    }
    wr = fixup_set_write(ff);
    if (wr!=FLCTRL.HDR.ptr_count*sizeof(size_t)) return -1; else written+=wr;
    wr = binary_stream_write(ff);
    if (wr!=FLCTRL.HDR.memory_size) return -1; else written+=wr;
    printf("# Flattening done. Summary:\n  Memory size: %zu bytes\n  Linked %zu pointers\n  Written %zu bytes\n",
    		FLCTRL.HDR.memory_size, FLCTRL.HDR.ptr_count, written);
    return 0;
}

void flatten_debug_info() {
	binary_stream_print();
    interval_tree_print(&FLCTRL.imap_root);
    fixup_set_print();
}

void flatten_fini() {
	binary_stream_destroy();
    interval_tree_destroy(&FLCTRL.imap_root);
    fixup_set_destroy();
    FLCTRL.rtail = FLCTRL.rhead;
    while(FLCTRL.rtail) {
    	struct root_addrnode* p = FLCTRL.rtail;
    	FLCTRL.rtail = FLCTRL.rtail->next;
    	free(p);
    }
}    

void unflatten_init() {
}

int unflatten_read(FILE* f) {

	TIME_MARK_START(unfl_b);
	size_t readin = 0;
	size_t rd = fread(&FLCTRL.HDR,sizeof(struct flatten_header),1,f);
	if (rd!=1) return -1; else readin+=sizeof(struct flatten_header);
	if (FLCTRL.HDR.magic!=FLATTEN_MAGIC) {
		fprintf(stderr,"Invalid magic while reading flattened image\n");
		return -1;
	}
	size_t i;
	for (i=0; i<FLCTRL.HDR.root_addr_count; ++i) {
		size_t root_addr_offset;
		size_t rd = fread(&root_addr_offset,sizeof(size_t),1,f);
		if (rd!=1) return -1; else readin+=sizeof(size_t);
		root_addr_append(root_addr_offset);
	}
	size_t memsz = FLCTRL.HDR.memory_size+FLCTRL.HDR.ptr_count*sizeof(size_t);
	FLCTRL.mem = malloc(memsz);
	assert(FLCTRL.mem);
	rd = fread(FLCTRL.mem,1,memsz,f);
	if (rd!=memsz) return -1; else readin+=rd;
	printf("# Unflattening done. Summary:\n");
	TIME_CHECK_FMT(unfl_b,read_e,"  Image read time: %fs\n");
	TIME_MARK_START(fix_b);
	fix_unflatten_memory(&FLCTRL.HDR,FLCTRL.mem);
	TIME_CHECK_FMT(fix_b,fix_e,"  Fixing memory time: %fs\n");
	TIME_CHECK_FMT(unfl_b,fix_e,"  Total time: %fs\n");
	printf("  Total bytes read: %zu\n",readin);
	return 0;
}

void unflatten_fini() {
	FLCTRL.rtail = FLCTRL.rhead;
    while(FLCTRL.rtail) {
    	struct root_addrnode* p = FLCTRL.rtail;
    	FLCTRL.rtail = FLCTRL.rtail->next;
    	free(p);
    }
    free(FLCTRL.mem);
}

void* root_pointer_next() {
	
	assert(FLCTRL.rhead!=0);

	if (FLCTRL.last_accessed_root==0) {
		FLCTRL.last_accessed_root = FLCTRL.rhead;
	}
	else {
		if (FLCTRL.last_accessed_root->next) {
			FLCTRL.last_accessed_root = FLCTRL.last_accessed_root->next;
		}
		else {
			assert(0);
		}
	}

	if (FLCTRL.last_accessed_root->root_addr==(size_t)-1) {
		return 0;
	}
	else {
		return (FLATTEN_MEMORY_START+FLCTRL.last_accessed_root->root_addr);
	}
}

void* root_pointer_seq(size_t index) {

	assert(FLCTRL.rhead!=0);

	FLCTRL.last_accessed_root = FLCTRL.rhead;

	size_t i=0;
	for (i=0; i<index; ++i) {
		if (FLCTRL.last_accessed_root->next) {
			FLCTRL.last_accessed_root = FLCTRL.last_accessed_root->next;
		}
		else {
			assert(0);
		}
	}

	if (FLCTRL.last_accessed_root->root_addr==(size_t)-1) {
		return 0;
	}
	else {
		return (FLATTEN_MEMORY_START+FLCTRL.last_accessed_root->root_addr);
	}
}

void flatten_set_debug_flag(int flag) {
	FLCTRL.debug_flag = flag;
}
