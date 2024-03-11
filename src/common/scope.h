#ifndef SCOPE_H
#define SCOPE_H

#include "c_hashmap.h"
#include "position.h"
#include <stdbool.h>

typedef struct _scope {
    struct _scope *parent;
    struct _scope *children;
    hashmap        name_element_map;

    position start, end;
    char    *name;
    bool     isFunc;
    bool     collecting;
} scope;

typedef struct _element {
    scope *root_scope;
    scope *cur_scope;

    position pos;
    char    *name;
} element;

struct _element_name_mapping {
    char    *name;
    element *element;
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
    return str_eq_func(((element *)v1)->name, ((element *)v2)->name);
}

scope *scope_new();
void   scope_free(scope *s);

element *scope_lookup(char *name);
element *scope_lookup_all(char *name);
void     scope_addEle(char *name, element *e);

scope *scope_enter(char *name);
scope *scope_exit();

#endif