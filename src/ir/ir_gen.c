#include "ir_gen.h"
#include "scope.h"
#include "type.h"

#include <stdio.h>
#include <stdlib.h>

static int var_id = 0;

void *_gen_temp_var_name() {
    char *name = calloc(256, sizeof(char));
    if (!name) {
        fprintf(stderr, "_gen_temp_var(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }

    sprintf(name, "t%d", var_id++);
    return name;
}

char *_gen_tac_from_operation(struct AstNode *node, struct TAC **tac) {
    struct Operation *op = node->data.operation;

    switch (op->op) {
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
        case ADD:
        case SUB:
        case MUL:
        case QUO:
        case REM:
        case AND:
        case OR:
        case XOR:
        case SHL:
        case SHR: {
            char *res_name = _gen_temp_var_name();
            *tac = create_tac(*tac, ((enum TacOpCode)(op->op - EQ)), gen_tac_from_ast(op->x, tac), gen_tac_from_ast(op->y, tac), res_name);
            return res_name;
        }
        case LAND: {
            char *x_name = gen_tac_from_ast(op->x, tac);
            char *y_name = gen_tac_from_ast(op->y, tac);
            char *res_name = _gen_temp_var_name();
            // JE x, 0, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, x_name, "0", IF_TRUE);
            // JE y, 0, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, y_name, "0", IF_TRUE);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, "1", NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, IF_END, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, IF_TRUE, NULL, NULL);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, "0", NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, IF_END, NULL, NULL);
            return res_name;
        }
        case LOR: {
            char *x_name = gen_tac_from_ast(op->x, tac);
            char *y_name = gen_tac_from_ast(op->y, tac);
            char *res_name = _gen_temp_var_name();
            // JE x, 1, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, x_name, "1", IF_TRUE);
            // JE y, 1, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, y_name, "1", IF_TRUE);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, "0", NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, IF_END, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, IF_TRUE, NULL, NULL);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, "1", NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, IF_END, NULL, NULL);
            return res_name;
        }
        case NOT: {
            char *res_name = gen_tac_from_ast(op->x, tac);
            *tac = create_tac(*tac, TAC_NOT, res_name, NULL, NULL);
            return res_name;
        }
        case LNOT: {
            char *cond_name = gen_tac_from_ast(op->x, tac);
            char *res_name = _gen_temp_var_name();
            // JE a, 1, IF_FALSE
            *tac = create_tac(*tac, TAC_JE, cond_name, "1", IF_FALSE);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, "1", NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, IF_END, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, IF_FALSE, NULL, NULL);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, "0", NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, IF_END, NULL, NULL);
            return res_name;
        }
        case ASSIGN: {
            char *res_name = gen_tac_from_ast(op->x, tac);
            *tac = create_tac(*tac, TAC_MOV, res_name, gen_tac_from_ast(op->y, tac), NULL);
            return res_name;
        }
    }

    return NULL;
}

char *gen_tac_from_ast(struct AstNode *node, struct TAC **tac) {
    if (!node) return NULL;

    if (!node->reachable) return NULL;
    switch (node->class) {
        case CODE_FILE: {
            struct CodeFile *code_file = node->data.code_file;
            gen_tac_from_ast(code_file->code_block, tac);
            break;
        }
        case CODE_BLOCK: {
            struct CodeBlock *code_block = node->data.code_block;
            for (int i = 0; i < code_block->size; i++) gen_tac_from_ast(code_block->stmts[i], tac);
            break;
        }
        case CALL_EXPR: {
            struct CallExpr *call_expr = node->data.call_expr;
            char            *func_name = call_expr->func_expr->data.name_expr->value;

            // prepare params
            for (int i = 0; i < call_expr->param_size; i++) *tac = create_tac(*tac, TAC_PARAM, gen_tac_from_ast(call_expr->params[i], tac), NULL, NULL);

            // res = call x, y
            char param_size[256];
            sprintf(param_size, "%d", call_expr->param_size);

            // return val
            char          *ret = NULL;
            struct Symbol *sym = scope_lookup_symbol_from_all(call_expr->func_expr->scope, func_name);
            if (sym->type->data.signature_type->ret_type->type_code != _void_type) {
                ret = _gen_temp_var_name();
            }
            *tac = create_tac(*tac, TAC_CALL, func_name, param_size, ret);
            return ret;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;
            char           *x_name = gen_tac_from_ast(inc_expr->x, tac);
            enum TacOpCode  op = inc_expr->is_inc ? TAC_ADD : TAC_SUB;
            // ++x
            // x = x + 1
            if (inc_expr->is_pre) {
                *tac = create_tac(*tac, op, x_name, "1", x_name);
                return x_name;
            }
            // x++
            // t1 = x
            // x = x + 1
            char *res_name = _gen_temp_var_name();
            *tac = create_tac(*tac, TAC_MOV, x_name, NULL, res_name);
            *tac = create_tac(*tac, op, x_name, "1", x_name);
            return res_name;
        }
        case NAME_EXPR: return node->data.name_expr->value;
        case OPERATION: return _gen_tac_from_operation(node, tac);
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;
            char             *var_name = field_decl->name_expr->data.name_expr->value;
            *tac = create_tac(*tac, TAC_MOV, var_name, "0", NULL);
            gen_tac_from_ast(field_decl->assign_expr, tac);
            return var_name;
        }
        case FUNC_DECL: {
            struct FuncDecl *func_decl = node->data.func_decl;
            *tac = create_tac(*tac, TAC_FUNC_S, func_decl->name_expr->data.name_expr->value, NULL, NULL);
            gen_tac_from_ast(func_decl->body, tac);
            *tac = create_tac(*tac, TAC_FUNC_E, NULL, NULL, NULL);
            break;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;
            // t1 = a < b
            char          *cond_res = gen_tac_from_ast(if_ctrl->cond, tac);
            // JE t1, 1 IF_TRUE
            *tac = create_tac(*tac, TAC_JE, cond_res, "1", IF_TRUE);
            // JMP IF_FALSE
            *tac = create_tac(*tac, TAC_JMP, IF_FALSE, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, IF_TRUE, NULL, NULL);
            gen_tac_from_ast(if_ctrl->then, tac);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, IF_END, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, IF_FALSE, NULL, NULL);
            for (int i = 0; i < if_ctrl->else_if_size; i++) gen_tac_from_ast(if_ctrl->else_ifs[i], tac);
            gen_tac_from_ast(if_ctrl->_else, tac);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, IF_END, NULL, NULL);
            break;
        }
        case RETURN_CTRL: {
            struct ReturnCtrl *return_ctrl = node->data.return_ctrl;
            char              *ret_var = gen_tac_from_ast(return_ctrl->ret_val, tac);
            *tac = create_tac(*tac, TAC_RET, ret_var, NULL, NULL);
            break;
        }
        case ELSE_IF_CTRL: {
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;
            // t1 = a < b
            char              *cond_res = gen_tac_from_ast(else_if_ctrl->cond, tac);
            // JE t1, 1 IF_TRUE
            *tac = create_tac(*tac, TAC_JE, cond_res, "1", IF_TRUE);
            // JMP IF_FALSE
            *tac = create_tac(*tac, TAC_JMP, IF_FALSE, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, IF_TRUE, NULL, NULL);
            gen_tac_from_ast(else_if_ctrl->then, tac);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, IF_END, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, IF_FALSE, NULL, NULL);
            break;
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;
            // for inits
            for (int i = 0; i < for_ctrl->inits_size; i++) gen_tac_from_ast(for_ctrl->inits[i], tac);
            // LABEL FOR_START
            *tac = create_tac(*tac, TAC_LABEL, FOR_START, NULL, NULL);
            // cond t1 = i <= n
            char *cond_res = gen_tac_from_ast(for_ctrl->cond, tac);
            // JE t1, 1 FOR_BODY
            *tac = create_tac(*tac, TAC_JE, cond_res, "1", FOR_BODY);
            // JMP FOR_END
            *tac = create_tac(*tac, TAC_JMP, FOR_END, NULL, NULL);
            // LABEL FOR_BODY
            *tac = create_tac(*tac, TAC_LABEL, FOR_BODY, NULL, NULL);
            // for body
            gen_tac_from_ast(for_ctrl->body, tac);
            // for updates
            for (int i = 0; i < for_ctrl->updates_size; i++) gen_tac_from_ast(for_ctrl->updates[i], tac);
            // JMP FOR_START
            *tac = create_tac(*tac, TAC_JMP, FOR_START, NULL, NULL);
            // LABEL FOR_END
            *tac = create_tac(*tac, TAC_LABEL, FOR_END, NULL, NULL);
            break;
        }
        case BREAK_CTRL: {
            *tac = create_tac(*tac, TAC_JMP, FOR_END, NULL, NULL);
            break;
        }
        case CONTINUE_CTRL: {
            *tac = create_tac(*tac, TAC_JMP, FOR_START, NULL, NULL);
            break;
        }
        case BASIC_LIT: return node->data.basic_lit->value;
        case BASIC_TYPE_DECL:
        case EMPTY_STMT: break;
    }

    return NULL;
}