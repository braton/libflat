## Libflat - library for fast serialization of C structures

Let's say you have a set data structure to store strings. It's implemented based on red-black tree. It has simple API as follows:<br>

```c
int stringset_insert(const char* s);
struct string_node* stringset_search(const char* s);
void stringset_print(struct rb_root* root);
```
You put some strings there and print its content:<br>
```c
stringset_insert("DAAA");
stringset_insert("AAAA");
stringset_insert("AAAB");
stringset_insert("BAAA");
stringset_insert("AAAC");
stringset_insert("AAAA");
stringset_insert("AAAA");
stringset_insert("BAAA");
stringset_print(&stringset_root);
```
You're satisfied with the output:<br>
```
[
  AAAA
  AAAB
  AAAC
  BAAA
  DAAA
]
```
Now you want to save the data for later processing. It's big and you want to access it quickly so you decide to serialize your string set and read-in all the content back later on.<br>

**Libflat is about how to do it as fast as possible!**<br>

Your base data structures that define the red-black tree are:<br>

```c
struct rb_root {
    struct rb_node *rb_node;
};

struct __attribute__((aligned(4))) rb_node {
    uintptr_t  __rb_parent_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
};

struct string_node {
    struct rb_node node;
    const char* s;
};
```
You write the recipes how to serialize them:<br>
```c
static inline const struct string_node* ptr_remove_color(const struct string_node* ptr) {
    return (const struct string_node*)( (uintptr_t)ptr & ~3 );
}

static inline struct flatten_pointer* fptr_add_color(struct flatten_pointer* fptr, const struct string_node* ptr) {
    fptr->offset |= (size_t)((uintptr_t)ptr & 3);
    return fptr;
}

FUNCTION_DEFINE_FLATTEN_STRUCT(string_node,
    STRUCT_ALIGN(4);
    AGGREGATE_FLATTEN_STRUCT_MIXED_POINTER(string_node, node.__rb_parent_color, ptr_remove_color, fptr_add_color);
    AGGREGATE_FLATTEN_STRUCT(string_node, node.rb_right);
    AGGREGATE_FLATTEN_STRUCT(string_node, node.rb_left);
    AGGREGATE_FLATTEN_STRING(s);
);

FUNCTION_DEFINE_FLATTEN_STRUCT(rb_root,
    AGGREGATE_FLATTEN_STRUCT(string_node, rb_node);
);
```
And make the actual serialization:<br>
```c
FOR_ROOT_POINTER(&stringset_root,
    FLATTEN_STRUCT(rb_root, &stringset_root);
);
```
```
$ examples/stringset-in_a
# Flattening done. Summary:
  Memory size: 196 bytes
  Linked 14 pointers
  Written 348 bytes
```
On the other side you read the entire tree back in:<br>
```c
const struct rb_root* root = ROOT_POINTER_NEXT(const struct rb_root*);
stringset_print(root);
```
```
$ examples/stringset-out_a
# Unflattening done. Summary:
  Image read time: 0.000033s
  Fixing memory time: 0.000001s
  Total time: 0.000063s
  Total bytes read: 348
[
  AAAA
  AAAB
  AAAC
  BAAA
  DAAA
]
```
"It was very small tree", you say. Ok, let's try a bigger one:<br>
```
$ examples/stringset-in_b 10000000
String set size: 9995017
# Flattening done. Summary:
  Memory size: 439780756 bytes
  Linked 29985050 pointers
  Written 679661196 bytes

real    1m44.941s
user    1m39.262s
sys     0m2.077s
```
You might want to increase stack size when serializing such large trees (as serialization is done recursively and stack overflow is lurking in the backgroud):<br>
```
ulimit -s 524288
```
Output:<br>
```
$ time examples/stringset-out_b
# Unflattening done. Summary:
  Image read time: 0.125965s
  Fixing memory time: 0.050811s
  Total time: 0.176827s
  Total bytes read: 679661196
String set size: 9995017

real    0m1.078s
user    0m0.949s
sys 0m0.128s
```
Here you are. In 15 lines of code you've made full serialization of a large tree that consumes more than 400MB of memory. Moreover you've read it back almost instantly!

You can find the above (and lots of other) examples in the example directory.<br>

###How to build
```
git clone git@github.com:braton/libflat.git
cd libflat
make
make examples
```
###How to test
```
export LD_LIBRARY_PATH=`pwd`
make test
examples/stringset-in_a
examples/stringset-out_a
(...)
```

###Documentation
[Full documentation with examples](http://libflat.codeawareness.com/)


###License
[GPL Version 2](http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
