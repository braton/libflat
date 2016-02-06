CC = gcc
AR = ar
LIBS = 
INCDIRS = 
CFLAGS = -Wno-address -Wno-missing-field-initializers

CFLAGS += -Wall -Wextra 
CFLAGS += -O2 -ggdb3
CFLAGS += -MD

LIB_CFLAGS += -fPIC
EXE_LDFLAGS += -static

LIB_SRC  := libflat.c rbtree.c interval_tree.c
TEST_SRC := main.c

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

$(OUTLIB): $(OBJ) $(LIBS)
	@echo "  [AR]   $@"
	$(AR) rcs $@ $^

$(OUTDYN): LDFLAGS += --shared $(LIB_LDFLAGS)
$(OUTDYN): $(LIB_OBJ) $(LIBS)
	@echo "  [LD]   $@"
	$(CC) $(LDFLAGS) -o $@ $^

$(OUTTEST): LDFLAGS += $(EXE_LDFLAGS)
$(OUTTEST): $(TEST_OBJ) $(OUTLIB)
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
	$(RM) $(OBJ) $(LIB_OBJ) $(TEST_OBJ) $(LIB_DEP) $(DEP) $(TEST_DEP) $(OUT) 

.PHONY : all clean static shared test
.SILENT : 
	
-include $(LIB_DEP) $(DEP) $(TEST_DEP)
