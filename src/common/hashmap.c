#include "hashmap.h"
#include "limits.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/*
 * capacity of hashmap must be power of 2.
 */
hashmap hashmap_init_f(int init_size, float expand_factor, float shrink_factor, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f) {
    hashmap map = (hashmap)calloc(1, sizeof(struct _hashmap));
    if (map == NULL) goto error;
    map->size = 0;

    if (init_size <= 0) init_size = DEFAULT_INIT_SIZE;

    if (init_size >= INT_MAX >> 1) // avoid overflow
        map->cap = INT_MAX;
    else
        map->cap = IS_POWER_OF_2(init_size) ? round_up_power_of_2(init_size) << 1 : round_up_power_of_2(init_size);

    map->expand_factor = expand_factor < 0 ? DEFAULT_EXPAND_FACTOR : expand_factor;
    map->shrink_factor = shrink_factor < 0 ? DEFAULT_SHRINK_FACTOR : shrink_factor;

    map->bucket = (hash_map_entry *)calloc(map->cap, sizeof(hash_map_entry));
    if (map->bucket == NULL) goto error;

    map->hash_f = hash_f == NULL ? &ptr_hash_func : hash_f;
    map->k_eq_f = k_eq_f == NULL ? &ptr_eq_func : k_eq_f;
    map->v_eq_f = v_eq_f == NULL ? &ptr_eq_func : v_eq_f;
    return map;

error:
    perror("no enough memory");
    return NULL;
}

hashmap hashmap_init(int init_size, hash_func hash_f, eq_func k_eq_f, eq_func v_eq_f) {
    return hashmap_init_f(init_size, DEFAULT_EXPAND_FACTOR, DEFAULT_SHRINK_FACTOR, hash_f, k_eq_f, v_eq_f);
}

void hashmap_set_k_free_func(const hashmap map, free_func k_free_f) {
    map->k_free_f = k_free_f;
}

void hashmap_set_v_free_func(const hashmap map, free_func v_free_f) {
    map->v_free_f = v_free_f;
}

int _hashmap_cul_index(uint cap, int h) {
    return h & (cap - 1);
}

void _hashmap_rehash(const hashmap map, hash_map_entry *new_bucket, uint new_cap) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e, ne;

    for (int i = 0; i < map->cap; i++, b++) {
        // if this bucket is empty, continue to next;
        if ((e = *b) == NULL) continue;

        // iterate entries and move them to new bucket
        while (e != NULL) {
            int new_idx = _hashmap_cul_index(new_cap, e->hash);
            ne = e->next;
            e->next = NULL;

            // head-insert to new bucket
            hash_map_entry h = new_bucket[new_idx];
            new_bucket[new_idx] = e;
            if (h != NULL) e->next = h;
            e = ne;
        }
    }

    free(map->bucket);
    map->cap = new_cap;
    map->bucket = new_bucket;
}

/*
 * inc_size == 0 means shrink.
 */
_Bool _hashmap_ensure_cap(const hashmap map, int inc_size) {
    if (map->cap >= INT_MAX) {
        perror("reach the max capacity of hash map");
        return false;
    }

    int mod;
    if (!inc_size && map->size <= map->shrink_factor * map->cap)
        mod = SHRINK_MOD;
    else if (inc_size > 0 && map->size + inc_size >= map->expand_factor * map->cap)
        mod = EXPAND_MOD;
    else
        return true;

    uint new_cap = mod ? map->cap << 1 : map->cap >> 1;
    // TODO: optimize, use realloc for expand, use calloc for shrink, thus the rehash method should also be updated to 2 methods.
    hash_map_entry *new_bucket = (hash_map_entry *)calloc(new_cap, sizeof(hash_map_entry));
    memset(new_bucket, 0, new_cap * sizeof(hash_map_entry));
    if (new_bucket == NULL) {
        perror("no enough memory");
        return false;
    }
    _hashmap_rehash(map, new_bucket, new_cap);
    return true;
}

_Bool hashmap_contains_key(const hashmap map, void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) return true;
        e = e->next;
    }
    return false;
}

_Bool hashmap_contains_value(const hashmap map, void *v) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;
        while (e != NULL) {
            if (map->v_eq_f(e->val, v)) return true;
            e = e->next;
        }
    }
    return false;
}

void *hashmap_put_f(const hashmap map, void *k, void *v, free_func k_free_f, free_func v_free_func) {
    hash_map_entry e = (hash_map_entry)malloc(sizeof(struct _hash_map_entry));
    if (e == NULL) {
        perror("no enough memory");
        return v;
    }
    e->hash = hash(map->hash_f, k);
    e->key = k;
    e->val = v;
    e->next = NULL;
    e->k_free_f = k_free_f;
    e->v_free_f = v_free_func;

    if (!_hashmap_ensure_cap(map, 1)) return v;

    int idx = _hashmap_cul_index(map->cap, e->hash);
    if (map->bucket[idx] == NULL) {
        map->bucket[idx] = e;
        map->size += 1;
        return v;
    }

    // find if key exists
    hash_map_entry h = map->bucket[idx];
    while (h != NULL) {
        if (map->k_eq_f(h->key, k)) {
            h->val = v;
            return v;
        }
        h = h->next;
    }
    // key not exists, use head-insert
    e->next = map->bucket[idx];
    map->bucket[idx] = e;
    map->size += 1;

    return v;
}

void *hashmap_put(const hashmap map, void *k, void *v) {
    return hashmap_put_f(map, k, v, NULL, NULL);
}

void *hashmap_get(const hashmap map, void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    while (e != NULL) {
        if (map->k_eq_f(e->key, k))
            return e->val;
        e = e->next;
    }
    return NULL;
}

void _free_entry(const hashmap map, hash_map_entry e) {
    free_func k_free_f = e->k_free_f == NULL ? map->k_free_f : e->k_free_f;
    free_func v_free_f = e->v_free_f == NULL ? map->v_free_f : e->v_free_f;
    if (k_free_f != NULL) k_free_f(e->key);
    if (v_free_f != NULL) v_free_f(e->val);
    free(e);
    e = NULL;
}

/*
 * returned value may be invalid caused by v_free_func.
 */
void *hashmap_remove(const hashmap map, void *k) {
    int h = hash(map->hash_f, k);
    int idx = _hashmap_cul_index(map->cap, h);

    hash_map_entry e = map->bucket[idx];
    hash_map_entry pe = NULL;
    while (e != NULL) {
        if (map->k_eq_f(e->key, k)) {
            if (pe == NULL)
                map->bucket[idx] = e->next;
            else
                pe->next = e->next;

            void *val = e->val;
            _free_entry(map, e);
            map->size -= 1;

            _hashmap_ensure_cap(map, 0);
            return val;
        }
        pe = e;
        e = e->next;
    }

    return NULL;
}

void hashmap_clear(const hashmap map) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e, ne;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL) {
            ne = e->next;
            _free_entry(map, e);
            e = ne;
        }
        *b = NULL;
    }

    map->size = 0;
}

void hashmap_free(hashmap map) {
    if (map == NULL) return;

    if (map->bucket != NULL) {
        hashmap_clear(map);
        free(map->bucket);
        map->bucket = NULL;
    }
    free(map);
    map = NULL;
}

hashmap_itr hashmap_itr_init(foreach_func foreach_f) {
    hashmap_itr itr = (hashmap_itr)calloc(1, sizeof(struct _hashmap_iterator));
    if (itr == NULL) {
        perror("no enough memory");
        return NULL;
    }

    itr->foreach_f = foreach_f;
    return itr;
}

void hashmap_itr_free(hashmap_itr itr) {
    free(itr);
    itr = NULL;
}

void hashmap_foreach(const hashmap map, const hashmap_itr itr) {
    hash_map_entry *b = map->bucket;
    hash_map_entry  e;
    for (int i = 0; i < map->cap; i++, b++) {
        if ((e = *b) == NULL) continue;

        while (e != NULL)
            if (itr->filter_f == NULL || itr->filter_f(e->key, e->val)) {
                itr->foreach_f(e->key, e->val);
                e = e->next;
            } else
                e = e->next;
    }
}