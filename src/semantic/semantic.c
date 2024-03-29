#include "semantic.h"
#include "type.h"

void manage_scope(struct AstNode *node, struct Scope *parent_scope, bool anonymous) {
    if (!node) return;

    node->scope = parent_scope;
    switch (node->class) {
        case CODE_FILE: {
            char name[128];
            sprintf(name, "code_file#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            node->scope = s;
            // in fact, code file should not have any parent scope
            if (!parent_scope) s->parent = parent_scope;
            struct CodeFile *code_file = node->data.code_file;
            manage_scope(code_file->code_block, s, false);
            break;
        }
        case CODE_BLOCK: {
            struct CodeBlock *code_block = node->data.code_block;
            if (anonymous) {
                char name[128];
                sprintf(name, "anonymous_block#%d", node->id);
                struct Scope *s = create_scope(parent_scope, name);
                parent_scope = s;
            }
            for (int i = 0; i < code_block->size; i++) manage_scope(code_block->stmts[i], parent_scope, true);
            break;
        }
        case FUNC_DECL: {
            struct FuncDecl *func_decl = node->data.func_decl;
            char             name[128];
            sprintf(name, "func[%s]#%d", func_decl->name_expr->data.name_expr->value, node->id);

            // name
            manage_scope(func_decl->name_expr, parent_scope, false);
            // ret_type
            manage_scope(func_decl->ret_type_decl, parent_scope, false);

            struct Symbol *symbol = create_symbol(create_signature_type(func_decl), func_decl->name_expr->data.name_expr->value, parent_scope, node->pos);

            struct Scope *s = create_scope(parent_scope, name);
            s->is_func = true;
            // func params
            for (int i = 0; i < func_decl->param_size; i++) manage_scope(func_decl->param_decls[i], s, false);
            // body
            manage_scope(func_decl->body, s, false);
            break;
        }
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;
            create_symbol(create_field_decl_type(field_decl), field_decl->name_expr->data.name_expr->value, parent_scope, node->pos);

            manage_scope(field_decl->type_decl, parent_scope, false);
            manage_scope(field_decl->name_expr, parent_scope, false);
            manage_scope(field_decl->assign_expr, parent_scope, false);
            break;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;

            // cond
            manage_scope(if_ctrl->cond, parent_scope, false);

            // then
            char name[128];
            sprintf(name, "if#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            manage_scope(if_ctrl->then, s, false);
            // else ifs
            for (int i = 0; i < if_ctrl->else_if_size; i++) manage_scope(if_ctrl->else_ifs[i], parent_scope, false);
            // else
            if (!if_ctrl->_else) break;
            sprintf(name, "else#%d", if_ctrl->_else->id);
            s = create_scope(parent_scope, name);
            manage_scope(if_ctrl->_else, s, false);
            // else's codeblock should in parent scope, not else's scope
            if_ctrl->_else->scope = parent_scope;
            break;
        }
        case ELSE_IF_CTRL: {
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;

            // cond
            manage_scope(else_if_ctrl->cond, parent_scope, false);

            // then
            char name[128];
            sprintf(name, "elseif#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            manage_scope(else_if_ctrl->then, s, false);
            break;
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;

            char name[128];
            sprintf(name, "for#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            // inits
            for (int i = 0; i < for_ctrl->inits_size; i++) manage_scope(for_ctrl->inits[i], s, false);
            // cond
            manage_scope(for_ctrl->cond, parent_scope, false);
            // updates
            for (int i = 0; i < for_ctrl->updates_size; i++) manage_scope(for_ctrl->updates[i], s, false);
            // body
            manage_scope(for_ctrl->body, s, false);
            break;
        }
        case RETURN_CTRL: {
            struct ReturnCtrl *return_ctrl = node->data.return_ctrl;
            manage_scope(return_ctrl->ret_val, parent_scope, false);
            break;
        }
        case OPERATION: {
            struct Operation *operation = node->data.operation;
            manage_scope(operation->x, parent_scope, false);
            manage_scope(operation->y, parent_scope, false);
            break;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;
            manage_scope(inc_expr->x, parent_scope, false);
            break;
        }
        case CALL_EXPR: {
            struct CallExpr *call_expr = node->data.call_expr;
            manage_scope(call_expr->func_expr, parent_scope, false);
            for (int i = 0; i < call_expr->param_size; i++) manage_scope(call_expr->params[i], parent_scope, false);
            break;
        }
        case NAME_EXPR:
        case BASIC_LIT:
        case BREAK_CTRL:
        case CONTINUE_CTRL:
        case BASIC_TYPE_DECL:
        case EMPTY_STMT: break;
        default: fprintf(stderr, "manage_scope(), invalid ast node class\n");
    }
}

struct Type *check_node_type(struct AstNode *node, struct Scope *parent_scope, struct Type *outer_type, bool anonymous) {
    if (!node) return NULL;

    struct Scope *cur_scope = parent_scope;
    switch (node->class) {
        case CODE_FILE: {
            struct CodeFile *code_file = node->data.code_file;
            cur_scope = node->scope;
            check_node_type(code_file->code_block, cur_scope, NULL, false);
            break;
        }
        case CODE_BLOCK: {
            struct CodeBlock *code_block = node->data.code_block;
            // if the code block is anonymous
            if (anonymous) {
                char name[128];
                sprintf(name, "anonymous_block#%d", node->id);
                cur_scope = enter_scope(cur_scope, name);
            }
            for (int i = 0; i < code_block->size; i++) check_node_type(code_block->stmts[i], cur_scope, outer_type, true);
            break;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;

            // if cond, in parent scope
            struct Type *cond_t = check_node_type(if_ctrl->cond, cur_scope, outer_type, false);
            if (!cond_t || cond_t->type_code != _basic_type && cond_t->data.basic_type->code != _bool_type) {
                fprintf(stderr, "at %s:%d:%d:%d, illegal type, expect bool\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
                exit(EXIT_FAILURE);
                return NULL;
            }

            char name[128];
            // enter if scope
            sprintf(name, "if#%d", node->id);
            cur_scope = enter_scope(cur_scope, name);

            // if-then, in if scope
            check_node_type(if_ctrl->then, cur_scope, outer_type, false);
            // exit if scope
            cur_scope = exit_scope(cur_scope);

            // if-else, in else scope
            // enter else scope
            if (if_ctrl->_else) {
                sprintf(name, "else#%d", if_ctrl->_else->id);
                cur_scope = enter_scope(cur_scope, name);
                check_node_type(if_ctrl->_else, cur_scope, outer_type, false);
                // exit else scope
                cur_scope = exit_scope(cur_scope);
            }

            // if-else-ifs, in parent scope
            for (int i = 0; i < if_ctrl->else_if_size; i++) check_node_type(if_ctrl->else_ifs[i], cur_scope, outer_type, false);
            break;
        }
        case ELSE_IF_CTRL: {
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;

            // elseif-cond, in parent scope
            struct Type *cond_t = check_node_type(else_if_ctrl->cond, cur_scope, outer_type, false);
            if (!cond_t || cond_t->type_code != _basic_type && cond_t->data.basic_type->code != _bool_type) {
                fprintf(stderr, "at %s:%d:%d:%d, illegal type, expect bool\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
                exit(EXIT_FAILURE);
                return NULL;
            }

            // enter elseif scope
            char name[128];
            sprintf(name, "elseif#%d", node->id);
            cur_scope = enter_scope(cur_scope, name);

            // elseif-then
            check_node_type(else_if_ctrl->then, cur_scope, outer_type, false);
            // exit elseif scope
            cur_scope = exit_scope(cur_scope);
            break;
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;

            // enter for scope
            char name[128];
            sprintf(name, "for#%d", node->id);
            cur_scope = enter_scope(cur_scope, name);

            // for-inits, in for scope
            for (int i = 0; i < for_ctrl->inits_size; i++) check_node_type(for_ctrl->inits[i], cur_scope, outer_type, false);

            // for-cond, in for scope
            struct Type *cond_t = check_node_type(for_ctrl->cond, cur_scope, outer_type, false);
            if (!cond_t || cond_t->type_code != _basic_type && cond_t->data.basic_type->code != _bool_type) {
                fprintf(stderr, "at %s:%d:%d:%d, illegal type, expect bool\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
                exit(EXIT_FAILURE);
                return NULL;
            }

            // for-updates, in for scope
            for (int i = 0; i < for_ctrl->updates_size; i++) check_node_type(for_ctrl->updates[i], cur_scope, outer_type, false);

            // for-body, in for scope
            check_node_type(for_ctrl->body, cur_scope, outer_type, false);

            // exit for scope
            cur_scope = exit_scope(cur_scope);
            break;
        }
        case FUNC_DECL: {
            struct FuncDecl *func_decl = node->data.func_decl;

            // func-ret type decl, in parent scope
            struct Type *ret_type = check_node_type(func_decl->ret_type_decl, cur_scope, NULL, false);

            // enter func scope
            char name[128];
            sprintf(name, "func[%s]#%d", func_decl->name_expr->data.name_expr->value, node->id);
            cur_scope = enter_scope(cur_scope, name);

            // func-param decls, in func scope
            for (int i = 0; i < func_decl->param_size; i++) check_node_type(func_decl->param_decls[i], cur_scope, NULL, false);

            // func-body, in func scope
            check_node_type(func_decl->body, cur_scope, ret_type, false);

            // exit func scope
            cur_scope = exit_scope(cur_scope);

            break;
        }
        case CALL_EXPR: {
            struct CallExpr *call_expr = node->data.call_expr;

            // lookup func symbol in scope
            struct Symbol *sym = scope_lookup_symbol_from_all(cur_scope, call_expr->func_expr->data.name_expr->value);
            return sym->type->data.signature_type->ret_type;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;

            // return operand's type
            return check_node_type(inc_expr->x, cur_scope, NULL, false);
        }
        case NAME_EXPR: {
            struct NameExpr *name_expr = node->data.name_expr;

            // lookup symbol in scope
            struct Symbol *sym = scope_lookup_symbol_from_all(cur_scope, name_expr->value);
            if (sym) return sym->type;

            fprintf(stderr, "at %s:%d:%d:%d, can not find symbol: %s\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col, name_expr->value);
            exit(EXIT_FAILURE);
            break;
        }
        case OPERATION: {
            struct Operation *operation = node->data.operation;

            struct Type *type_x = check_node_type(operation->x, cur_scope, NULL, false);
            struct Type *type_y = check_node_type(operation->y, cur_scope, NULL, false);

            if (!type_x) {
                fprintf(stderr,
                        "at %s:%d:%d:%d, can not get type\n",
                        operation->x->pos->filename,
                        operation->x->pos->off,
                        operation->x->pos->row,
                        operation->x->pos->col);
                exit(EXIT_FAILURE);
                break;
            }

            if (type_x->type_code != _basic_type) {
                fprintf(stderr,
                        "at %s:%d:%d:%d, illegal type for operation\n",
                        operation->x->pos->filename,
                        operation->x->pos->off,
                        operation->x->pos->row,
                        operation->x->pos->col);
                exit(EXIT_FAILURE);
                break;
            }

            if (!type_y) return type_x;

            if (type_x->type_code != type_y->type_code ||
                (type_x->type_code == _basic_type && type_x->data.basic_type->code != type_y->data.basic_type->code)) {
                fprintf(stderr,
                        "at %s:%d:%d:%d, illegal type for operation\n",
                        operation->y->pos->filename,
                        operation->y->pos->off,
                        operation->y->pos->row,
                        operation->y->pos->col);
                exit(EXIT_FAILURE);
                break;
            }

            return type_x;
        }
        case RETURN_CTRL: {
            struct ReturnCtrl *return_ctrl = node->data.return_ctrl;

            if (!outer_type) {
                if (!return_ctrl->ret_val) break;

                fprintf(stderr, "at %s:%d:%d:%d, illegal return type, need: void\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
                exit(EXIT_FAILURE);
                break;
            }

            if (outer_type->type_code == _basic_type && outer_type->data.basic_type->code == _void_type) {
                if (!return_ctrl->ret_val) break;

                fprintf(stderr, "at %s:%d:%d:%d, illegal return type, need: void\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
                exit(EXIT_FAILURE);
                break;
            }

            if (return_ctrl->ret_val) {
                struct Type *t = check_node_type(return_ctrl->ret_val, cur_scope, NULL, false);
                if (t && t->type_code == outer_type->type_code) return outer_type;
            }

            fprintf(stderr, "at %s:%d:%d:%d, illegal return type\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
            exit(EXIT_FAILURE);
            break;
        }
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;

            if (!field_decl->assign_expr) break;

            check_node_type(field_decl->assign_expr, cur_scope, NULL, false);
            break;
        }
        case BASIC_LIT: {
            struct BasicLit *basic_lit = node->data.basic_lit;

            struct Type *t = CREATE_STRUCT_P(Type);
            t->type_code = _basic_type;
            t->data.basic_type = &basic_types[basic_lit->lk];
            return t;
        }
        case BASIC_TYPE_DECL: return create_basic_type(node->data.basic_type_decl);
        case EMPTY_STMT:
        case BREAK_CTRL:
        case CONTINUE_CTRL: break;
        default: {
            fprintf(stderr, "check_node_type(), invalid ast node class\n");
            exit(EXIT_FAILURE);
        }
    }

    return NULL;
}

bool check_stmt(struct AstNode *node, bool must_return, bool in_loop) {
    if (!node) return false;

    node->reachable = true;
    switch (node->class) {
        case CODE_FILE: {
            struct CodeFile *code_file = node->data.code_file;
            check_stmt(code_file->code_block, false, false);
            break;
        }
        case CODE_BLOCK: {
            struct CodeBlock *code_block = node->data.code_block;
            bool              returned = false;
            bool              reachable = true;
            struct Position  *pos = node->pos;
            for (int i = 0; i < code_block->size && reachable; i++) {
                enum NodeClass class = code_block->stmts[i]->class;
                if (class == RETURN_CTRL) reachable = false;
                if (class == CONTINUE_CTRL || class == BREAK_CTRL) {
                    if (in_loop) reachable = false;
                    else {
                        pos = code_block->stmts[i]->pos;
                        fprintf(stderr, "at %s:%d:%d:%d, illegal statement\n", pos->filename, pos->off, pos->row, pos->col);
                        exit(EXIT_FAILURE);
                        return false;
                    }
                }
                returned |= check_stmt(code_block->stmts[i], false, in_loop);
                if (returned) reachable = false;
            }

            if (must_return && !returned) {
                fprintf(stderr, "at %s:%d:%d:%d, missing return statement\n", pos->filename, pos->off, pos->row, pos->col);
                exit(EXIT_FAILURE);
                return false;
            }
            return returned;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;
            bool           returned = true;
            check_stmt(if_ctrl->cond, false, in_loop);
            returned &= check_stmt(if_ctrl->then, false, in_loop);
            returned &= check_stmt(if_ctrl->_else, false, in_loop);
            for (int i = 0; i < if_ctrl->else_if_size; i++) returned &= check_stmt(if_ctrl->else_ifs[i], false, in_loop);
            return returned;
        }
        case ELSE_IF_CTRL: {
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;
            check_stmt(else_if_ctrl->cond, false, in_loop);
            return check_stmt(else_if_ctrl->then, false, in_loop);
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;
            // inits
            for (int i = 0; i < for_ctrl->inits_size; i++) check_stmt(for_ctrl->inits[i], false, false);
            // updates
            for (int i = 0; i < for_ctrl->updates_size; i++) check_stmt(for_ctrl->updates[i], false, false);
            // cond
            check_stmt(for_ctrl->cond, false, false);
            // body
            return check_stmt(for_ctrl->body, false, true);
        }
        case FUNC_DECL: {
            struct FuncDecl *func_decl = node->data.func_decl;
            must_return = !(func_decl->ret_type_decl->class == BASIC_TYPE_DECL && func_decl->ret_type_decl->data.basic_type_decl->tk == _void_type);
            
            // name
            check_stmt(func_decl->name_expr, false, in_loop);
            // params
            for (int i = 0; i < func_decl->param_size; i++) check_stmt(func_decl->param_decls[i], false, in_loop);
            // ret_type
            check_stmt(func_decl->ret_type_decl, false, in_loop);
            // body
            check_stmt(func_decl->body, must_return, in_loop);
            return false;
        }
        case RETURN_CTRL: {
            struct ReturnCtrl *return_ctrl = node->data.return_ctrl;
            check_stmt(return_ctrl->ret_val, false, in_loop);
            return true;
        }
        case BASIC_LIT: break;
        case CALL_EXPR: {
            struct CallExpr *call_expr = node->data.call_expr;
            check_stmt(call_expr->func_expr, false, in_loop);
            for (int i = 0; i < call_expr->param_size; i++) check_stmt(call_expr->params[i], false, in_loop);
            break;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;
            check_stmt(inc_expr->x, false, inc_expr);
            break;
        }
        case OPERATION: {
            struct Operation *operation = node->data.operation;
            check_stmt(operation->x, false, in_loop);
            check_stmt(operation->y, false, in_loop);
            break;
        }
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;
            check_stmt(field_decl->type_decl, false, in_loop);
            check_stmt(field_decl->name_expr, false, in_loop);
            check_stmt(field_decl->assign_expr, false, in_loop);
            break;
        }
        case EMPTY_STMT:
        case BREAK_CTRL:
        case CONTINUE_CTRL:
        case NAME_EXPR:
        case BASIC_TYPE_DECL: break;
        default: {
            fprintf(stderr, "check_node_type(), invalid ast node class\n");
            exit(EXIT_FAILURE);
        }
    }

    return false;
}