#ifndef LEX_H
#define LEX_H

#include "token.h"
#include <stdbool.h>

#define MAX_LINE_LEN 1024
#define MAX_WORD_LEN 256
#define MAX_NUMBER_LEN 32

extern bool         debug;
extern char        *filename;
extern char         ch;
extern int          off;
extern int          row;
extern int          col;
extern enum Token   tk;
extern enum LitKind lk;
extern char         lexeme[MAX_LINE_LEN];
extern char        *lex_bad_msg;

bool       lex_init(char *filepath, bool debug);
enum Token lex_next();

static void _next_skip_white_space();
static void _next_ch();
static void _contract();
static void _newline();
static bool _scan_word();
static bool _scan_number(bool float_part);

#endif