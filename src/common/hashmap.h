#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>

static int int_hash(int i) {
    return (i == 0) ? 0 : i ^ (i >> 16);
}

static int hash(int (*hash_func)(void *), void *key) {
    return int_hash(hash_func(key));
}

static int str_hash_func(void *k) {
    int   h = 0;
    char *c = k;
    while (*c != '\0') {
        h += h * 7 + *c;
        c++;
    }
    return h;
}

static int ptr_hash_func(void *k) {
    return *((long *)k);
}

static int ptr_eq_func(void *k1, void *k2) {
    return k1 == k2;
}

static int int_eq_func(void *k1, void *k2) {
    return *(int *)k1 == *(int *)k2;
}

static int str_eq_func(void *k1, void *k2) {
    char *c1 = (char *)k1;
    char *c2 = (char *)k2;
    while (*c1 != '\0' && *c2 == '\0' && *c1 == *c2) {
        c1++;
        c2++;
    }

    return *c1 == *c2;
}

typedef unsigned int uint;

#define IS_POWER_OF_2(s) !((s) & ((s)-1))
static uint round_up_power_of_2(uint n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

typedef struct _hash_map_entry {
    int   hash;
    void *key;
    void *val;

    struct _hash_map_entry *next;
} *hash_map_entry;

typedef int (*hash_func)(void *);
typedef int (*eq_func)(void *, void *);

typedef struct _hashmap {
    uint            size;
    uint            cap;
    float           expand_factor;
    float           shrink_factor;
    hash_map_entry *bucket;

    hash_func hash_f;
    eq_func   eq_f;
} *hashmap;

#define DEFAULT_INIT_SIZE 8
#define DEFAULT_EXPAND_FACTOR 0.75
#define DEFAULT_SHRINK_FACTOR 0.20
hashmap hashmap_init(int init_size, hash_func hash_f, eq_func eq_f);
hashmap hashmap_init_f(int init_size, float expand_factor, float shrink_factor, hash_func hash_f, eq_func eq_f);

void  hashmap_free(hashmap map);
int   hashmap_cul_index(uint cap, int h);

#define SHRINK_MOD 0
#define EXPAND_MOD 1
void  hashmap_rehash(const hashmap map, hash_map_entry *new_bucket, uint new_cap);
int   hashmap_ensure_cap(const hashmap map, int inc_size);
void *hashmap_put(const hashmap map, void *k, void *v);
void *hashmap_get(const hashmap map, void *k);
void *hashmap_remove(const hashmap map, void *k);

typedef int (*filter_func)(const void *, void *);
typedef void (*foreach_func)(const void *, void *);

typedef struct _hashmap_iterator {
    foreach_func foreach_f;
    filter_func  filter_f;
} *hashmap_itr;

hashmap_itr hashmap_itr_init(foreach_func foreach);
void        hashmap_itr_free(hashmap_itr itr);

void hashmap_foreach(const hashmap map, const hashmap_itr itr);

#endif