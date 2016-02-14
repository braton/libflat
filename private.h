#ifndef __PRIVATE_H__
#define __PRIVATE_H__

#include <string.h>
#include "config.h"
#include "interval_tree.h"
#ifdef __linux__
#include <sys/time.h>
#else
#ifdef _WIN32
#include <Windows.h>
#endif
#endif

#ifdef _WIN32
/*
* gettimeofday.c
*	  Win32 gettimeofday() replacement
*
* Copyright (c) 2003 SRA, Inc.
* Copyright (c) 2003 SKC, Inc.
*
* Permission to use, copy, modify, and distribute this software and
* its documentation for any purpose, without fee, and without a
* written agreement is hereby granted, provided that the above
* copyright notice and this paragraph and the following two
* paragraphs appear in all copies.
*
* IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
* LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
* DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
* THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS
* IS" BASIS, AND THE AUTHOR HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE,
* SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

/* FILETIME of Jan 1 1970 00:00:00, the PostgreSQL epoch */
static const unsigned __int64 epoch = (__int64)116444736000000000LL;

static inline int
gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	FILETIME	file_time;
	SYSTEMTIME  system_time;
	ULARGE_INTEGER ularge;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	ularge.LowPart = file_time.dwLowDateTime;
	ularge.HighPart = file_time.dwHighDateTime;

	tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

	return 0;
}
#endif

#define TIME_MARK_START(start_marker)		\
		struct timeval  tv_mark_##start_marker;	\
		gettimeofday(&tv_mark_##start_marker, 0)
#define TIME_CHECK_ON(start_marker,end_marker)	do {	\
		struct timeval  tv_mark_##start_marker##_##end_marker;	\
		gettimeofday(&tv_mark_##start_marker##_##end_marker, 0);	\
		printf("@Elapsed ("#start_marker" -> "#end_marker"): (%f)[s]\n",	\
		(double) (tv_mark_##start_marker##_##end_marker.tv_usec - tv_mark_##start_marker.tv_usec) / 1000000 +	\
		         (double) (tv_mark_##start_marker##_##end_marker.tv_sec - tv_mark_##start_marker.tv_sec) );	\
	} while(0)
#define TIME_CHECK_FMT(start_marker,end_marker,fmt)	do {	\
		struct timeval  tv_mark_##start_marker##_##end_marker;	\
		gettimeofday(&tv_mark_##start_marker##_##end_marker, 0);	\
		printf(fmt,	\
		(double) (tv_mark_##start_marker##_##end_marker.tv_usec - tv_mark_##start_marker.tv_usec) / 1000000 +	\
		         (double) (tv_mark_##start_marker##_##end_marker.tv_sec - tv_mark_##start_marker.tv_sec) );	\
	} while(0)

#define DBGL(n,...)		do { if (n<=CONFIG_DEBUG_LEVEL)	printf(__VA_ARGS__); } while(0)

#define FLATTEN_MAGIC 0x464c415454454e00ULL

/* Private debugging facilities. In order to use them include their prototypes in the source file */
void flatten_debug_info();
void flatten_set_debug_flag(int flag);

/* Binary stream doubly linked list implementation */

struct blstream {
	struct blstream* next;
	struct blstream* prev;
	void* data;
	size_t size;
	size_t index;
};

struct blstream* binary_stream_append_reserve(size_t size);
struct blstream* binary_stream_insert_front_reserve(size_t size, struct blstream* where);
struct blstream* binary_stream_insert_back_reserve(size_t size, struct blstream* where);
struct blstream* binary_stream_update(const void* data, size_t size, struct blstream* where);
void binary_stream_calculate_index();
void binary_stream_update_pointers();
void binary_stream_destroy();
void binary_stream_print();
size_t binary_stream_size();
size_t binary_stream_write(FILE* f);

/* Set of memory fixup elements implemented using red-black tree */

struct fixup_set_node {
	struct rb_node node;
  	/* Storage area and offset where the original address to be fixed is stored */
	struct interval_tree_node* inode;
	size_t offset;
	/* Storage area and offset where the original address points to */
	struct flatten_pointer* ptr;
};

struct fixup_set_node* fixup_set_search(uintptr_t v);
void fixup_set_print();
size_t fixup_set_count();
void fixup_set_destroy();
size_t fixup_set_write(FILE* f);

/* Root address list */
struct root_addrnode {
	struct root_addrnode* next;
	uintptr_t root_addr;
};
size_t root_addr_count();

void fix_unflatten_memory(struct flatten_header* hdr, void* memory);

#ifdef __linux__
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
#ifdef _WIN32
#define container_of(ptr, type, member) (type *)( (char *)(ptr) - offsetof(type,member) )
#endif
#endif

#define DBGC(b,...)		do { if (b!=0)	__VA_ARGS__; } while(0)

struct interval_nodelist {
	struct interval_nodelist* next;
	struct interval_tree_node* node;
};

#endif /* __PRIVATE_H__ */
