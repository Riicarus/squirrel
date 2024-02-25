#ifndef TOKEN_H
#define TOKEN_H

#include "c_hashmap.h"
#include <stdlib.h>

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
    // functionality
    _type,
    _struct,
    _new,
    _sizeof,
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
    // scope
    _pkg,
    _import,
    _const,

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

typedef struct _tk {
    char *name;
    token token;
} tk;

static tk *tk_new(char *name, token token) {
    tk *t = (tk *)calloc(1, sizeof(tk));
    t->name = name;
    t->token = token;
    return t;
}

static void *get_tk_name(void *ele) {
    return ((tk *)ele)->name;
}

static void *get_tk_token(void *ele) {
    return &((tk *)ele)->token;
}

static void tk_token_update(void *ele1, void *ele2) {
    ((tk *)ele1)->token = ((tk *)ele2)->token;
}

static hashmap reserved_tk_map = NULL;

// clang-format off
#define TK(name_str) (tk){(name_str)}
#define ADD_TK(T) hashmap_put(reserved_tk_map, tk_new((#T), (_##T)))
#define GET_TK(name_str) (token *)hashmap_get(reserved_tk_map, &TK(name_str))
// clang-format on

static void reserved_tk_map_init() {
    reserved_tk_map = hashmap_new(
        _const << 1, &get_tk_name, &get_tk_token, &tk_token_update, &str_hash_func, &str_eq_func, &int_eq_func);

    ADD_TK(int);
    ADD_TK(float);
    ADD_TK(bool);
    ADD_TK(char);
    ADD_TK(string);
    ADD_TK(func);
    ADD_TK(void);
    ADD_TK(type);
    ADD_TK(struct);
    ADD_TK(new);
    ADD_TK(sizeof);
    ADD_TK(true);
    ADD_TK(false);
    ADD_TK(null);
    ADD_TK(for);
    ADD_TK(while);
    ADD_TK(if);
    ADD_TK(else);
    ADD_TK(elseif);
    ADD_TK(switch);
    ADD_TK(case);
    ADD_TK(default);
    ADD_TK(continue);
    ADD_TK(break);
    ADD_TK(return);
    ADD_TK(pkg);
    ADD_TK(import);
    ADD_TK(const);
};

static token lookup_reserved_tk(char *s) {
    if (reserved_tk_map == NULL) return _not_exist;
    token *t = GET_TK(s);
    return t == NULL ? _not_exist : *t;
}

typedef enum { int_lit, float_lit, bool_lit, char_lit, string_lit, null_lit } lit_kind;

#endif