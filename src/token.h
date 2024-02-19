#ifndef TOKEN_H
#define TOKEN_H

#include "position.h"
#include <stdio.h>

typedef unsigned char token;

enum token {
    // reserved words
    // type
    INT,
    FLOAT,
    BOOL,
    CHAR,
    STRING,
    FUNC,
    VOID,
    // functionality
    TYPE,
    STRUCT,
    NEW,
    SIZEOF,
    // value
    TRUE,
    FALSE,
    NIL,
    // control
    FOR,
    WHILE,
    IF,
    ELSE,
    ELSEIF,
    SWITCH,
    CASE,
    DEFAULT,
    CONTINUE,
    BREAK,
    RETURN,
    // scope
    PKG,
    IMPORT,
    CONST,

    // lit
    IDENT,
    LIT,

    // operator
    // rel
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    // arith
    ADD,
    SUB,
    MUL,
    QUO,
    REM,
    AND,
    OR,
    XOR,
    NOT,
    SHL,
    SHR,
    // assign
    ASSIGN,
    INC,
    DEC,
    // logic
    LAND,
    LOR,
    LNOT,

    // delimeter
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    RARROW,
    COMMA,
    PERIOD,
    SEMICOLON,
    COLON,
    QUES,
    COMMENT,

    // others
    _EOF,
    ILLEGAL
};

token lookUpReserved() {
        
}

#define INT_LIT 0
#define FLOAT_LIT 1
#define BOOL_LIT 2
#define CHAR_LIT 3
#define STRING_LIT 4
#define NULL_LIT 5

#endif