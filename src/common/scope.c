#include "scope.h"
#include "c_hashmap.h"
#include "global.h"

extern bool debug;

void free_scope(struct Scope *s) {
    if (!s) return;
    if (s->name) free(s->name);
    if (s->first_symbol) free_symbol(s->first_symbol);
    if (s->next) free_scope(s->next);
    free(s);
    s = NULL;
}

void free_symbol(struct Symbol *s) {
    if (!s) return;
    if (s->type) free(s->type);
    if (s->name) free(s->name);
    if (s->pos) free(s->pos);
    if (s->next) free_symbol(s->next);
    free(s);
    s = NULL;
}

struct Scope *create_scope(struct Scope *parent, char *name) {
    if (!name) {
        fprintf(stderr, "create_scope(), invalid null argument: name\n");
        exit(EXIT_FAILURE);
    }
    
    printf("create scope: %s\n", name);

    struct Scope *s = CREATE_STRUCT_P(Scope);
    if (!s) goto error;
    s->name = calloc(strlen(name) + 1, sizeof(char));
    if (!s->name) goto error;
    strcpy(s->name, name);

    if (!parent) s->parent = s;
    else {
        s->parent = parent;
        if (!parent->first_child_scope) parent->first_child_scope = s;
        else parent->last_child_scope->next = s;
        parent->last_child_scope = s;
    }

    return s;

error:
    fprintf(stderr, "create_scope(), no enough memory\n");
    free_scope(s);
    exit(EXIT_FAILURE);
    return NULL;
}

struct Symbol *create_symbol(struct Type *type, char *name, struct Scope *scope, struct Position *pos) {
    if (!type || !name || !scope || !pos) {
        fprintf(stderr, "create_symbol(), invalid null argument\n");
        exit(EXIT_FAILURE);
    }
    struct Symbol *s = CREATE_STRUCT_P(Symbol);
    if (!s) goto error;
    s->scope = scope;
    scope_add_symbol(scope, s);
    s->pos = pos;
    s->type = type;
    s->name = calloc(strlen(name) + 1, sizeof(char));
    if (!s->name) goto error;
    strcpy(s->name, name);
    return s;
error:
    fprintf(stderr, "create_symbol(), no enough memory\n");
    free_symbol(s);
    exit(EXIT_FAILURE);
    return NULL;
}

struct Symbol *scope_lookup_symbol(struct Scope *s, char *name) {
    if (!s || !name) return NULL;

    struct Symbol *symbol = s->first_symbol;
    while (symbol) {
        if (str_eq_func(symbol->name, name)) return symbol;
        symbol = symbol->next;
    }

    return NULL;
}

struct Symbol *scope_lookup_symbol_from_all(struct Scope *s, char *name) {
    if (!s || !name) return NULL;

    struct Symbol *symbol = s->first_symbol;
    while (symbol) {
        if (str_eq_func(symbol->name, name)) return symbol;
        symbol = symbol->next;
    }

    if (s->parent != s) return scope_lookup_symbol_from_all(s->parent, name);
    return NULL;
}

void scope_add_symbol(struct Scope *s, struct Symbol *symbol) {
    if (!s || !symbol) return;
    struct Symbol *sym = scope_lookup_symbol(s, symbol->name);
    if (sym) {
        fprintf(stderr, "symbol %s have already defined\n", symbol->name);
        return;
    }

    if (!s->first_symbol) s->first_symbol = symbol;
    else s->last_symbol->next = symbol;
    s->last_symbol = symbol;
}

struct Scope *enter_scope(struct Scope *s, char *name) {
    if (!s || !name) return NULL;

    struct Scope *scope = s->first_child_scope;
    while (scope) {
        if (str_eq_func(scope->name, name)) return scope;
        scope = scope->next;
    }

    return s;
}

struct Scope *exit_scope(struct Scope *s) {
    if (!s || s->parent == s) return s;
    return s->parent;
}