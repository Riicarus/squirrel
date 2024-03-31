#include "ir_gen.h"
#include "scope.h"
#include "type.h"

#include <stdio.h>
#include <stdlib.h>

static int var_id = 0;

char *_pack_str_arg(char *name, char prefix, bool need_free) {
    char *packed_name = calloc(256, sizeof(char));
    if (!packed_name) {
        fprintf(stderr, "_pack_str_arg(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }
    sprintf(packed_name, "%c#%s", prefix, name);
    if (need_free) free(name);

    return packed_name;
}

char *_pack_int_arg(int val) {
    char *packed_name = calloc(256, sizeof(char));
    if (!packed_name) {
        fprintf(stderr, "_pack_int_arg(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }
    sprintf(packed_name, "%c#%d", LIT_PREFIX, val);

    return packed_name;
}

char *_pack_float_arg(float val) {
    char *packed_name = calloc(256, sizeof(char));
    if (!packed_name) {
        fprintf(stderr, "_pack_float_arg(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }
    sprintf(packed_name, "%c#%f", LIT_PREFIX, val);

    return packed_name;
}

char *_unpack_name(char *name) {
    if (!name) return NULL;

    return name + 2;
}

void *_gen_temp_var_name() {
    char *name = calloc(254, sizeof(char));
    if (!name) {
        fprintf(stderr, "_gen_temp_var(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }
    sprintf(name, "t%d", var_id++);
    return _pack_str_arg(name, VAR_PREFIX, true);
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
            char  if_true[256];
            char  if_end[256];
            sprintf(if_true, "%s#%d", IF_TRUE, node->id);
            sprintf(if_end, "%s#%d", IF_END, node->id);
            // JE x, 0, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, x_name, _pack_int_arg(0), if_true);
            // JE y, 0, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, y_name, _pack_int_arg(0), if_true);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(1), NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, if_end, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, if_true, NULL, NULL);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(0), NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, if_end, NULL, NULL);
            return res_name;
        }
        case LOR: {
            char *x_name = gen_tac_from_ast(op->x, tac);
            char *y_name = gen_tac_from_ast(op->y, tac);
            char *res_name = _gen_temp_var_name();
            char  if_true[256];
            char  if_end[256];
            sprintf(if_true, "%s#%d", IF_TRUE, node->id);
            sprintf(if_end, "%s#%d", IF_END, node->id);
            // JE x, 1, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, x_name, _pack_int_arg(1), if_true);
            // JE y, 1, IF_TRUE
            *tac = create_tac(*tac, TAC_JE, y_name, _pack_int_arg(1), if_true);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(0), NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, if_end, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, if_true, NULL, NULL);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(1), NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, if_end, NULL, NULL);
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
            char  if_false[256];
            char  if_end[256];
            sprintf(if_false, "%s#%d", IF_FALSE, node->id);
            sprintf(if_end, "%s#%d", IF_END, node->id);
            // JE a, 1, IF_FALSE
            *tac = create_tac(*tac, TAC_JE, cond_name, _pack_int_arg(1), if_false);
            // MOV res, 1
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(1), NULL);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, if_end, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, if_false, NULL, NULL);
            // MOV res, 0
            *tac = create_tac(*tac, TAC_MOV, res_name, _pack_int_arg(0), NULL);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, if_end, NULL, NULL);
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

            char *param_size = _pack_int_arg(call_expr->param_size);

            // return val
            char          *ret = NULL;
            struct Symbol *sym = scope_lookup_symbol_from_all(call_expr->func_expr->scope, func_name);
            struct Type   *ret_type = sym->type->data.signature_type->ret_type;
            if (ret_type->type_code != _basic_type || ret_type->data.basic_type->code != _void_type) ret = _gen_temp_var_name();

            // res = call x, y
            *tac = create_tac(*tac, TAC_CALL, _pack_str_arg(func_name, VAR_PREFIX, false), param_size, ret);
            return ret;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;
            char           *x_name = gen_tac_from_ast(inc_expr->x, tac);
            enum TacOpCode  op = inc_expr->is_inc ? TAC_ADD : TAC_SUB;
            // ++x
            // x = x + 1
            if (inc_expr->is_pre) {
                *tac = create_tac(*tac, op, x_name, _pack_int_arg(1), x_name);
                return x_name;
            }
            // x++
            // t1 = x
            // x = x + 1
            char *res_name = _gen_temp_var_name();
            *tac = create_tac(*tac, TAC_MOV, x_name, res_name, NULL);
            *tac = create_tac(*tac, op, x_name, _pack_int_arg(1), x_name);
            return res_name;
        }
        case NAME_EXPR: return _pack_str_arg(node->data.name_expr->value, VAR_PREFIX, false);
        case OPERATION: return _gen_tac_from_operation(node, tac);
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;
            char             *default_var = _pack_str_arg(basic_type_default_val[field_decl->type_decl->data.basic_type_decl->tk], LIT_PREFIX, false);
            char             *var_name = _pack_str_arg(field_decl->name_expr->data.name_expr->value, VAR_PREFIX, false);
            *tac = create_tac(*tac, TAC_MOV, var_name, default_var, NULL);
            gen_tac_from_ast(field_decl->assign_expr, tac);
            return var_name;
        }
        case FUNC_DECL: {
            struct FuncDecl *func_decl = node->data.func_decl;
            *tac = create_tac(*tac, TAC_FUNC_S, _pack_str_arg(func_decl->name_expr->data.name_expr->value, VAR_PREFIX, false), NULL, NULL);
            gen_tac_from_ast(func_decl->body, tac);
            *tac = create_tac(*tac, TAC_FUNC_E, _pack_str_arg(func_decl->name_expr->data.name_expr->value, VAR_PREFIX, false), NULL, NULL);
            break;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;
            char           if_true[256];
            char           if_false[256];
            char           if_end[256];
            sprintf(if_true, "%s#%d", IF_TRUE, node->id);
            sprintf(if_false, "%s#%d", IF_FALSE, node->id);
            sprintf(if_end, "%s#%d", IF_END, node->id);
            // t1 = a < b
            char *cond_res = gen_tac_from_ast(if_ctrl->cond, tac);
            // JE t1, 1 IF_TRUE
            *tac = create_tac(*tac, TAC_JE, cond_res, _pack_int_arg(1), if_true);
            // JMP IF_FALSE
            *tac = create_tac(*tac, TAC_JMP, if_false, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, if_true, NULL, NULL);
            gen_tac_from_ast(if_ctrl->then, tac);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, if_end, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, if_false, NULL, NULL);
            for (int i = 0; i < if_ctrl->else_if_size; i++) gen_tac_from_ast(if_ctrl->else_ifs[i], tac);
            gen_tac_from_ast(if_ctrl->_else, tac);
            // LABEL IF_END
            *tac = create_tac(*tac, TAC_LABEL, if_end, NULL, NULL);
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
            char               if_true[256];
            char               if_false[256];
            char               if_end[256];
            sprintf(if_true, "%s#%d", IF_TRUE, node->id);
            sprintf(if_false, "%s#%d", IF_FALSE, node->id);
            sprintf(if_end, "%s#%d", IF_END, node->id);
            // t1 = a < b
            char *cond_res = gen_tac_from_ast(else_if_ctrl->cond, tac);
            // JE t1, 1 IF_TRUE
            *tac = create_tac(*tac, TAC_JE, cond_res, _pack_int_arg(1), if_true);
            // JMP IF_FALSE
            *tac = create_tac(*tac, TAC_JMP, if_false, NULL, NULL);
            // LABEL IF_TRUE
            *tac = create_tac(*tac, TAC_LABEL, if_true, NULL, NULL);
            gen_tac_from_ast(else_if_ctrl->then, tac);
            // JMP IF_END
            *tac = create_tac(*tac, TAC_JMP, if_end, NULL, NULL);
            // LABEL IF_FALSE
            *tac = create_tac(*tac, TAC_LABEL, if_false, NULL, NULL);
            break;
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;
            char            for_start[256];
            char            for_body[256];
            char            for_end[256];
            sprintf(for_start, "%s#%d", FOR_START, node->id);
            sprintf(for_body, "%s#%d", FOR_BODY, node->id);
            sprintf(for_end, "%s#%d", FOR_END, node->id);
            // for inits
            for (int i = 0; i < for_ctrl->inits_size; i++) gen_tac_from_ast(for_ctrl->inits[i], tac);
            // LABEL FOR_START
            *tac = create_tac(*tac, TAC_LABEL, for_start, NULL, NULL);
            // cond t1 = i <= n
            char *cond_res = gen_tac_from_ast(for_ctrl->cond, tac);
            // JE t1, 1 FOR_BODY
            *tac = create_tac(*tac, TAC_JE, cond_res, _pack_int_arg(1), for_body);
            // JMP FOR_END
            *tac = create_tac(*tac, TAC_JMP, for_end, NULL, NULL);
            // LABEL FOR_BODY
            *tac = create_tac(*tac, TAC_LABEL, for_body, NULL, NULL);
            // for body
            gen_tac_from_ast(for_ctrl->body, tac);
            // for updates
            for (int i = 0; i < for_ctrl->updates_size; i++) gen_tac_from_ast(for_ctrl->updates[i], tac);
            // JMP FOR_START
            *tac = create_tac(*tac, TAC_JMP, for_start, NULL, NULL);
            // LABEL FOR_END
            *tac = create_tac(*tac, TAC_LABEL, for_end, NULL, NULL);
            break;
        }
        case BREAK_CTRL: {
            char for_end[256];
            sprintf(for_end, "%s#%d", FOR_END, node->id);
            *tac = create_tac(*tac, TAC_JMP, for_end, NULL, NULL);
            break;
        }
        case CONTINUE_CTRL: {
            char for_start[256];
            sprintf(for_start, "%s#%d", FOR_START, node->id);
            *tac = create_tac(*tac, TAC_JMP, for_start, NULL, NULL);
            break;
        }
        case BASIC_LIT: return _pack_str_arg(node->data.basic_lit->value, LIT_PREFIX, false);
        case BASIC_TYPE_DECL:
        case EMPTY_STMT: break;
    }

    return NULL;
}

struct VarUsageEntry {
        char name[256];
        int  cnt;
};

struct VarUsageEntry *_create_var_usage_entry(char *name, int cnt) {
    struct VarUsageEntry *entry = CREATE_STRUCT_P(VarUsageEntry);
    if (!entry) {
        fprintf(stderr, "_create_var_usage_entry(), no enough memory");
        exit(EXIT_FAILURE);
    }

    strcpy(entry->name, name);
    entry->cnt = cnt;
    return entry;
}

void *get_var_usage_name(void *ele) {
    return ((struct VarUsageEntry *)ele)->name;
}

void *get_var_usage_cnt(void *ele) {
    return &((struct VarUsageEntry *)ele)->cnt;
}

void update_var_usage_cnt(void *ele1, void *ele2) {
    ((struct VarUsageEntry *)ele1)->cnt = ((struct VarUsageEntry *)ele2)->cnt;
}

hashmap create_used_var_map() {
    hashmap map = hashmap_new_default(get_var_usage_name, get_var_usage_cnt, update_var_usage_cnt, str_hash_func, str_eq_func, int_eq_func);
    return map;
}

// constant folding & propagation
void tac_constant_optimize(struct TAC *tail_tac, hashmap map) {
    if (!tail_tac) return;
    if (!map) map = create_used_var_map();

    switch (tail_tac->op) {
        case TAC_EQ:
        case TAC_NE:
        case TAC_LT:
        case TAC_LE:
        case TAC_GT:
        case TAC_GE:
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_QUO:
        case TAC_REM:
        case TAC_AND:
        case TAC_OR:
        case TAC_XOR:
        case TAC_SHL:
        case TAC_SHR:
        case TAC_NOT:
        case TAC_MOV:
        case TAC_JMP:
        case TAC_JE:
        case TAC_JNE:
        case TAC_LABEL:
        case TAC_FUNC_S:
        case TAC_FUNC_E: break;
        case TAC_PARAM: {
            if (*tail_tac->x == VAR_PREFIX) hashmap_put(map, _create_var_usage_entry(_unpack_name(tail_tac->x), 1));
            break;
        }
        case TAC_CALL: {
            char                 *func_name = _unpack_name(tail_tac->x);
            struct VarUsageEntry *entry = _create_var_usage_entry(_unpack_name(func_name), 1);
            if (hashmap_contains_key(map, entry)) hashmap_put(map, entry);
            else {
                *tail_tac->res = '\0';
                free(entry);
            }

            break;
        }
        case TAC_RET:
            if (*tail_tac->x == VAR_PREFIX) hashmap_put(map, _unpack_name(tail_tac->x));
    }

    tac_constant_optimize(tail_tac->prev, map);
}

// dead code elimination
void tac_dead_code_optimize(struct TAC *tac) {
    if (!tac) return;

    tac_dead_code_optimize(tac->next);
}

// copy propagation
void tac_copy_propagation_optimize(struct TAC *tac);
