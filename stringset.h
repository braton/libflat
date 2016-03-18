#ifndef STRINGSET_H_
#define STRINGSET_H_

#include "rbtree.h"

static struct rb_root stringset_root = RB_ROOT;

struct string_node {
	struct rb_node node;
	const char* s;
};

static struct string_node* stringset_search(const char* s) {

	struct rb_node *node = stringset_root.rb_node;

	while (node) {
		struct string_node* data = container_of(node, struct string_node, node);

		if (strcmp(s,data->s)<0) {
			node = node->rb_left;
		}
		else if (strcmp(s,data->s)>0) {
			node = node->rb_right;
		}
		else
			return data;
	}

	return 0;
}

static int stringset_insert(const char* s) {

	struct string_node* data = calloc(1,sizeof(struct string_node));
	data->s = s;
	struct rb_node **new = &(stringset_root.rb_node), *parent = 0;

	/* Figure out where to put new node */
	while (*new) {
		struct string_node* this = container_of(*new, struct string_node, node);

		parent = *new;
		if (strcmp(data->s,this->s)<0)
			new = &((*new)->rb_left);
		else if (strcmp(data->s,this->s)>0)
			new = &((*new)->rb_right);
		else
			return 0;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, &stringset_root);

	return 1;
}

static void stringset_print(const struct rb_root* root) {

	struct rb_node * p = rb_first(root);
	printf("[\n");
	while(p) {
		struct string_node* data = (struct string_node*)p;
		printf("  %s\n",data->s);
		p = rb_next(p);
	}
	printf("]\n");
}

static size_t stringset_count(const struct rb_root* root) {

	struct rb_node * p = rb_first(root);
	size_t count = 0;
	while(p) {
		count++;
		p = rb_next(p);
	}
	return count;
}

#endif /* STRINGSET_H_ */
