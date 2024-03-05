#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdbool.h>

#define MAX_LINE_LEN 1024
#define MAX_WORD_LEN 256
#define MAX_NUMBER_LEN 32

struct _lexer {
    bool debug;

    char         *filename;
    char         *buffer;
    unsigned long buffer_len;

    char ch;
    int  off;
    int  row;
    int  col;

    token    tk;
    lit_kind lit_kind;
    char     lexeme[MAX_LINE_LEN];

    char *bad_msg;
};

extern struct _lexer *lexer;

void lexer_init(char *filepath, bool debug);

void lexer_free();

token lexer_next();

static void _next_skip_white_space();

static void _next_ch();

static void _contract();

static void _newline();

static bool _scan_word();

static bool _scan_number(bool float_part);

#endif