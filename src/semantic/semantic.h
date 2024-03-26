#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "scope.h"

void         manage_scope(struct AstNode *node, struct Scope *parent_scope, bool anonymous);
struct Type *check_node_type(struct AstNode *node);

#endif