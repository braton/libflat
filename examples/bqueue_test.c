#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "libflat.h"

int main(void) {

    struct bqueue bq;
    size_t i,j,s,r,sz=0;
    unsigned char srcv[4096];
    unsigned char rstv[4096] = {0};

    bqueue_init(&bq,128*1024);

    srand(time(0));
    for (i=0; i<4096*16; ++i) {
        s = (size_t)(rand()%4096);
        for (j=0; j<s/sizeof(int); ++j) {
            ((int*)srcv)[j] = rand();
        }
        bqueue_push_back(&bq,srcv,s);
        sz+=s;
        r = (size_t)(rand()%4096);
        if (r>sz) r=sz;
        bqueue_pop_front(&bq,rstv,r);
        sz-=r;
    }

    if (sz>=s) {
        size_t m = sz-s;
        while(m) {
            size_t popn = (m>4096)?(4096):(m);
            bqueue_pop_front(&bq,rstv,popn);
            m-=popn;
            sz-=popn;
        }
        bqueue_pop_front(&bq,rstv,s);
        sz-=s;
    }
    else {
        memmove(rstv,rstv+r-(s-sz),s-sz);
        bqueue_pop_front(&bq,rstv+s-sz,sz);
        sz-=sz;
    }

    if (memcmp(srcv,rstv,s)) {
        printf("FAIL\n");
    }
    else printf("PASS\n");

    bqueue_destroy(&bq);
    return 0;
}
