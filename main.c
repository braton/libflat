#include "main.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

FUNCTION_DEFINE_FLATTEN_STRUCT(command,
		AGGREGATE_FLATTEN_STRING(cmd);
		AGGREGATE_FLATTEN_TYPE_ARRAY(char*,values,ATTR(size));
		FOREACH_POINTER(const void*,p,ATTR(values),ATTR(size),
				FLATTEN_STRING(p);
			);
		
		AGGREGATE_FLATTEN_TYPE(float***,deep_fp_value);
		FOR_POINTER(float***,p,ATTR(deep_fp_value),
			FLATTEN_TYPE(float**,p);
			FOR_POINTER(float**,q,p,
				FLATTEN_TYPE(float*,q);
				FOR_POINTER(float*,r,q,
					FLATTEN_TYPE(float,r);
				);
			);
		);

	)

FUNCTION_DEFINE_FLATTEN_STRUCT(dep,
		AGGREGATE_FLATTEN_TYPE_ARRAY(struct file*,f,3);
		FOREACH_POINTER(const struct file*,p,ATTR(f),3,
				FLATTEN_STRUCT(file,p);
			);
		AGGREGATE_FLATTEN_STRING(fno);
		AGGREGATE_FLATTEN_TYPE_ARRAY(int,arr,5);
		int index=0;
		FOREACH_POINTER(int*,p,ATTR(pi),4,
				FLATTEN_TYPE_ARRAY(int,p,ATTR(pi_size)[index]);
				index++;
			);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(file,
		AGGREGATE_FLATTEN_STRING(name);
		AGGREGATE_FLATTEN_STRING(value);
		AGGREGATE_FLATTEN_TYPE_ARRAY(char*,stp,ptrarrmemlen((const void * const*)ATTR(stp)));
		FOREACH_POINTER(const char*,p,ATTR(stp),ptrarrmemlen((const void * const*)ATTR(stp)),
				FLATTEN_STRING(p);
			);
		AGGREGATE_FLATTEN_STRUCT(command,c);
		AGGREGATE_FLATTEN_STRUCT(file,next);
    	AGGREGATE_FLATTEN_STRUCT(file,prev);
    	AGGREGATE_FLATTEN_STRUCT(dep,d);
    	AGGREGATE_FLATTEN_STRUCT(filearr,farr);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(filearr,
		AGGREGATE_FLATTEN_STRUCT_ARRAY(file,files,8);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(list,
		AGGREGATE_FLATTEN_STRUCT(list,next);
		AGGREGATE_FLATTEN_STRING(name);
	)

static void print_int_array(int * ai, int indent, int len);
static void print_struct_dep(struct dep* d, int indent, int recurse);
static void print_struct_command(struct command* c, int indent);
static void print_struct_file(struct file* f, int indent, int recurse);
static void print_struct_filearr(struct filearr* farr, int indent, int recurse);
static void print_struct_list(struct list* l, int indent, int recurse);

static void print_struct_list(struct list* l, int indent, int recurse) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	printf("%sstruct list {\n",chr_indent);
	printf("%s\tname: (%s)\n",chr_indent,l->name);
	printf("%s\tnext: ",chr_indent);
	if (l->next) {
		printf("\n");
		if (recurse) {
			print_struct_list(l->next,indent+2,0);
		}
		else {
			printf("%s\t(...)\n",chr_indent);
		}
	}
	else {
		printf("0\n");
	}
	printf("%s};\n",chr_indent);
}

static void print_struct_command(struct command* c, int indent) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	printf("%sstruct command {\n",chr_indent);
	printf("%s\tcmd: (%s)\n",chr_indent,c->cmd);
	printf("%s\tvalues:\n",chr_indent);
	int i;
	for (i=0; i<c->size; ++i) {
		printf("%s\t\t(%s)\n",chr_indent,c->values[i]);
	}
	printf("%s\tsize: (%d)\n",chr_indent,c->size);
	printf("%s\tdeep_fp_value: ",chr_indent);
	if (c->deep_fp_value) {
		if (*c->deep_fp_value) {
			if (**c->deep_fp_value) {
				if (***c->deep_fp_value) {
					printf("**** -> *** -> ** -> -> * -> (%f)\n",****c->deep_fp_value);
				}
				else {
					printf("**** -> *** -> ** -> 0\n");
				}
			}
			else {
				printf("**** -> *** -> 0\n");
			}
		}
		else {
			printf("**** -> 0\n");
		}
	}
	else {
		printf("0\n");
	}

	printf("%s};\n",chr_indent);
}

static void print_struct_file(struct file* f, int indent, int recurse) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	printf("%sstruct file {\n",chr_indent);
	printf("%s\tname: (%s)\n",chr_indent,f->name);
	printf("%s\tvalue: (%s)\n",chr_indent,f->value);
	printf("%s\tp: (%d)\n",chr_indent,f->p);
	printf("%s\tstp:\n",chr_indent);
	int i;
	size_t sz = ptrarrmemlen((const void * const*)f->stp);
	for (i=0; i<sz; ++i) {
		printf("%s\t\t(%s)\n",chr_indent,f->stp[i]);
	}
	printf("%s\tc: ",chr_indent);
	if (f->c) {
		printf("\n");
		print_struct_command(f->c,indent+2);
	}
	else {
		printf("0\n");
	}
	printf("%s\tnext: ",chr_indent);
	if (f->next) {
		printf("\n");
		if (recurse) {
			print_struct_file(f->next,indent+2,0);
		}
		else {
			printf("%s\t(...)\n",chr_indent);
		}
	}
	else {
		printf("0\n");
	}
	printf("%s\tprev: ",chr_indent);
	if (f->prev) {
		printf("\n");
		if (recurse) {
			print_struct_file(f->prev,indent+2,0);
		}
		else {
			printf("%s\t(...)\n",chr_indent);
		}
	}
	else {
		printf("0\n");
	}
	printf("%s\tc: ",chr_indent);
	if (f->d) {
		printf("\n");
		if (recurse) {
			print_struct_dep(f->d,indent+2,0);
		}
		else {
			printf("%s\t(...)\n",chr_indent);
		}
	}
	else {
		printf("0\n");
	}
	printf("%s\tfarr: ",chr_indent);
	if (f->farr) {
		printf("\n");
		if (recurse) {
			print_struct_filearr(f->farr,indent+2,0);
		}
		else {
			printf("%s\t(...)\n",chr_indent);
		}
	}
	else {
		printf("0\n");
	}
	printf("%s};\n",chr_indent);
}

static void print_int_array(int * ai, int indent, int len) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	if (!ai) {
		printf("%s0\n",chr_indent);
		return;
	}
	int i;
	printf("%s[ ",chr_indent);
	for (i=0; i<len; ++i) {
		printf("%d ",ai[i]);
	}
	printf("]\n");
}

static void print_struct_filearr(struct filearr* farr, int indent, int recurse) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	printf("%sstruct filearr {\n",chr_indent);
	printf("%s\tfiles:\n",chr_indent);
	int i;
	if (farr->files) {
		printf("\n");
		for (i=0; i<8; ++i) {
			if (recurse) {
				print_struct_file(&farr->files[i],indent+2,1);
			}
			else {
				print_struct_file(&farr->files[i],indent+2,0);
			}
		}
	}
	else {
		printf("0\n");
	}
}

static void print_struct_dep(struct dep* d, int indent, int recurse) {
	char chr_indent[indent+1];
	memset(chr_indent,'\t',indent);
	chr_indent[indent] = 0;
	printf("%sstruct dep {\n",chr_indent);
	printf("%s\tf:\n",chr_indent);
	int i;
	for (i=0; i<3; ++i) {
		if (d->f[i]) {
			if (recurse) {
				print_struct_file(d->f[i],indent+2,1);
			}
			else {
				print_struct_file(d->f[i],indent+2,0);
			}
		}
		else {
			printf("%s\t\t0\n",chr_indent);
		}
	}
	printf("%s\tn: (%d)\n",chr_indent,d->n);
	printf("%s\tfno: (%s)\n",chr_indent,d->fno);
	printf("%s\tarr: \n",chr_indent);
	print_int_array(d->arr,indent+2,5);
	printf("%s\tpi: ",chr_indent);
	if (d->pi) {
		printf("\n");
	}
	else {
		printf("0\n");
	}
	for (i=0; i<4; ++i) {
		print_int_array(d->pi[i],indent+2,d->pi_size[i]);
	}
	printf("%s};\n",chr_indent);
}

char* st[] = {
    "string0",
    "string1",
    "string2",
    "string3",
    "string4",
    "string5",
    "string6",
    "string7",
    "string8",
    "string9",
    "string10",
    "string11",
    "string12",
    "string13",
    "string14",
    "string15",
    "string16",
    "string17",
    "string18",
    "string19",
    0
};

struct A {
	int k[10];
};

int main(void) {

//#define HASH_TABLE_TEST
#ifdef HASH_TABLE_TEST

#define LONG_ARR_SIZE	1000000

	srand (time(0));
	static unsigned long long_arr[LONG_ARR_SIZE];
	int i;
	struct hlist_node empty;
	hash_init(integer_set);
	for (i=0; i<LONG_ARR_SIZE; ++i) {
		long_arr[i] = ((unsigned long)rand() << 32) | (unsigned long)rand();
		hash_add(integer_set,&empty,long_arr[i]);
	}
	printf("%d\n",hash_has_key(integer_set,long_arr[0]));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	printf("%d\n",hash_has_key(integer_set,((unsigned long)rand() << 32) | (unsigned long)rand()));
	
	return 0;
#endif

//#define FLATTEN_ARRAY_TEST
#ifdef FLATTEN_ARRAY_TEST
	char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ#$*@0123456789";
	int ia[10] = {0,1,2,3,4,5,6,7,8,9};
	FLATTEN_TYPE_ARRAY(char,s+10,5);	// KLMNO
	FLATTEN_TYPE_ARRAY(char,s+20,3);	// UVW
	FLATTEN_TYPE_ARRAY(char,s+30,6);	// 012345
	FLATTEN_STRING(s); // full string
	FLATTEN_TYPE_ARRAY(int,ia,10);	// 0..9
	binary_stream_print();
    interval_tree_print(&imap_root);
    binary_stream_destroy();
    interval_tree_destroy(&imap_root);
	return 0;
#endif

//#define NODE_TEST
#ifdef NODE_TEST
	struct interval_tree_node* n1 = calloc(1,sizeof(struct interval_tree_node));
	n1->start=30;
	n1->last=100;
	struct interval_tree_node* n2 = calloc(1,sizeof(struct interval_tree_node));
	n2->start=130;
	n2->last=150;
	struct interval_tree_node* n3 = calloc(1,sizeof(struct interval_tree_node));
	n3->start=190;
	n3->last=210;
	struct interval_tree_node* n4 = calloc(1,sizeof(struct interval_tree_node));
	n4->start=220;
	n4->last=230;
	struct interval_tree_node* n5 = calloc(1,sizeof(struct interval_tree_node));
	n5->start=360;
	n5->last=420;
	struct interval_tree_node* n6 = calloc(1,sizeof(struct interval_tree_node));
	n6->start=500;
	n6->last=560;
	interval_tree_insert(n1, &imap_root);
	interval_tree_insert(n6, &imap_root);
	interval_tree_insert(n3, &imap_root);
	interval_tree_insert(n2, &imap_root);
	interval_tree_insert(n5, &imap_root);
	interval_tree_insert(n4, &imap_root);

	struct interval_tree_node *node;
	for (node = interval_tree_iter_first(&imap_root, 0, 10000); node;
	     node = interval_tree_iter_next(node, 0, 10000)) {
		printf("@ (%lu):(%lu)\n",node->start,node->last);
	}
	return 0;
#endif

#ifdef FLATTEN_TEST

	float fp_value = M_PI;
    float* pfp = &fp_value;
    float** ppfp = &pfp;
    float*** pppfp = &ppfp;
    float**** ppppfp = &pppfp;

    const char* list_names[] = {"list1","list2","list3","list4","list5","list6"};
    struct list* l1 = calloc(1,sizeof(struct list));
    assert(l1!=0);
    l1->name = list_names[0];
    struct list* l2 = calloc(1,sizeof(struct list));
    assert(l2!=0);
    l2->name = list_names[1];
    struct list* l3 = calloc(1,sizeof(struct list));
    assert(l3!=0);
    l3->name = list_names[2];
    struct list* l4 = calloc(1,sizeof(struct list));
    assert(l4!=0);
    l4->name = list_names[3];
    struct list* l5 = calloc(1,sizeof(struct list));
    assert(l5!=0);
    l5->name = list_names[4];
    struct list* l6 = calloc(1,sizeof(struct list));
    assert(l6!=0);
    l6->name = list_names[4];
    l1->next = l2;
    l2->next = l3;
    l3->next = l4;
    l4->next = l5;
    l5->next = l6;
    l6->next = l1;

	struct file f[8] = {};
	struct filearr farr = {f};
    struct command c[8] = {
    	{"command0",st+20-0,1,ppppfp},
    	{"command1",st+20-1,2,ppppfp},
    	{"command2",st+20-2,3,ppppfp},
    	{"command3",st+20-3,4,ppppfp},
    	{"command4",st+20-4,5},
    	{"command5",st+20-5,6},
    	{"command6",st+20-6,7},
    	{"command7",st+20-7,8}
    };

    struct file* darr0[3] = {&f[1],&f[2],&f[3]};
    struct file* darr2[3] = {&f[4],&f[5]};
    struct file* darr6[3] = {&f[3],&f[5],&f[7]};

    int iarr0[5] = {1,2,3,4,5};
    int iarr2[5] = {0,100};

    int pi1[3] = {0,1,2};
    int pi2[1] = {4};
    int pi3[6] = {6,7,8,9,10,11};
    int pi4[3] = {3,2};

    struct dep d0 = {darr0,3,"dep0",iarr0,{pi1,pi2,pi3,pi4},{3,1,6,3}};
    struct dep d2 = {darr2,3,"dep2",iarr2,{pi1,pi2,pi3,pi4},{3,1,6,3}};
    struct dep d6 = {darr6,3,"dep6"};

#define STRUCT_FILE_INIT(n) do {    \
    f[n].name = "file"#n;   \
    f[n].value = "value"#n; \
    f[n].p=n;   \
    f[n].stp=st+10+n;   \
    f[n].c=&c[n];   \
} while (0)

    STRUCT_FILE_INIT(0); f[0].next=&f[1]; f[0].prev=0; f[0].d = &d0;
    STRUCT_FILE_INIT(1); f[1].next=&f[2]; f[1].prev=&f[0]; f[1].d = 0;
    STRUCT_FILE_INIT(2); f[2].next=&f[0]; f[2].prev=&f[1]; f[2].d = &d2;
    STRUCT_FILE_INIT(3); f[3].next=&f[5]; f[3].prev=0; f[3].d = 0;
    STRUCT_FILE_INIT(4); f[4].next=0; f[4].prev=0; f[4].d = 0;
    STRUCT_FILE_INIT(5); f[5].next=&f[4]; f[5].prev=&f[3]; f[5].d = 0;
    STRUCT_FILE_INIT(6); f[6].next=&f[7]; f[6].prev=0; f[6].d = &d6;
    STRUCT_FILE_INIT(7); f[7].next=&f[5]; f[7].prev=0; f[7].d = 0;
    f[0].name = &f[1].name[2];
    f[0].farr = &farr;

    print_struct_file(&f[0],0,1);
    print_struct_file(&f[1],0,1);
    print_struct_file(&f[2],0,1);
    print_struct_file(&f[3],0,1);
    print_struct_file(&f[4],0,1);
    print_struct_file(&f[5],0,1);
    print_struct_file(&f[6],0,1);
    print_struct_file(&f[7],0,1);
    print_struct_list(l1,0,1);
    print_struct_list(l2,0,1);
    print_struct_list(l3,0,1);
    print_struct_list(l4,0,1);
    print_struct_list(l5,0,1);
    print_struct_list(l6,0,1);

	/* Flatten the struct file array structure */
	flatten_init();

	FOR_ROOT_POINTER(f,
		FLATTEN_STRUCT_ARRAY(file,f,8);
	);

	FOR_ROOT_POINTER(l1,
		FLATTEN_STRUCT(list,l1);
	);
	
	FILE* ff = fopen("flatten.dat","w");
	assert(ff!=0);
	int status = flatten_write(ff);
	assert(status==0);
	fclose(ff);

	flatten_fini();
	free(l1); free(l2); free(l3); free(l4); free(l5); free(l6);
	return 0;

#endif /* FLATTEN_TEST */

#ifdef UNFLATTEN_TEST

#define free libflat_free
#define realloc libflat_realloc

	/* Unflatten the struct file array structure */

	unflatten_init();

	FILE* ff = fopen("flatten.dat","r");
	int status = unflatten_read(ff);
	assert(status==0);
	fclose(ff);

	struct file* f = ROOT_POINTER_NEXT(struct file*);
	struct list* l = ROOT_POINTER_NEXT(struct list*);

	struct list* l1 = calloc(1,sizeof(struct list));
    assert(l1!=0);
    int * iarr = calloc(10,sizeof * iarr);
	
	/*struct list* l = ROOT_POINTER_SEQ(struct list*,1);
	struct file* f = ROOT_POINTER_SEQ(struct file*,0);*/

	/*struct file* f = ROOT_POINTER_SEQ(struct file*,0);
	struct list* l = ROOT_POINTER_NEXT(struct list*);*/
		
	print_struct_file(f+0,0,1);
    print_struct_file(f+1,0,1);
    print_struct_file(f+2,0,1);
    print_struct_file(f+3,0,1);
    print_struct_file(f+4,0,1);
    print_struct_file(f+5,0,1);
    print_struct_file(f+6,0,1);
    print_struct_file(f+7,0,1);
    print_struct_list(l,0,1);
    print_struct_list(l->next,0,1);
    print_struct_list(l->next->next,0,1);
    print_struct_list(l->next->next->next,0,1);
    print_struct_list(l->next->next->next->next,0,1);
    print_struct_list(l->next->next->next->next->next,0,1);
    free(l->next);
    void* plist = realloc(l->next,100);
    free(plist);
    free(l1);
    free(realloc(iarr,20 * sizeof * iarr));
    free(0);

    unflatten_fini();
	
	return 0;
#endif

}
