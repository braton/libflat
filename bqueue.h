#ifndef BQUEUE_H_
#define BQUEUE_H_

#include <stdlib.h>
#include "private.h"

struct queue_block {
    struct queue_block* next;
    unsigned char data[];
};

struct bqueue {
    size_t block_size;
    size_t size;
    struct queue_block* front_block;
    size_t front_index;
    struct queue_block* back_block;
    size_t back_index;
};

static inline void bqueue_init(struct bqueue* q, size_t block_size) {

    q->block_size = block_size;
    q->size = 0;
    q->front_block = malloc(block_size+sizeof(struct queue_block*));
    q->front_block->next = 0;
    q->back_block = q->front_block;
    q->front_index=0;
    q->back_index=0;
}

static inline void bqueue_destroy(struct bqueue* q) {

    struct queue_block* back = q->back_block;
    while(back) {
        struct queue_block* tmp = back;
        back = back->next;
        free(tmp);
    }
}

static inline int bqueue_empty(struct bqueue* q) {

    return q->size == 0;
}

static inline void bqueue_push_back(struct bqueue* q, const void* m, size_t s) {

    size_t copied = 0;
    while(s>0) {
        size_t avail_size = q->block_size-q->front_index;
        size_t copy_size = (s>avail_size)?(avail_size):(s);
        memcpy(q->front_block->data+q->front_index,m+copied,copy_size);
        copied+=copy_size;
        if (s>=avail_size) {
            s=s-avail_size;
            struct queue_block* new_block = malloc(q->block_size+sizeof(struct queue_block*));
            new_block->next = 0;
            q->front_block->next = new_block;
            q->front_block = new_block;
        }
        else s=0;
        q->front_index = (q->front_index+copy_size)%q->block_size;
    }
    q->size+=copied;
}

static inline void bqueue_pop_front(struct bqueue* q, void* m, size_t s) {

    assert(q->size>=s && "bqueue underflow");
    size_t copied = 0;

    while(s>0) {
        size_t avail_size = q->block_size-q->back_index;
        size_t copy_size = (s>avail_size)?(avail_size):(s);
        memcpy(m+copied,q->back_block->data+q->back_index,copy_size);
        copied+=copy_size;
        if (s>=avail_size) {
            s=s-avail_size;
            struct queue_block* tmp = q->back_block;
            q->back_block = q->back_block->next;
            free(tmp);
        }
        else s=0;
        q->back_index = (q->back_index+copy_size)%q->block_size;
    }
    q->size-=copied;
}

#endif /* BQUEUE_H_ */
