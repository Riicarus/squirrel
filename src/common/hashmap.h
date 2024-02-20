#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>

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

static _Bool ptr_eq_func(void *k1, void *k2) {
    return k1 == k2;
}

static _Bool int_eq_func(void *k1, void *k2) {
    return *(int *)k1 == *(int *)k2;
}

static _Bool str_eq_func(void *k1, void *k2) {
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

typedef void (*free_func)(void *);

typedef struct _hash_map_entry {
    int   hash;
    void *key;
    void *val;

    struct _hash_map_entry *next;

    free_func k_free_f;
    free_func v_free_f;
} *hash_map_entry;

typedef int (*hash_func)(void *);
typedef _Bool (*eq_func)(void *, void *);

typedef struct _hashmap {
    uint            size;
    uint            cap;
    float           expand_factor;
    float           shrink_factor;
    hash_map_entry *bucket;

    hash_func hash_f;
    eq_func   k_eq_f;
    eq_func   v_eq_f;
    free_func k_free_f;
    free_func v_free_f;
} *hashmap;

#define DEFAULT_INIT_SIZE 8
#define DEFAULT_EXPAND_FACTOR 0.75
#define DEFAULT_SHRINK_FACTOR 0.20
hashmap hashmap_init_f(int init_size, float expand_factor, float shrink_factor, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);
hashmap hashmap_init(int init_size, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);

void hashmap_set_k_free_func(const hashmap map, free_func k_free_f);
void hashmap_set_v_free_func(const hashmap map, free_func v_free_f);

#define SHRINK_MOD 0
#define EXPAND_MOD 1

_Bool hashmap_contains_key(const hashmap map, void *k);
_Bool hashmap_contains_value(const hashmap map, void *v);
void *hashmap_put_f(const hashmap map, void *k, void *v, free_func k_free_f, free_func v_free_func);
void *hashmap_put(const hashmap map, void *k, void *v);
void *hashmap_get(const hashmap map, void *k);
void *hashmap_remove(const hashmap map, void *k);
void  hashmap_clear(const hashmap map);
void  hashmap_free(hashmap map);

typedef _Bool (*filter_func)(const void *, void *);
typedef void (*foreach_func)(const void *, void *);

typedef struct _hashmap_iterator {
    foreach_func foreach_f;
    filter_func  filter_f;
} *hashmap_itr;

hashmap_itr hashmap_itr_init(foreach_func foreach);
void        hashmap_itr_free(hashmap_itr itr);

void hashmap_foreach(const hashmap map, const hashmap_itr itr);

#endif