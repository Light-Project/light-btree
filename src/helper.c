/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2022 Sanpe <sanpeqf@gmail.com>
 */

#include "btree.h"
#include <stdlib.h>

#define BLOCK_SIZE 128
#define NODE_SIZE (BLOCK_SIZE - sizeof(struct btree_node))
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define UINTPTR_PER_U32 DIV_ROUND_UP(sizeof(uint32_t), sizeof(uintptr_t))
#define UINTPTR_PER_U64 DIV_ROUND_UP(sizeof(uint64_t), sizeof(uintptr_t))

struct btree_layout btree_layout32 = {
    .keylen = UINTPTR_PER_U32,
    .keynum = NODE_SIZE / sizeof(uintptr_t) / (UINTPTR_PER_U32 + 1),
    .ptrindex = UINTPTR_PER_U32 * (NODE_SIZE / sizeof(uintptr_t) / (UINTPTR_PER_U32 + 1)),
    .nodesize = NODE_SIZE,
};

struct btree_layout btree_layout64 = {
    .keylen = UINTPTR_PER_U64,
    .keynum = NODE_SIZE / sizeof(uintptr_t) / (UINTPTR_PER_U64 + 1),
    .ptrindex = UINTPTR_PER_U64 * (NODE_SIZE / sizeof(uintptr_t) / (UINTPTR_PER_U64 + 1)),
    .nodesize = NODE_SIZE,
};

struct btree_layout btree_layoutptr = {
    .keylen = 1,
    .keynum = NODE_SIZE / sizeof(uintptr_t) / 2,
    .ptrindex = NODE_SIZE / sizeof(uintptr_t) / 2,
    .nodesize = NODE_SIZE,
};

long btree_keyfind(struct btree_root *root, uintptr_t *node, uintptr_t *key)
{
    struct btree_layout *layout = root->layout;
    unsigned int index;

    for (index = 0; index < layout->keylen; ++index) {
        if (node[index] < key[index])
            return -1;
        if (node[index] > key[index])
            return 1;
    }

    return 0;
}

void *btree_alloc(struct btree_root *root)
{
    struct btree_layout *layout = root->layout;
    return malloc(layout->nodesize);
}

void btree_free(struct btree_root *root, void *node)
{
    free(node);
}
