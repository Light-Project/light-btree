/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2021 Sanpe <sanpeqf@gmail.com>
 */

#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>

#define BTREE_DEBUG 0
#define TEST_LEN 1000000

struct bench_node {
    unsigned int num;
    unsigned long data;
};

#if BTREE_DEBUG
static void node_dump(struct bench_node *node)
{
    printf("  %04d: ", node->num);
    printf("data %#018lx ", node->data);
    printf("\n");
}
#else
# define node_dump(node) ((void)(node))
#endif

static void time_dump(int ticks, clock_t start, clock_t stop, struct tms *start_tms, struct tms *stop_tms)
{
    printf("  real time: %lf\n", (stop - start) / (double)ticks);
    printf("  user time: %lf\n", (stop_tms->tms_utime - start_tms->tms_utime) / (double)ticks);
    printf("  kern time: %lf\n", (stop_tms->tms_stime - start_tms->tms_stime) / (double)ticks);
}

int main(void)
{
    struct bench_node *node, *tnode;
    struct tms start_tms, stop_tms;
    clock_t start, stop;
    unsigned int count, ticks;
    uintptr_t key, tkey;
    int ret = 0;

    BTREE_ROOT(bench_root, &btree_layoutptr,
        btree_alloc, btree_free, btree_keyfind,
        NULL, NULL, NULL
    );

    ticks = sysconf(_SC_CLK_TCK);

    printf("Generate %u Node:\n", TEST_LEN);
    start = times(&start_tms);
    for (count = 0; count < TEST_LEN; ++count) {
        node = malloc(sizeof(*node));
        if ((ret = !node)) {
            printf("Insufficient Memory!\n");
            goto error;
        }

        node->num = count + 1;
        node->data = ((unsigned long)rand() << 32) | rand();

#if BTREE_DEBUG
        printf("  %08d: 0x%016lx\n", node->num, node->data);
#endif

        btree_insert(&bench_root, &node->data, node);
    }
    stop = times(&stop_tms);
    time_dump(ticks, start, stop, &start_tms, &stop_tms);

    start = times(&start_tms);
    count = 0;
    printf("Btree Iteration:\n");
    btree_for_each(&bench_root, &key, node) {
        node_dump(node);
        count++;
    }
    stop = times(&stop_tms);
    printf("  total num: %u\n", count);
    time_dump(ticks, start, stop, &start_tms, &stop_tms);

    start = times(&start_tms);
    count = 0;
    printf("Btree Reverse Iteration:\n");
    btree_for_each_reverse(&bench_root, &key, node) {
        node_dump(node);
        count++;
    }
    stop = times(&stop_tms);
    printf("  total num: %u\n", count);
    time_dump(ticks, start, stop, &start_tms, &stop_tms);

    printf("Deletion All Node...\n");
error:
    btree_for_each_safe(&bench_root, &key, node, &tkey, tnode) {
        btree_remove(&bench_root, &key);
        free(node);
    }
    btree_destroy(&bench_root);

    if (!ret)
        printf("Done.\n");
    else
        printf("Abort.\n");

    return ret;
}
