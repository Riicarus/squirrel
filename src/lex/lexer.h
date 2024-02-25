#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LEN 1024
#define MAX_WORD_LEN 256
#define MAX_NUMBER_LEN 16

struct _lexer {
    bool debug;

    char         *filename;
    char         *buffer;
    unsigned long buffer_len;

    char ch;
    int  off;
    int  row;
    int  col;
};

static struct _lexer *lexer = &(struct _lexer){};

void lexer_init(char *filepath, bool debug);

void lexer_free();

token next();

static void _next_skip_white_space();

static void _next_ch();

static void _contract();

static void _newline();

static char *_scan_word();

static char *_scan_number();

#endif