------------------------------------------------------------------------------------------------

static void stack_crash(int s) {
  char ch_arr[s];
  (void)ch_arr;
  int final = 0;
  (void)final;
  printf("ch_arr addr/size %016lx/%d\n",(unsigned long)ch_arr,s);
  printf("s addr/size: %016lx/%lu\n",(unsigned long)&s,sizeof s);
  printf("final addr/size: %016lx/%lu\n",(unsigned long)&final,sizeof final);
  printf("Press any key to double the stack size...\n");
  getchar();
  stack_crash(2*s);
}

stack_crash(1);

#if 0
address tree:
[600:680] [1140:1270] [4000:4040]
(ccff)		(50f6)		...
Address tree holds pointers to the storage area (blstream*)
If address map changes, corresponding storage area changes as well

binary_stream:
{ccff}[80]->{50f6}[130]->[40]

[blstream*]->[blstream*]->...

[20|*|40|*|4]->[60|*|62]->...

The data at the following addresses should be fixed (the rest of the data is ok):
blstream* : blstream_offset
i.e.:
ccff:20, ccff:68, 50f6:60 etc.
Where to store those values?

When storage is complete we will walk through storage area (linked blstream*) and compute total size and offset of each storage fragment (adding additional position field)
We will store linked list of offsets that need fixing. Fixing will be done by traversing whole address tree, getting each interval structure,
reading blstream* pointer and linked list of offsets, looking at each offset in blstream* (there should be address of interval node) and reading corresponding storage value of
interval node

#endif


#if 0
struct flatten_pointer* flatten_struct_file(struct file* f) {

	typedef struct file _container_type;

	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)f, (uint64_t)f+sizeof(struct file)-1);
	if (node) {
		assert(node->start==(uint64_t)f);
		assert(node->last==(uint64_t)f+sizeof(struct file)-1);
		return make_flatten_pointer(node,0);
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uint64_t)f;
		node->last = (uint64_t)f + sizeof(struct file)-1;
		node->storage = binary_stream_append_reserve(sizeof(struct file));
		interval_tree_insert(node, &imap_root);
	}

    FLATTEN_STRING(f,name);
    FLATTEN_STRING(f,value);
    //FLATTEN_STRING_ARRAY(f,stp);
	//FLATTEN_STRUCT(command,f,c);
    FLATTEN_STRUCT(file,f,next);
    FLATTEN_STRUCT(file,f,prev);
    //FLATTEN_STRUCT(dep,f,d);

    binary_stream_update(f,sizeof(struct file),node->storage);
    return make_flatten_pointer(node,0);
}
#endif


    //link_list_print();
    //link_list_destroy();


#if 0
struct linktab {
	struct linktab* next;
	void* req;
	void* addr;
	unsigned int field_offset;
};

extern struct linktab *lhead,*lptr;
void link_list_append(void* req, void* addr, unsigned int field_offset);
void link_list_destroy();
void link_list_print();
#endif


#if 0

unsigned long flatten_string(const char* s);
unsigned long flatten_string_array(const char** ps);
unsigned long flatten_string_array_size(const char** ps, size_t s);


#define FLATTEN_STRING(p,f)   do {  \
        if ((p)->f) {   \
            (p)->f = (void*)flatten_string((p)->f); \
        }   \
        else {  \
            (p)->f = (void*)0; \
        }   \
    } while(0)

#define FLATTEN_STRING_ARRAY(p,f)    do {    \
        if ((p)->f) {   \
            (p)->f = (void*)flatten_string_array((p)->f); \
        }   \
        else {  \
            (p)->f = (void*)0; \
        }   \
    } while(0)

#define FLATTEN_STRING_ARRAY_SIZE(p,f,s)    do {    \
        if ((p)->f) {   \
            (p)->f = (void*)flatten_string_array_size((p)->f,(s)); \
        }   \
        else {  \
            (p)->f = (void*)0; \
        }   \
    } while(0)
#endif

#if 0
#define FLATTEN_STRUCT(T,p,f)	do {	\
    	if ((p)->f) {	\
    		struct flatten_pointer* _p = flatten_struct_##T((p)->f);	\
		    if (_p->node->storage->data==0) {	\
		        link_list_append((p)->f,(p),offsetof(_container_type,f));	\
		    }	\
		    else {	\
		        (p)->f = (void*)_p;	\
		    }	\
		    APPEND_FIELD_OFFSET(f);	\
		}	\
	} while(0)
#endif


#if 0
struct linktab *lhead,*lptr;

static struct linktab* create_link_element() {
	struct linktab* n = calloc(1,sizeof(struct linktab));
	assert(n!=0);
	return n;
}

void link_list_append(void* req, void* addr, unsigned int field_offset) {
    struct linktab* v = create_link_element();
    v->req = req;
    v->addr = addr;
    v->field_offset = field_offset;
    if (!lhead) {
        lhead = v;
        lptr = v;
    }
    else {
        lptr->next = v;
        lptr = lptr->next;
    }
}

void link_list_print() {
	
	struct linktab* cp = lhead;
    while(cp) {
    	struct linktab* p = cp;
    	cp = cp->next;
    	printf("{REQ(%016lx)ADDR(%016lx)F(%u)}\n",(unsigned long)p->req,(unsigned long)p->addr,p->field_offset);
    }
}

void link_list_destroy() {
	lptr = lhead;
    while(lptr) {
    	struct linktab* p = lptr;
    	lptr = lptr->next;
    	free(p);
    }
}
#endif


#if 0
unsigned long flatten_string(const char* s) {

    //printf("@ flatten_string: IN(%08lx)\n",(unsigned long)s);
    if (MAP_HAS_KEY(ulong_type,flatmap,(uint64_t)s)) {
    	//printf("@ flatten_string: OUTx(%08lx)\n",MAP_ACCESS(ulong_type,flatmap,(uint64_t)s));
        return MAP_ACCESS(ulong_type,flatmap,(uint64_t)s);
    }
    else {
        size_t size = strlen(s)+1;
        unsigned long pos = bstream_pos();
        bstream_append((unsigned char*)s,size);
        MAP_INSERT(ulong_type,flatmap,(uint64_t)s,pos);
        //printf("@ flatten_string: OUTy(%08lx)\n",pos);
        return pos;
    }
}

unsigned long flatten_string_array(const char** ps) {
    if (MAP_HAS_KEY(ulong_type,flatmap,(uint64_t)ps)) {
        return MAP_ACCESS(ulong_type,flatmap,(uint64_t)ps);
    }
    else {
        const char** p = ps;
        unsigned i = 0;
        while(*p) {
        	//printf("@ flatten_string(%08lx)\n",(unsigned long)*p);
            *p = (void*)flatten_string(*p);
            //printf("@ flattened string(%08lx)\n",(unsigned long)*p);
            ++p; ++i;
        }
        unsigned long pos = bstream_pos();
        bstream_append((unsigned char*)ps,i*sizeof(char*));
        MAP_INSERT(ulong_type,flatmap,(uint64_t)ps,pos);
        return pos;
    }
}

unsigned long flatten_string_array_size(const char** ps, size_t s) {
    if (MAP_HAS_KEY(ulong_type,flatmap,(uint64_t)ps)) {
        return MAP_ACCESS(ulong_type,flatmap,(uint64_t)ps);
    }
    else {
        const char** p = ps;
        size_t i = 0;
        for (i=0; i<s; ++i) {
            if (p[i]) {
                p[i] = (void*)flatten_string(p[i]);
            }
            else {
                p[i] = (void*)-1;
            }
        }
        unsigned long pos = bstream_pos();
        bstream_append((unsigned char*)ps,i*sizeof(char*));
        MAP_INSERT(ulong_type,flatmap,(uint64_t)ps,pos);
        return pos;
    }
}
#endif




struct flatten_pointer* flatten_plain_char_size(const char* v, size_t size) {

	struct interval_tree_node *node = interval_tree_iter_first(&imap_root, (uint64_t)v, (uint64_t)v+size-1);
	struct flatten_pointer* r = 0;
	if (node) {
		uint64_t p = (uint64_t)v;
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
			node = interval_tree_iter_next(node, (uint64_t)v, (uint64_t)v+size-1);
		}
		if ((uint64_t)v+size>p) {
			assert(prev->storage!=0);
			struct interval_tree_node* nn = calloc(1,sizeof(struct interval_tree_node));
			assert(nn!=0);
			nn->start = p;
			nn->last = (uint64_t)v+size-1;
			nn->storage = binary_stream_insert_back((void*)p,(uint64_t)v+size-p,prev->storage);
			interval_tree_insert(nn, &imap_root);
		}
		return r;
	}
	else {
		node = calloc(1,sizeof(struct interval_tree_node));
		assert(node!=0);
		node->start = (uint64_t)v;
		node->last = (uint64_t)v + size-1;
		node->storage = binary_stream_append(v,size);
		interval_tree_insert(node, &imap_root);
		return make_flatten_pointer(node,0);
	}
}



// We will have lots of pointer structures with addresses in the storage. We have to change these addresses for new addresses from the storage


struct field_offset {
	struct field_offset* next;
	unsigned long offset;
};

struct field_offset* create_field_offset_element(unsigned long offset);
void field_offset_destroy(struct field_offset* head);
void field_offset_print(struct field_offset* head);


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


#define AGGREGATE_FLATTEN_POINTER_TYPE_ARRAY(T,f,n)   do {  \
    if (ATTR(f)) {   \
        fixup_list_append(node,offsetof(_container_type,f),flatten_plain_type(ATTR(f),(n)*sizeof(T*)));	\
    }   \
} while(0)



(void)__fptr;	\
/* __fptr->node; */	\
/* __fptr->offset; */	\

#define FLATTEN_TYPE(T,p)	(p)?((void*)flatten_plain_type((p),sizeof(T))):(0)

/*struct file* pf = f;
    FOR_POINTER(struct file*,fp,&pf,
		FLATTEN_STRUCT(file,fp);
	);*/
	struct file* pf = f;
    FOR_POINTER(struct file*,fp,&farr.files,
		FLATTEN_STRUCT_ARRAY(file,fp,8);
	);

	/*printf("get_pointer_node (%016lx)\n",(unsigned long)_ptr);*/

		#if 0
	struct file* pf = f;
    /*FOR_POINTER(struct file**,fp,&pf,
		FLATTEN_STRUCT_ARRAY(file,fp,8);
	);*/
	do {
      //struct file* const * _m = (struct file* const *)(&pf); 
      //struct file* fp = (_m)?(*(_m)):(0); 
      //struct flatten_pointer* __fptr = get_pointer_node(&fp);
		struct flatten_pointer* __fptr = make_flatten_pointer(0,0);
      do { 
        if (pf) {
        	fixup_list_append(__fptr->node,__fptr->offset,flatten_struct_file_array((pf),(8))); 
      	} 
      } while(0);
      free(__fptr);
    } while(0);
    #endif
    

    /*
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
}*/

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
	printf("[\n");
	while(fhead) {
    	struct fixup_node* p = fhead;
    	fhead = fhead->next;
    	unsigned long newptr = p->ptr->node->storage->index+p->ptr->offset;
    	unsigned long origptr = p->node->storage->index+p->offset;
    	printf(" %8lu: (%016lx:%lu)->(%016lx:%lu) | %8lu <- %8lu\n",p->node->storage->index,(unsigned long)p->node,p->offset,(unsigned long)p->ptr->node,p->ptr->offset,origptr,newptr);
    }
    printf("]\n");
}

		printf("i:[%016lx:%016lx](%lu){%016lx}\n",((struct interval_tree_node*)rb)->start,((struct interval_tree_node*)rb)->last,((struct interval_tree_node*)rb)->last - ((struct interval_tree_node*)rb)->start + 1,(unsigned long)((struct interval_tree_node*)rb)->storage);	\
		if (((struct interval_tree_node*)prev))	\
			printf("p:[%016lx:%016lx](%lu){%016lx}\n",((struct interval_tree_node*)prev)->start,((struct interval_tree_node*)prev)->last,((struct interval_tree_node*)prev)->last - ((struct interval_tree_node*)prev)->start + 1,(unsigned long)((struct interval_tree_node*)prev)->storage);	\
		else printf("p:[0]\n");	\



	/*printf("(%016lx)[ ",(unsigned long)mem);
	for (j=0; j<hdr->memory_size; ++j) {
		if ((j>0)&&(j%16==0)) printf("\n");
		printf("%02x ",((unsigned char*)(mem))[j]);
	}
	printf("]\n");*/



		/*if (i<=u) {
			printf("(%016lx)[ ",(unsigned long)mem);
			for (j=0; j<hdr->memory_size; ++j) {
				if ((j>0)&&(j%16==0)) printf("\n");
				printf("%02x ",((unsigned char*)(mem))[j]);
			}
			printf("]\n");
		}*/


		//printf("@ fix_loc(%lu) : ptr(%lu)\n",fix_loc,ptr);
		/* Make the fix */



	/*int j;
	printf("(%016lx)[ ",(unsigned long)mem);
	for (j=0; j<hdr->memory_size; ++j) {
		if ((j>0)&&(j%16==0)) printf("\n");
		printf("%02x ",((unsigned char*)(mem))[j]);
	}
	printf("]\n");
	int u=5;*/




static void
print_file_cmds (const void *item) {
	const struct file *f = item;
	printf("@ cmds: (%016lx)\n",(unsigned long)f->cmds);
	if (f->cmds) {
		printf("@ commands: (%s)\n",f->cmds->commands);
		printf("@ ncommand_lines: (%d)\n",f->cmds->ncommand_lines);
		printf("@ command_lines: (%016lx)\n",(unsigned long)f->cmds->command_lines);
		printf("@ lines_flags: (%016lx)\n",(unsigned long)f->cmds->lines_flags);
	}
}


	//hash_map (&files, print_file_cmds);





printf("@ fix_unflatten_memory() | fix_loc: %lu ptr: %016lx fixed: %016lx\n",fix_loc,ptr,(unsigned long)(mem + ptr));

	for (i=0; i<hdr->ptr_count; ++i) {
		unsigned long fix_loc = *((unsigned long*)memory+i);
		unsigned long ptr = *((unsigned long*)(mem+fix_loc));
		printf("@ fix_unflatten_memory() (sort done) | fix_loc: %lu ptr: %016lx fixed: %016lx\n",fix_loc,ptr,(unsigned long)(mem + ptr));
	}


	printf("@ flatmem_ptrcmp(): (%016lx) vs (%016lx)\n",memptra,memptrb);
	printf("@ flatmem_kptrcmp(): (%016lx) vs (%016lx)\n",*(unsigned long*)k,memptr);

printf("@ Root address offset: %lu\n",root_addr_offset);

	/* 	Sort the fix locations by the value of pointers at these locations.
		This will speed-up searching (by binary search) if a given pointer points to unflatten memory */
	qsort (FLCTRL.mem, FLCTRL.HDR.ptr_count, sizeof(unsigned long), flatmem_ptrcmp);

static inline int flatmem_ptrcmp(const void* pa,const void* pb) {

	unsigned long memptra = (unsigned long)(  *((void**)(FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(unsigned long) + *(unsigned long*)pa))  );
	unsigned long memptrb = (unsigned long)(  *((void**)(FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(unsigned long) + *(unsigned long*)pb))  );

	if ( memptra <  memptrb ) return -1;
	else if ( memptra ==  memptrb ) return 0;
	else return 1;
}

static inline int flatmem_kptrcmp(const void* k,const void* v) {

	unsigned long memptr = (unsigned long)(  *((void**)(FLCTRL.mem+FLCTRL.HDR.ptr_count*sizeof(unsigned long) + *(unsigned long*)v))  );

	if ( *(unsigned long*)k <  memptr ) return -1;
	else if ( *(unsigned long*)k ==  memptr ) return 0;
	else return 1;
}

  /*printf("Makefile cache: RCF (%016lx)(%s)\n",(unsigned long)read_cache_file,read_cache_file);
  printf("Makefile cache: MCF (%016lx)(%s)\n",(unsigned long)make_cache_file,make_cache_file);
  printf("Makefile cache: MCFS (%016lx)(%s)\n",(unsigned long)make_cache_file_stop,make_cache_file_stop);*/

  #define SZ 1000000
  srand (time(0));
  struct myType_ci* civ = malloc(sizeof(struct myType_ci)*SZ);
  struct myAlignedType_ci* aciv = malloc(sizeof(struct myType_ci)*SZ);
  unsigned u;
  for (u=0; u<SZ; ++u) {
    civ[u].c = rand();
    civ[u].i = 0;
    aciv[u].c = rand();
    aciv[u].i = 0;
  }
  
  unsigned long sum = 0;
  
  for (u=0; u<SZ; ++u) {
    sum+=civ[u].c;
  }
  printf("Sum of all char elements: %lu\n",sum);
  
  free(civ);
  free(aciv);




* 0 pattern_rules
* 1 suffix_file
* 2 pattern_vars
* 3 global_variable_set
* 4 global_setlist
* 5 files
* 6 default_file
* 7 default_goal_var
* 8 goals
* 9 read_files
10 vpaths
11 general_vpath
12 gpaths

-C DIRECTORY, --directory=DIRECTORY
-e, --environment-overrides
‘--eval=string’
-f FILE, --file=FILE, --makefile=FILE
-I DIRECTORY, --include-dir=DIRECTORY
-o FILE, --old-file=FILE, --assume-old=FILE
-r, --no-builtin-rules      Disable the built-in implicit rules
-R, --no-builtin-variables  Disable the built-in variable settings
-W FILE, --what-if=FILE, --new-file=FILE, --assume-new=FILE

make GOAL1 GOAL2 ... v=x ...

# variables
.DEFAULT_GOAL
MAKEFILE_LIST
MAKECMDGOALS
MAKE_RESTARTS
MAKE_TERMOUT
MAKE_TERMERR
.RECIPEPREFIX
.VARIABLES
.FEATURES
.INCLUDE_DIRS
# target names
.PHONY
.SUFFIXES
.DEFAULT
.PRECIOUS
.INTERMEDIATE
.SECONDARY
SECONDEXPANSION
.DELETE_ON_ERROR
.IGNORE
.LOW_RESOLUTION_TIME
.SILENT
.EXPORT_ALL_VARIABLES
.NOTPARALLEL
.ONESHELL
.POSIX

    PRINT_VARIABLE_VALUE("MAKE_TERMERR");
    die(0);



MAKELEVEL
MAKE_VERSION : ('# default', '4.1')
MAKE_HOST : ('# default', 'x86_64-unknown-linux-gnu')

MFLAGS
TODO: -*-eval-flags-*- :
TODO: default_defined
	  /* Ignore .RECIPEPREFIX and .VARIABLES special variables as they have only meaning in the context of reading makefiles */
	  /* TODO: check if commands can depend on those variables */
	  /* TODO: check if command line variables are properly accessible through PRINT_VARIABLE_VALUE just before update_goal_chain() */
	  /* Check the .SHELLFLAGS value and issue warning if different */ /* TODO: test it */

#if 0
	  2. environment difference + GNUMAKEFLAGS (env)
	  3. Different command line variables
	  Warning: unknown variable origin (# command line):(VAR1=value1)
	  Warning: unknown variable origin (# command line):(VAR2=value2)
	  1. check suffix rules that are executed before commands and pattern rules executed during commands?
			  /* Enter the default suffix rules as file rules.  This used to be done in
			     install_default_implicit_rules, but that loses because we want the
			     suffix rules installed before reading makefiles, and the pattern rules
			     installed after.  */
			  install_default_suffix_rules
#endif



## files

* after "stdin_makefiles_read" there's only files defined for command line goals.

after "default_defined" there's 72 more files defined:
.web.p
...
.info
If we use -r there's only 2 more defined:
.SUFFIXES
.DEFAULT

after "all_makefiles_read" there's many more files depending on the content of makefiles (1897)


## variables

after "stdin_makefiles_read" there's N variables from the environment , 10 default variables, 1 variable not from environment (GNUMAKEFLAGS) but marked as # environment (?)
and 1 makefile originated variable: *(CURDIR)
* (.INCLUDE_DIRS) : (/usr/local/include /usr/local/include /usr/include)
*(MAKE_COMMAND :) : (/home/bzator/Desktop/make-4.1/make)
*(MAKE) : ($(MAKE_COMMAND))
*(MAKE_TERMERR :) : (/dev/pts/12)
*(.VARIABLES :) : ()
*(MAKE_TERMOUT :) : (/dev/pts/12) {"this isn't present when stdout is piped to a file"}
*(MAKECMDGOALS :) : (adb adbd)
*(.FEATURES :) : (target-specific order-only second-expansion else-if shortest-stem undefine oneshell archives jobserver output-sync check-symlink load)
*(.RECIPEPREFIX :) : ()
*(.SHELLFLAGS :) : (-c)


When we pass two variables on command line additional "automatic" and "command line" variables are defined:
-*-command-variables-*- := VAR2=value2 VAR1=value1
VAR1 = value1
VAR2 = value2
MAKEOVERRIDES ('# environment', '${-*-command-variables-*-}')

When we pass strings to evaluate from command line the following variable (automatic) is defined:
-*-eval-flags-*- :

85 more variables are defined after "default_defined"
# 14 auto variables (defined also with -R)
(@F) : ($(notdir $@))
(+F) : ($(notdir $+))
(@D) : ($(patsubst %/,%,$(dir $@)))
(+D) : ($(patsubst %/,%,$(dir $+)))
(%D) : ($(patsubst %/,%,$(dir $%)))
(%F) : ($(notdir $%))
(<F) : ($(notdir $<))
(<D) : ($(patsubst %/,%,$(dir $<)))
(*D) : ($(patsubst %/,%,$(dir $*)))
(?F) : ($(notdir $?))
(*F) : ($(notdir $*))
(?D) : ($(patsubst %/,%,$(dir $?)))
(^D) : ($(patsubst %/,%,$(dir $^)))
(^F) : ($(notdir $^))
# 3 new makefile variables
(SHELL) : (/bin/sh)
(.DEFAULT_GOAL :) : ()
(MAKEFLAGS) : ()
# 2 new environment variables
(MAKELEVEL :) : (0)
(MFLAGS) : ()
# 67 new default variables
(F77FLAGS) : ($(FFLAGS))
(CHECKOUT,v) : (+$(if $(wildcard $@),,$(CO) $(COFLAGS) $< $@))
...
(M2C) : (m2c)
(WEAVE) : (weave)

with -R (and therefore -r) some more variables are defined (?):
MAKEFILES : ('# default', '')
sSUFFIXES : ('# default', '')


after "all_makefiles_read" there's many more variables defined in makefiles




-c, --cache-read           Read makefile cache\n"),
-m, --make-cache,           Create makefile cache and continue building targets
-M, --make-cache-stop,      Create makefile cache and exit before building any targets



------------------------------------------------------------------------
Other:
tpl,efl

Check:
Google protobuff to use libflat
https://github.com/thekvs/cpp-serializers
https://github.com/USCiLab/cereal


$ examples/circular-in 16384
# Flattening done. Summary:
  Memory size: 2147876895 bytes
  Linked 268435458 pointers
  Written 4295360599 bytes

# Unflattening done. Summary:
  Image read time: 0.756021s
  Fixing memory time: 0.828377s
  Total time: 1.584446s
  Total bytes read: 4295360599
Number of edges/diagonals: 134209536
Sum of lengths of edges/diagonals: 170891318.36704444885253906
Half of the circumference: 3.14159263433796188



<FOR_EACH_POINTER is not just a for with FOR_POINTER, it uses different storage for pointer each time>
Difference between structure and plain type? You cannot make a pointer to the middle of plain type but you can actually make a pointer to the middle of structure!
<root poninter is flattened?> root pointer must be the part of flattened memory; maybe we should add flattening root pointer if not present?
<NULL pointers how?>
<ptr that points inside long?>
<bit-fields and unions and C++ classes>



@ Implementation details
@ Limitations
	- pointers to functions
	- system elements
	- cannot free memory after deserialization
	<PRIMITIVES:>
		<casting and dumping part of structure memory into file: assert breaks dumping later: to enhance>
		<FOR_POINTER: each pointer cannot be partially flattened earlier>
	Padding
@ Conclusion (@ Final word)
@ Showcases
	- make cache
	- estrace
	- Python serialization
@ Troubleshooting
	<troubleshooting (when flattening crashes)>
@ Contribution
	<TODO LIST>
		<LICENSE do libflat>
		examples on Windows
		README to github
	Port to ARM
	Flattening pointer to functions through DLLADDR
	We have flattened static array struct table {...} through pointer &table but something inside our flattened memory had pointer to it
	Onefile compilation
	Fadeing away the memory at each delete
	1. Generating structure flattening descriptions using clang and parser
	2. Serialization protocol for read only and add it to Python

To conclude all thoughts let's get back to the serializing make internal data structures. ...

============================================
PLATFORM_VERSION_CODENAME=REL
PLATFORM_VERSION=6.0
TARGET_PRODUCT=aosp_arm64
TARGET_BUILD_VARIANT=eng
TARGET_BUILD_TYPE=release
TARGET_BUILD_APPS=
TARGET_ARCH=arm64
TARGET_ARCH_VARIANT=armv8-a
TARGET_CPU_VARIANT=generic
TARGET_2ND_ARCH=arm
TARGET_2ND_ARCH_VARIANT=armv7-a-neon
TARGET_2ND_CPU_VARIANT=cortex-a15
HOST_ARCH=x86_64
HOST_OS=linux
HOST_OS_EXTRA=Linux-3.13.0-58-generic-x86_64-with-Ubuntu-14.04-trusty
HOST_BUILD_TYPE=release
BUILD_ID=MRA58K
OUT_DIR=out
============================================
including ./abi/cpp/Android.mk ...
including ./art/Android.mk ...
including ./bionic/Android.mk ...
...
including ./system/security/softkeymaster/Android.mk ...
including ./system/vold/Android.mk ...
including ./tools/external/fat32lib/Android.mk ...
# Flattening done. Summary:
  Memory size: 467750822 bytes
  Linked 13075800 pointers
  Written 572357358 bytes

real	1m26.891s
user	0m45.526s
sys	0m39.327s

bzator@bzator-RedDragon:~/data/android/android6$ time /home/bzator/Desktop/make-4.1/make -c.cachefull6
# Unflattening done. Summary:
  Image read time: 0,101719s
  Fixing memory time: 0,033349s
  Total time: 0,135113s
  Total bytes read: 572357358
Warning: .INCLUDE_DIRS variable differs against cache. Makefiles need to be recreated.
make: Nothing to be done for 'droid'.

real	0m0.662s
user	0m0.465s
sys	0m0.200s
