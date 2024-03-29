#ifndef IR_GEN_H
#define IR_GEN_H

#include "ast.h"
#include "ir.h"

struct Quad *gen_quads_from_ast(struct AstNode *node, struct Scope *scope);

#endif