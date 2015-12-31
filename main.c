#include "main.h"

FUNCTION_DEFINE_FLATTEN_STRUCT(command,
		AGGREGATE_FLATTEN_STRING(cmd);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(dep,
		AGGREGATE_FLATTEN_STRING(fno);
		AGGREGATE_FLATTEN_TYPE_ARRAY_SIZE(int,arr,5);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(file,
		AGGREGATE_FLATTEN_STRING(name);
		AGGREGATE_FLATTEN_STRING(value);
		AGGREGATE_FLATTEN_STRUCT(command,c);
		AGGREGATE_FLATTEN_STRUCT(file,next);
    	AGGREGATE_FLATTEN_STRUCT(file,prev);
    	AGGREGATE_FLATTEN_STRUCT(dep,d);
    	AGGREGATE_FLATTEN_STRUCT(filearr,farr);
	)

FUNCTION_DEFINE_FLATTEN_STRUCT(filearr,
		AGGREGATE_FLATTEN_STRUCT_ARRAY_SIZE(file,files,8);
	)

struct rb_root imap_root = RB_ROOT;

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

int main(void) {

//#define FLATTEN_ARRAY_TEST
#ifdef FLATTEN_ARRAY_TEST
	char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ#$*@0123456789";
	int ia[10] = {0,1,2,3,4,5,6,7,8,9};
	FLATTEN_TYPE_ARRAY_SIZE(char,s+10,5);	// KLMNO
	FLATTEN_TYPE_ARRAY_SIZE(char,s+20,3);	// UVW
	FLATTEN_TYPE_ARRAY_SIZE(char,s+30,6);	// 012345
	FLATTEN_STRING(s); // full string
	FLATTEN_TYPE_ARRAY_SIZE(int,ia,10);	// 0..9
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

	struct file f[8] = {};
	struct filearr farr = {f};
    struct command c[8] = {
    	{"command0",st+20-0,0},
    	{"command1",st+20-1,1},
    	{"command2",st+20-2,2},
    	{"command3",st+20-3,3},
    	{"command4",st+20-4,4},
    	{"command5",st+20-5,5},
    	{"command6",st+20-6,6},
    	{"command7",st+20-7,7}
    };

    struct file* darr0[3] = {&f[1],&f[2],&f[3]};
    struct file* darr2[2] = {&f[4],&f[5]};
    struct file* darr6[3] = {&f[3],&f[5],&f[7]};

    int iarr0[5] = {1,2,3,4,5};
    int iarr2[5] = {0,100};

    struct dep d0 = {darr0,3,"dep0",iarr0};
    struct dep d2 = {darr2,2,"dep2",iarr2};
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

    FLATTEN_STRUCT(file,&f[0]);
    FLATTEN_STRUCT_ARRAY_SIZE(file,&farr.files[0],8);

    binary_stream_print();
    interval_tree_print(&imap_root);

    binary_stream_destroy();
    interval_tree_destroy(&imap_root);

	return 0;
}
