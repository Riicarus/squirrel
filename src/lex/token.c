#include "token.h"

struct TokenSymbol tk_symbols[] = {"int",
                                   "float",
                                   "bool",
                                   "char",
                                   "string",
                                   "void",
                                   "func",
                                   // value
                                   "true",
                                   "false",
                                   // control
                                   "for",
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
                                   // binary
                                   "==",
                                   "!=",
                                   "<",
                                   "<=",
                                   ">",
                                   ">=",
                                   "+",
                                   "-",
                                   "*",
                                   "/",
                                   "%%",
                                   "&",
                                   "|",
                                   "^",
                                   "<<",
                                   ">>",
                                   "&&",
                                   "||",
                                   // unary
                                   "~",
                                   "!",
                                   // assign
                                   "=",
                                   "++",
                                   "--",

                                   // delimeter
                                   "(",
                                   ")",
                                   "[",
                                   "]",
                                   "{",
                                   "}",
                                   "->",
                                   ",",
                                   ".",
                                   ";",
                                   ":",
                                   "?",
                                   "//",

                                   // others
                                   "eof",
                                   "not_exist",
                                   "illegal"};

struct LitKindSymbol lit_kind_symbols[] = {
    "int",
    "float",
    "bool",
    "char",
    "string",
};

enum Token basic_type_tokens[BASIC_TYPE_TOKEN_NUMBER] = {_int, _float, _bool, _char, _string, _void};

enum Token unary_op_tokens[UNARY_OP_TOKEN_NUMBER] = {_not, _lnot};

enum Token binary_op_tokens[BINARY_OP_TOKEN_NUMBER] = {
    _eq,
    _ne,
    _lt,
    _le,
    _gt,
    _ge,
    _add,
    _sub,
    _mul,
    _quo,
    _rem,
    _and,
    _or,
    _xor,
    _shl,
    _shr,
    _land,
    _lor,
    _assign
};

enum Token ctrl_start_tokens[CTRL_START_TOKEN_NUMBER] = {_break, _continue, _return, _if, _for};

enum Token expr_start_tokens[EXPR_START_TOKEN_NUMBER] = {_lparen, _ident, _inc, _dec, _not, _lnot};

enum Token basic_lit_tokens[BASIC_LIT_TOKEN_NUMBER] = {_lit, _true, _false};

hashmap reserved_tk_map = NULL;