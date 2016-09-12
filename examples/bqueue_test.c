#include <stdio.h>
#include "bqueue.h"

int main(void) {

    struct bqueue bq;

    int tab[10] = {0,1,2,3,4,5,6,7,8,9};
    int ntab[10] = {0};

    bqueue_init(&bq,10);
    bqueue_push_back(&bq,tab,40);
    bqueue_push_back(&bq,tab,40);
    bqueue_push_back(&bq,tab,40);
    bqueue_pop_front(&bq,ntab,40);
    bqueue_pop_front(&bq,ntab,40);
    bqueue_pop_front(&bq,ntab,40);
    printf("empty(): %d\n",bqueue_empty(&bq));
    int i;
    printf("[ ");
    for (i=0; i<10; ++i) printf("%d ",ntab[i]);
    printf("]\n");
    bqueue_destroy(&bq);

    return 0;
}
