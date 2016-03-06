#include "libflat.h"
#include "private.h"
#include "stringset.h"

int main(void) {

	FILE* in = fopen("/tmp/memory.img", "r");
	assert(in);
	unflatten_init();

	assert(unflatten_read(in) == 0);

	const struct rb_root* root = ROOT_POINTER_NEXT(const struct rb_root*);

	stringset_print(root);

	unflatten_fini();
	fclose(in);

	return 0;
}
