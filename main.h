#ifndef __MAIN_H__
#define __MAIN_H__

#include "libflat.h"

struct command {
    char* cmd;
    char** values;
    int size;
};

FUNCTION_DECLARE_FLATTEN_STRUCT(command);

struct dep {
    struct file** f;
    int n;
    char* fno;
    int* arr;
};

FUNCTION_DECLARE_FLATTEN_STRUCT(dep);

struct filearr;

struct file {
    char* name;
    char* value;
    int p;
    char** stp;
    struct command* c;
    struct file* next;
    struct file* prev;
    struct dep* d;
    struct filearr* farr;
};

FUNCTION_DECLARE_FLATTEN_STRUCT(file);

struct filearr {
	struct file* files;
};

INLINE_FUNCTION_DEFINE_FLATTEN_STRUCT_ARRAY_SIZE(file);
FUNCTION_DECLARE_FLATTEN_STRUCT(filearr);

#endif
