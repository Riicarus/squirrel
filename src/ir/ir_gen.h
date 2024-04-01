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

// return the result var name
char *gen_tac_from_ast(struct AstNode *node, struct TAC **tac);

hashmap create_used_var_map();

// constant folding & propagation
struct TAC *tac_global_var_removal(struct TAC *tail_tac, hashmap map);

// dead code elimination
void tac_dead_code_optimize(struct TAC *tac);

// copy propagation
void tac_copy_propagation_optimize(struct TAC *tac);

void tac_optimize(struct TAC *tac);

#endif