#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "rte_ring.h"

#define RING_SIZE 16<<20

typedef struct cc_queue_node {
    int data;
} cc_queue_node_t;

static struct rte_ring *r;

typedef unsigned long long ticks;

static __inline__ ticks getticks(void)
{
    u_int32_t a, d;

    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return (((ticks)a) | (((ticks)d) << 32));
}


void *enqueue_fun(void *data)
{
    int n = (int)data;
    int i = 0;
    int ret;
    cc_queue_node_t *p;

    for (; i < n; i++) {
        p = (cc_queue_node_t *)malloc(sizeof(cc_queue_node_t));
        p->data = i;
        ret = rte_ring_mp_enqueue(r, p);
        if (ret != 0) {
            printf("enqueue failed: %d\n", i);
        }
    }

    return NULL;
}

void *dequeue_func(void *data)
{
    int ret;
    int i = 0;
    int sum = 0;
    int n = (int)data;
    cc_queue_node_t *p;
    ticks t1, t2, diff;
    //return;

    t1 = getticks();
    while (1) {
        p = NULL;
        ret = rte_ring_sc_dequeue(r, (void **)&p);
        if (ret != 0) {
            //do something
        }
        if (p != NULL) {
            i++;
            sum += p->data;
            free(p);
            if (i == n) {
                break;
            }
        }
    }

    t2 = getticks();
    diff = t2 - t1;
    printf("time diff: %llu\n", diff);
    printf("dequeue total: %d, sum: %d\n", i, sum);

    return NULL;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    pthread_t pid1, pid2, pid3, pid4, pid5, pid6;
    pthread_attr_t pthread_attr;

    r = rte_ring_create("test", RING_SIZE, 0);

    if (r == NULL) {
        return -1;
    }

    printf("start enqueue, 5 producer threads, echo thread enqueue 1000 numbers.\n");

    pthread_attr_init(&pthread_attr);
    if ((ret = pthread_create(&pid1, &pthread_attr, enqueue_fun, (void *)1000)) == 0) {
        pthread_detach(pid1);
    }

    if ((ret = pthread_create(&pid2, &pthread_attr, enqueue_fun, (void *)1000)) == 0) {
        pthread_detach(pid2);
    }

    if ((ret = pthread_create(&pid3, &pthread_attr, enqueue_fun, (void *)1000)) == 0) {
        pthread_detach(pid3);
    }
    
    if ((ret = pthread_create(&pid4, &pthread_attr, enqueue_fun, (void *)1000)) == 0) {
        pthread_detach(pid4);
    }

    if ((ret = pthread_create(&pid5, &pthread_attr, enqueue_fun, (void *)1000)) == 0) {
        pthread_detach(pid5);
    }

    printf("start dequeue, 1 consumer thread.\n");

    if ((ret = pthread_create(&pid6, &pthread_attr, dequeue_func, (void *)5000)) == 0) {
        //pthread_detach(pid6);
    }
    
    pthread_join(pid6, NULL);

    rte_ring_free(r);

    return 0;
}
