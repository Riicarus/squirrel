#ifndef SYNTAXER_H
#define SYNTAXER_H

#include "token.h"
#include "ast.h"
#include <stdbool.h>

typedef bool (*update_list_func)(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme);

struct AstNode *parse();

#endif