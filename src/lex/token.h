#ifndef TOKEN_H
#define TOKEN_H

#include "c_hashmap.h"
#include <stdlib.h>
#include <stdio.h>

typedef enum {
    // reserved words
    // type
    _int = 1,
    _float,
    _bool,
    _char,
    _string,
    _func,
    _void,
    // value
    _true,
    _false,
    _null,
    // control
    _for,
    _while,
    _if,
    _else,
    _elseif,
    _switch,
    _case,
    _default,
    _continue,
    _break,
    _return,

    // lit
    _ident,
    _lit,

    // operator
    // rel
    _eq,
    _ne,
    _lt,
    _le,
    _gt,
    _ge,
    // arith
    _add,
    _sub,
    _mul,
    _quo,
    _rem,
    _and,
    _or,
    _xor,
    _not,
    _shl,
    _shr,
    // assign
    _assign,
    _inc,
    _dec,
    // logic
    _land,
    _lor,
    _lnot,

    // delimeter
    _lparen,
    _rparen,
    _lbracket,
    _rbracket,
    _lbrace,
    _rbrace,
    _rarrow,
    _comma,
    _period,
    _semi,
    _colon,
    _ques,
    _comment,

    // others
    _eof,
    _not_exist,
    _illegal,
} token;

typedef struct _tk_mapping {
    char *name;
    token token;
} tk_mapping;

static tk_mapping *tk_mapping_new(char *name, token token) {
    tk_mapping *t = (tk_mapping *)calloc(1, sizeof(tk_mapping));
    t->name = name;
    t->token = token;
    return t;
}

static void *get_tk_mapping_name(void *ele) {
    return ((tk_mapping *)ele)->name;
}

static void *get_tk_mapping_token(void *ele) {
    return &((tk_mapping *)ele)->token;
}

static void update_tk_mapping_token(void *ele1, void *ele2) {
    ((tk_mapping *)ele1)->token = ((tk_mapping *)ele2)->token;
}

static hashmap reserved_tk_map = NULL;

// clang-format off
#define TK_MAPPING(name_str) (tk_mapping){(name_str)}
#define ADD_TK_MAPPING(T) hashmap_put(reserved_tk_map, tk_mapping_new((#T), (_##T)))
#define GET_TK_MAPPING(name_str) (token *)hashmap_get(reserved_tk_map, &TK_MAPPING(name_str))
// clang-format on

static void reserved_tk_map_init() {
    reserved_tk_map = hashmap_new(_return << 1,
                                  &get_tk_mapping_name,
                                  &get_tk_mapping_token,
                                  &update_tk_mapping_token,
                                  &str_hash_func,
                                  &str_eq_func,
                                  &int_eq_func);

    ADD_TK_MAPPING(int);
    ADD_TK_MAPPING(float);
    ADD_TK_MAPPING(bool);
    ADD_TK_MAPPING(char);
    ADD_TK_MAPPING(string);
    ADD_TK_MAPPING(func);
    ADD_TK_MAPPING(void);
    ADD_TK_MAPPING(true);
    ADD_TK_MAPPING(false);
    ADD_TK_MAPPING(null);
    ADD_TK_MAPPING(for);
    ADD_TK_MAPPING(while);
    ADD_TK_MAPPING(if);
    ADD_TK_MAPPING(else);
    ADD_TK_MAPPING(elseif);
    ADD_TK_MAPPING(switch);
    ADD_TK_MAPPING(case);
    ADD_TK_MAPPING(default);
    ADD_TK_MAPPING(continue);
    ADD_TK_MAPPING(break);
    ADD_TK_MAPPING(return);
};

static token lookup_reserved_tk(char *s) {
    if (reserved_tk_map == NULL) reserved_tk_map_init();
    if (reserved_tk_map == NULL) printf("Lexer: could not init reserved token map");
    token *t = GET_TK_MAPPING(s);
    return t == NULL ? _not_exist : *t;
}

typedef enum { int_lit = 1, float_lit, bool_lit, char_lit, string_lit, null_lit } lit_kind;

#endif