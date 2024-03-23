#ifndef SCOPE_H
#define SCOPE_H

#include "c_hashmap.h"
#include "position.h"
#include <stdbool.h>

struct Scope {
    struct Scope *parent;
    struct Scope *children;
    hashmap       name_element_map;

    struct Position start, end;
    char           *name;
    bool            isFunc;
    bool            collecting;
};

struct Element {
    struct Scope *root_scope;
    struct Scope *cur_scope;

    struct Position pos;
    char           *name;
};

struct _element_name_mapping {
    char           *name;
    struct Element *element;
};

static void *get_element_name_mapping_name(void *ele) {
    return ((struct _element_name_mapping *)ele)->name;
}

static void *get_element_name_mapping_element(void *ele) {
    return ((struct _element_name_mapping *)ele)->element;
}

static void update_element_name_mapping_element(void *ele1, void *ele2) {
    ((struct _element_name_mapping *)ele1)->element = ((struct _element_name_mapping *)ele2)->element;
}

static bool element_name_mapping_element_eq_f(void *v1, void *v2) {
    return str_eq_func(((struct Element *)v1)->name, ((struct Element *)v2)->name);
}

struct Scope *scope_new();
void          scope_free(struct Scope *s);

struct Element *scope_lookup(char *name);
struct Element *scope_lookup_all(char *name);
void            scope_addEle(char *name, struct Element *e);

struct Scope *scope_enter(char *name);
struct Scope *scope_exit();

#endif