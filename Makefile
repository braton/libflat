__LIBFLAT_VERSION__:=$(shell grep "Libflat, version " libflat.h | python -c "import sys; print sys.stdin.read().split(\"\\n\")[0].split()[-1]")

CC = gcc
AR = ar
LIBS = 
INCDIRS = 
CFLAGS = -Wno-address -Wno-missing-field-initializers

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CFLAGS += -Wall -Wextra 
CFLAGS += -O0 -ggdb3
CFLAGS += -MD
CFLAGS += -D__LIBFLAT_VERSION__=\"$(__LIBFLAT_VERSION__)\"

LIB_CFLAGS += -fPIC
EXE_LDFLAGS +=

LIB_SRC  := libflat.c rbtree.c interval_tree.c
TEST_SRC := main.c

EXAMPLE_DIR := examples
EXAMPLE_SRC := $(wildcard $(EXAMPLE_DIR)/*.c)
EXAMPLE_OUT := $(basename $(EXAMPLE_SRC))
EXAMPLE_DEP := $(EXAMPLE_SRC:.c=.d)

LIB_OBJ := $(LIB_SRC:.c=.lib.o)
LIB_DEP := $(LIB_SRC:.c=.lib.d)

OBJ := $(LIB_SRC:.c=.o)
DEP := $(LIB_SRC:.c=.d)

TEST_OBJ := $(TEST_SRC:.c=.o)
TEST_DEP := $(TEST_SRC:.c=.d)

OUTLIB = libflat.a
OUTDYN = libflat.so
OUTTEST = flattest
MAKEFILE = Makefile

OUT := $(OUTLIB) $(OUTDYN) $(OUTTEST) $(EXAMPLE_OUT)
all: $(OUT)
	@echo "Built version $(__LIBFLAT_VERSION__)"

static: $(OUTLIB)
shared: $(OUTDYN)
test: $(OUTTEST)
	@python flattest.py flattest
examples: $(EXAMPLE_OUT)

define compile_example
$(1): LDFLAGS += $(EXE_LDFLAGS) -lm -lflat -L$(ROOT_DIR)
$(1): CFLAGS += -I$(ROOT_DIR) -Wno-unused-local-typedefs -Wno-unused-function
$(1): $(addsuffix .c,$(1)) $(OUTDYN)
	@echo "  [LD]   $$@"
	$$(CC) $$(CFLAGS) $$(addsuffix .c,$$@) $$(LDFLAGS) -o $$@
endef

$(foreach example,$(EXAMPLE_OUT),$(eval $(call compile_example,$(example))))

$(OUTLIB): $(OBJ) $(LIBS)
	@echo "  [AR]   $@"
	$(AR) rcs $@ $^

$(OUTDYN): LDFLAGS += --shared $(LIB_LDFLAGS)
$(OUTDYN): $(LIB_OBJ) $(LIBS)
	@echo "  [LD]   $@"
	$(CC) $(LDFLAGS) -o $@ $^

$(OUTTEST): LDFLAGS += $(EXE_LDFLAGS)
$(OUTTEST): $(TEST_OBJ) $(OUTDYN)
	@echo "  [LD]   $@"
	$(CC) $(LDFLAGS) -o $@ $^

$(LIB_OBJ): CFLAGS += $(LIB_CFLAGS)
$(LIB_OBJ): %.lib.o : %.c $(MAKEFILE)
	@echo "  [CC (pic)]   $@"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ) $(TEST_OBJ): %.o : %.c $(MAKEFILE)
	@echo "  [CC]   $@"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up..."
	$(RM) $(OBJ) $(LIB_OBJ) $(TEST_OBJ) $(LIB_DEP) $(DEP) $(TEST_DEP) $(OUT) $(EXAMPLE_OUT) $(EXAMPLE_DEP)

OUTHDR := $(OUTDYN:.so=.h)

install:
	@cp -f $(OUTHDR) /usr/local/include
	@cp -f $(OUTDYN) /usr/local/lib/$(OUTDYN).$(__LIBFLAT_VERSION__)
	@rm -f /usr/local/lib/$(OUTDYN)
	@ln -s /usr/local/lib/$(OUTDYN).$(__LIBFLAT_VERSION__) /usr/local/lib/$(OUTDYN) 
	@echo "Libflat successfully installed"

.PHONY : all clean install static shared test examples
.SILENT : 
	
-include $(LIB_DEP) $(DEP) $(TEST_DEP)
-include $(EXAMPLE_DEP)
