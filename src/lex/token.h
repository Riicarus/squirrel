#ifndef TOKEN_H
#define TOKEN_H

#include "c_hashmap.h"
#include <stdlib.h>
#include <stdio.h>

enum Token {
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
    // array
    _at,

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
};

struct {
    char *symbol;
} tk_symbols[_illegal + 1] = {"",
                              "int",
                              "float",
                              "bool",
                              "char",
                              "string",
                              "func",
                              "void",
                              // value
                              "true",
                              "false",
                              "null",
                              // control
                              "for",
                              "while",
                              "if",
                              "else",
                              "elseif",
                              "continue",
                              "break",
                              "return",

                              // lit
                              "ident",
                              "lit",

                              // operator
                              // rel
                              "eq",
                              "ne",
                              "lt",
                              "le",
                              "gt",
                              "ge",
                              // arith
                              "add",
                              "sub",
                              "mul",
                              "quo",
                              "rem",
                              "and",
                              "or",
                              "xor",
                              "not",
                              "shl",
                              "shr",
                              // assign
                              "assign",
                              "inc",
                              "dec",
                              // logic
                              "land",
                              "lor",
                              "lnot",
                              // array
                              "at",

                              // delimeter
                              "lparen",
                              "rparen",
                              "lbracket",
                              "rbracket",
                              "lbrace",
                              "rbrace",
                              "rarrow",
                              "comma",
                              "period",
                              "semi",
                              "colon",
                              "ques",
                              "comment",

                              // others
                              "eof",
                              "not_exist",
                              "illegal"};

struct TokenMapping {
    char      *name;
    enum Token token;
};

static struct TokenMapping *tk_mapping_new(char *name, enum Token token) {
    struct TokenMapping *t = (struct TokenMapping *)calloc(1, sizeof(struct TokenMapping));
    t->name = name;
    t->token = token;
    return t;
}

static void *get_tk_mapping_name(void *ele) {
    return ((struct TokenMapping *)ele)->name;
}

static void *get_tk_mapping_token(void *ele) {
    return &((struct TokenMapping *)ele)->token;
}

static void update_tk_mapping_token(void *ele1, void *ele2) {
    ((struct TokenMapping *)ele1)->token = ((struct TokenMapping *)ele2)->token;
}

static hashmap reserved_tk_map = NULL;

// clang-format off
#define TK_MAPPING(name_str) (struct TokenMapping){(name_str)}
#define ADD_TK_MAPPING(T) hashmap_put(reserved_tk_map, tk_mapping_new((#T), (_##T)))
#define GET_TK_MAPPING(name_str) (enum Token *)hashmap_get(reserved_tk_map, &TK_MAPPING(name_str))
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
    ADD_TK_MAPPING(continue);
    ADD_TK_MAPPING(break);
    ADD_TK_MAPPING(return);
};

static enum Token lookup_reserved_tk(char *s) {
    if (reserved_tk_map == NULL) reserved_tk_map_init();
    if (reserved_tk_map == NULL) printf("Lexer: could not init reserved token map");
    enum Token *t = GET_TK_MAPPING(s);
    return t == NULL ? _not_exist : *t;
}

enum LitKind { int_lk, float_lk, bool_lk, char_lk, string_lk, null_lk };

#endif