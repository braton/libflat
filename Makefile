all:
	@gcc -o flattest main.c libflat.c interval_tree.c rbtree.c -ggdb3 -Wall -Wno-address
