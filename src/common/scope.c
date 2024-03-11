#include "scope.h"
#include <stdlib.h>

scope *scope_new() {
    scope  *s = calloc(1, sizeof(scope));
    hashmap elements = hashmap_new_default(&get_element_name_mapping_name,
                                           &get_element_name_mapping_element,
                                           &update_element_name_mapping_element,
                                           &str_hash_func,
                                           &str_eq_func,
                                           &element_name_mapping_element_eq_f);
    s->elements = elements;
    s->parent = s;
    s->collecting = true;
    return s;
}

void scope_free(scope *s) {
    if (s->name != NULL) free(s->name);
    if (s->elements != NULL) hashmap_free(s->elements);
    free(s);
    s = NULL;
}