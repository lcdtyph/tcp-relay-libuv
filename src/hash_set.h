#ifndef __TCP_RELAY_HASH_SET_H__
#define __TCP_RELAY_HASH_SET_H__

typedef struct hash_set hash_set;

typedef void (*hash_set_walk_cb)(void *key, void *arg);

hash_set *hash_set_new();
void hash_set_drop(hash_set *h);
void hash_set_put(hash_set *h, void *key);
void *hash_set_get(hash_set *h, void *key);
size_t hash_set_size(const hash_set *h);
void hash_set_delete(hash_set *h, void *key);
void hash_set_walk(hash_set *h, hash_set_walk_cb cb, void *arg);

#endif // __TCP_RELAY_HASH_SET_H__
