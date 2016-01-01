#ifndef __MAIN_H__
#define __MAIN_H__

#include "libflat.h"

struct command {
    char* cmd;
    char** values;
    int size;
    float**** deep_fp_value;
};

FUNCTION_DECLARE_FLATTEN_STRUCT(command);

struct dep {
    struct file** f;
    int n;
    char* fno;
    int* arr;
    int* pi[4];
    int pi_size[4];
} __attribute__((packed));

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

FUNCTION_DECLARE_FLATTEN_STRUCT(filearr);

#endif
