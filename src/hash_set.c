#include <uthash.h>

#include "hash_set.h"

typedef struct {
    UT_hash_handle hh;
    void *key;
} __nodes;

struct hash_set {
    __nodes *head;
};

hash_set *hash_set_new() {
    hash_set *h = (hash_set *)malloc(sizeof *h);
    memset(h, 0, sizeof *h);
    return h;
}

void hash_set_drop(hash_set *h) {
    __nodes *tmp, *item;
    HASH_ITER(hh, h->head, item, tmp) {
        HASH_DELETE(hh, h->head, item);
        free(item);
    }
}

void hash_set_put(hash_set *h, void *key) {
    __nodes *existing = NULL;
    __nodes *item = (__nodes *)malloc(sizeof *item);
    item->key = key;
    HASH_REPLACE_PTR(h->head, key, item, existing);
    if (existing) {
        free(existing);
    }
}

void *hash_set_get(hash_set *h, void *key) {
    __nodes *item = NULL;
    HASH_FIND_PTR(h->head, key, item);
    return item ? item->key : item;
}

size_t hash_set_size(const hash_set *h) {
    return HASH_COUNT(h->head);
}

void hash_set_delete(hash_set *h, void *key) {
    __nodes *item = NULL;
    HASH_FIND_PTR(h->head, key, item);
    if (item) {
        HASH_DELETE(hh, h->head, item);
        free(item);
    }
}

void hash_set_walk(hash_set *h, hash_set_walk_cb cb, void *arg) {
    __nodes *tmp, *item;
    HASH_ITER(hh, h->head, item, tmp) {
        cb(item->key, arg);
    }
}

