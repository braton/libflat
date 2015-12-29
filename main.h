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

struct file {
    char* name;
    char* value;
    int p;
    char** stp;
    struct command* c;
    struct file* next;
    struct file* prev;
    struct dep* d;
};

FUNCTION_DECLARE_FLATTEN_STRUCT(file);

#endif
