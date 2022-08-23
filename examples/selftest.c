/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2022 Sanpe <sanpeqf@gmail.com>
 */

#include "btree.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_LOOP 100

struct btree_test_node {
    struct list_head list;
    union {
        uintptr_t key;
        char uuid[11];
    };
};

static int btree_test_clash(struct btree_root *root, void *value, void *clash)
{
    struct btree_test_node *vnode = value;
    struct btree_test_node *cnode = clash;
    list_add(&vnode->list, &cnode->list);
    return 0;
}

static void *btree_test_remove(struct btree_root *root, void *value)
{
    struct btree_test_node *vnode = value;
    struct btree_test_node *remove;

    if (list_check_empty(&vnode->list))
        return NULL;

    remove = list_first_entry(&vnode->list, struct btree_test_node, list);
    list_del(&remove->list);

    return remove;
}

static long btree_test_strfind(struct btree_root *root, uintptr_t *node, uintptr_t *key)
{
    const char *nstring = (void *)*node ?: "";
    const char *kstring = (void *)*key ?: "";
    return strcmp(nstring, kstring);
}

static int btree_test_testing(struct btree_test_node *nodes)
{
    struct btree_test_node *lookup;
    unsigned int count;
    uintptr_t insert;
    void *value;
    int retval;

    BTREE_ROOT(root32, &btree_layout32,
        btree_alloc, btree_free, btree_keyfind,
        btree_test_clash, btree_test_remove, NULL
    );

    for (count = 0; count < TEST_LOOP; ++count) {
        list_head_init(&nodes[count].list);
        nodes[count].key = (uintptr_t)rand();
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        retval = btree_insert(&root32, &nodes[count].key, &nodes[count]);
        printf("btree random insert test%d: %#010lx ret %d\n", count,
                nodes[count].key, retval);
        if (retval)
            return retval;
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        lookup = btree_lookup(&root32, &nodes[count].key);
        printf("btree random lookup test%d: ", count);
        if (!lookup || (lookup != &nodes[count] && list_check_empty(&lookup->list))) {
            printf("failed\n");
            return -ENOENT;
        }
        printf("pass\n");
    }

    btree_for_each(&root32, &insert, value) {
        printf("btree random for each: %#010lx = %p\n",
                insert, value);
    }

    btree_for_each_reverse(&root32, &insert, value) {
        printf("btree random for each reverse: %#010lx = %p\n",
                insert, value);
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        lookup = btree_remove(&root32, &nodes[count].key);
        printf("btree random remove test%d\n", count);
    }

    btree_destroy(&root32);

    BTREE_ROOT(rootstr, &btree_layout32,
        btree_alloc, btree_free, btree_test_strfind,
        btree_test_clash, btree_test_remove, NULL
    );

    for (count = 0; count < TEST_LOOP; ++count) {
        list_head_init(&nodes[count].list);
        sprintf(nodes[count].uuid, "%#010x", rand());
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        insert = (uintptr_t)&nodes[count].uuid;
        retval = btree_insert(&rootstr, &insert, &nodes[count]);
        printf("btree string insert test%d: %s ret %d\n", count,
                nodes[count].uuid, retval);
        if (retval)
            return retval;
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        insert = (uintptr_t)&nodes[count].uuid;
        lookup = btree_lookup(&rootstr, &insert);
        printf("btree string lookup test%d: ", count);
        if (!lookup || (lookup != &nodes[count] && list_check_empty(&lookup->list))) {
            printf("failed\n");
            return -ENOENT;
        }
        printf("pass\n");
    }

    btree_for_each(&rootstr, &insert, value) {
        printf("btree string for each: %s = %p\n",
                (char *)insert, value);
    }

    btree_for_each_reverse(&rootstr, &insert, value) {
        printf("btree string for each reverse: %s = %p\n",
                (char *)insert, value);
    }

    for (count = 0; count < TEST_LOOP; ++count) {
        insert = (uintptr_t)&nodes[count].uuid;
        lookup = btree_remove(&rootstr, &insert);
        printf("btree string remove test%d\n", count);
    }

    btree_destroy(&rootstr);

    return 0;
}

int main(void)
{
    struct btree_test_node *bdata;
    int retval;

    bdata = malloc(sizeof(*bdata) * TEST_LOOP);
    if (!bdata)
        return -1;

    retval = btree_test_testing(bdata);
    free(bdata);

    return retval;
}
