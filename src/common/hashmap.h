#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef unsigned int uint;

typedef void (*free_func)(const void *o);

typedef struct _hash_map_entry {
    int         hash;
    const void *key;
    void       *val;

    struct _hash_map_entry *next;

    free_func k_free_f;
    free_func v_free_f;
} *hash_map_entry;

typedef int (*hash_func)(const void *k);
typedef bool (*eq_func)(const void *k1, const void *k2);

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
hashmap hashmap_init_f(
    int init_size, float expand_factor, float shrink_factor, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);
hashmap hashmap_init(int init_size, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f);

void hashmap_set_k_free_func(const hashmap map, free_func k_free_f);
void hashmap_set_v_free_func(const hashmap map, free_func v_free_f);

#define SHRINK_MOD 0
#define EXPAND_MOD 1

bool hashmap_contains_key(const hashmap map, const void *k);
bool hashmap_contains_value(const hashmap map, void *v);

typedef void *(*val_produce_func)(const void *k);
void *hashmap_get(const hashmap map, const void *k);
void *hashmap_get_or_default(const hashmap map, const void *k, void *def_val);
void *hashmap_get_or_default_f(const hashmap map, const void *k, val_produce_func val_produce_f);
void *hashmap_put_f(const hashmap map, const void *k, void *v, free_func k_free_f, free_func v_free_func);
void *hashmap_put(const hashmap map, const void *k, void *v);
void *hashmap_put_if_absent(const hashmap map, const void *k, void *def_val);
void *hashmap_put_if_absent_f(const hashmap map, const void *k, val_produce_func val_produce_f);
bool  hashmap_set_entry_free_func(const hashmap map, const void *k, free_func k_free_f, free_func v_free_f);
void *hashmap_remove(const hashmap map, const void *k);
void  hashmap_clear(const hashmap map);
void  hashmap_free(hashmap map);

typedef bool (*filter_func)(const void *k, void *v);
// return true means stop.
typedef bool (*foreach_func)(const void *k, void *v);

typedef struct _hashmap_iterator {
    foreach_func foreach_f;
    filter_func  filter_f;
} *hashmap_itr;

hashmap_itr hashmap_itr_init(foreach_func foreach);
void        hashmap_itr_free(hashmap_itr itr);
void        hashmap_itr_set_filter_f(const hashmap_itr itr, filter_func filter_f);
void        hashmap_itr_set_foreach_f(const hashmap_itr itr, foreach_func foreach_f);

void hashmap_foreach(const hashmap map, const hashmap_itr itr);

/* 
 * util functions
 */

static bool is_power_of_2(long s) {
    return !(s & (s - 1));
}

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

static int int_hash(int i) {
    return (i == 0) ? 0 : i ^ (i >> 16);
}

static int hash(hash_func hash_f, const void *k) {
    return int_hash(hash_f(k));
}

static int str_hash_func(const void *k) {
    int         h = 0;
    const char *c = k;
    while (*c != '\0') {
        h += h * 7 + *c;
        c++;
    }
    return h;
}

static int ptr_hash_func(const void *k) {
    return *((long *)k);
}

static bool ptr_eq_func(const void *k1, const void *k2) {
    return k1 == k2;
}

static bool int_eq_func(const void *k1, const void *k2) {
    return *(int *)k1 == *(int *)k2;
}

static bool str_eq_func(const void *k1, const void *k2) {
    const char *c1 = (char *)k1;
    const char *c2 = (char *)k2;
    while (*c1 != '\0' && *c2 == '\0' && *c1 == *c2) {
        c1++;
        c2++;
    }
    return *c1 == *c2;
}

#endif