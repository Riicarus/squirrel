#ifndef IR_GEN_H
#define IR_GEN_H

#include "ast.h"
#include "ir.h"

#define IF_TRUE "IF_TRUE"
#define IF_FALSE "IF_FALSE"
#define IF_END "IF_END"

#define FOR_START "FOR_START"
#define FOR_BODY "FOR_BODY"
#define FOR_END "FOR_END"

// return the result var name
char *gen_tac_from_ast(struct AstNode *node, struct TAC **tac);

#endif