#ifndef SCOPE_H
#define SCOPE_H

#include "position.h"
#include <stdbool.h>

struct Scope {
    char          *name;         // name of scope
    bool           is_func;      // is scope a func scope
    struct Symbol *first_symbol; // first symbol in symbol list
    struct Symbol *last_symbol;  // last symbol in symbol list

    struct Scope *parent;            // parent scope
    struct Scope *first_child_scope; // first scope in children scope list
    struct Scope *last_child_scope;  // last scope in children scope list
    struct Scope *next;              // next scope in the same scope level
};

struct Symbol {
    struct Type *type; // type of symbol
    char        *name; // name of symbol

    struct Scope    *scope; // which scope the symbol exists
    struct Position *pos;   // position of the symbol
    struct Symbol   *next;  // next symbol in the same scope level
};

void           free_scope(struct Scope *s);
void           free_symbol(struct Symbol *s);
struct Scope  *create_scope(struct Scope *parent, char *name);
struct Symbol *create_symbol(struct Type *type, char *name, struct Scope *scope, struct Position *pos);

struct Symbol *scope_lookup_symbol(struct Scope *s, char *name);
struct Symbol *scope_lookup_symbol_from_all(struct Scope *s, char *name);
void           scope_add_symbol(struct Scope *s, struct Symbol *symbol);

struct Scope *enter_scope(struct Scope *s, char *name);
struct Scope *exit_scope(struct Scope *s);

#endif