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

#define VAR_PREFIX 'V'
#define LIT_PREFIX 'L'
#define FUNC_S_PREFIX 'S'
#define FUNC_E_PREFIX 'E'

char *pack_str_arg(char *name, char prefix, bool need_free);
char *pack_int_arg(int val);
char *pack_float_arg(float val);
char *unpack_name(char *name);

// return the result var name
char *gen_tac_from_ast(struct AstNode *node, struct TAC **tac, char *func_name);

#endif