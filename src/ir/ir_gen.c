#include "ir_gen.h"
#include <stdio.h>
#include <stdlib.h>

static int var_id = 0;

void *_gen_temp_var() {
    char *name = calloc(16, sizeof(char));
    if (!name) {
        fprintf(stderr, "_gen_temp_var(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }

    sprintf(name, "t%d", var_id);
    return name;
}

char *_get_operand(struct AstNode *node, struct Scope *scope, struct Quad **quads) {
    return NULL;
}

enum QuadOpCode _get_operator(struct AstNode *node, struct Scope *scope, struct Quad **quads) {
    return NULL;
}

struct Quad *_gen_quads_from_expression(struct AstNode *node, struct Scope *scope) {
    return NULL;
}

struct Quad *gen_quads_from_ast(struct AstNode *node, struct Scope *scope) {
    if (!node || (!scope && node->class != CODE_FILE)) return NULL;

    switch (node->class) {
        case CODE_FILE: {
            struct CodeFile *code_file = node->data.code_file;
            return gen_quads_from_ast(code_file->code_block, code_file->scope);
        }
        case CODE_BLOCK: {
            struct CodeBlock *code_block = node->data.code_block;
            struct Quad      *quad = NULL;
            for (int i = 0; i < code_block->size; i++) {
                if (!quad) quad = gen_quads_from_ast(code_block->stmts[i], scope);
                else {
                    quad->next = gen_quads_from_ast(code_block->stmts[i], scope);
                    if (quad->next) quad = quad->next;
                }
            }
            return quad;
        }
        case CALL_EXPR: {
            struct CallExpr *call_expr = node->data.call_expr;
            struct Quad *quad = CREATE_STRUCT_P(Quad);
            quad->op = QUAD_CALL;
            quad->x = calloc(strlen(call_expr->func_expr->data.name_expr->value) + 1, sizeof(char));
        }
        case INC_EXPR:
        case NAME_EXPR:
        case OPERATION:
        case BREAK_CTRL:
        case CONTINUE_CTRL:
        case RETURN_CTRL:
        case IF_CTRL:
        case ELSE_IF_CTRL:
        case FOR_CTRL:
        case FUNC_DECL:
        case FIELD_DECL:
        case BASIC_TYPE_DECL:
        case EMPTY_STMT:
        case BASIC_LIT: break;
    }

    return NULL;
}