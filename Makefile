CC = gcc
AR = ar
LIBS = 
INCDIRS = 
CFLAGS = -Wno-address -Wno-missing-field-initializers

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CFLAGS += -Wall -Wextra 
CFLAGS += -O2 -ggdb3
CFLAGS += -MD

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

OUT := $(OUTLIB) $(OUTDYN) $(OUTTEST)
all: $(OUT) 

static: $(OUTLIB)
shared: $(OUTDYN)
test: $(OUTTEST)
	@python flattest.py flattest
examples: $(EXAMPLE_OUT)

define compile_example
$(1): LDFLAGS += $(EXE_LDFLAGS) -lm
$(1): CFLAGS += -I$(ROOT_DIR) -Wno-unused-local-typedefs
$(1): $(addsuffix .c,$(1)) $(OUTDYN)
	@echo "  [LD]   $$@"
	$$(CC) $$(CFLAGS) $$^ $$(LDFLAGS) -o $$@
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
$(LIB_OBJ): %.lib.o : %.c
	@echo "  [CC (pic)]   $@"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ) $(TEST_OBJ): %.o : %.c
	@echo "  [CC]   $@"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up..."
	$(RM) $(OBJ) $(LIB_OBJ) $(TEST_OBJ) $(LIB_DEP) $(DEP) $(TEST_DEP) $(OUT) $(EXAMPLE_OUT) $(EXAMPLE_DEP)

.PHONY : all clean static shared test examples
.SILENT : 
	
-include $(LIB_DEP) $(DEP) $(TEST_DEP)
-include $($(EXAMPLE_SRC):.c=.d)
