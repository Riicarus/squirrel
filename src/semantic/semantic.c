#include "semantic.h"
#include "type.h"

void manage_scope(struct AstNode *node, struct Scope *parent_scope, bool anonymous) {
    if (!node) return;

    switch (node->class) {
        case CODE_FILE: {
            char name[128];
            sprintf(name, "code_file#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            node->data.code_file->scope = s;
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

            struct Symbol *symbol = create_symbol(create_signature_type(func_decl), func_decl->name_expr->data.name_expr->value, parent_scope, node->pos);

            struct Scope *s = create_scope(parent_scope, name);
            s->is_func = true;
            for (int i = 0; i < func_decl->param_size; i++) manage_scope(func_decl->param_decls[i], s, false);
            manage_scope(func_decl->body, s, false);
            break;
        }
        case FIELD_DECL: {
            struct FieldDecl *field_decl = node->data.field_decl;
            // TODO: get field decl type
            create_symbol(create_field_decl_type(field_decl), field_decl->name_expr->data.name_expr->value, parent_scope, node->pos);
            break;
        }
        case IF_CTRL: {
            struct IfCtrl *if_ctrl = node->data.if_ctrl;
            char           name[128];
            sprintf(name, "if#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            // then
            manage_scope(if_ctrl->then, s, false);
            // else ifs
            for (int i = 0; i < if_ctrl->else_if_size; i++) manage_scope(if_ctrl->else_ifs[i], parent_scope, false);
            // else
            sprintf(name, "else#%d", if_ctrl->_else->id);
            s = create_scope(parent_scope, name);
            manage_scope(if_ctrl->_else, s, false);
            break;
        }
        case ELSE_IF_CTRL: {
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;
            char               name[128];
            sprintf(name, "elseif#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            manage_scope(else_if_ctrl->then, s, false);
            break;
        }
        case FOR_CTRL: {
            struct ForCtrl *for_ctrl = node->data.for_ctrl;
            char            name[128];
            sprintf(name, "for#%d", node->id);
            struct Scope *s = create_scope(parent_scope, name);
            // inits
            for (int i = 0; i < for_ctrl->inits_size; i++) manage_scope(for_ctrl->inits[i], s, false);
            // body
            manage_scope(for_ctrl->body, s, false);
            break;
        }
        case ARRAY_TYPE_DECL:
        case BASIC_TYPE_DECL:
        case BASIC_LIT:
        case ARRAY_LIT:
        case CALL_EXPR:
        case INC_EXPR:
        case INDEX_EXPR:
        case NAME_EXPR:
        case OPERATION:
        case SIZE_EXPR:
        case BREAK_CTRL:
        case CONTINUE_CTRL:
        case RETURN_CTRL:
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
            cur_scope = code_file->scope;
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
            sprintf(name, "else#%d", if_ctrl->_else->id);
            cur_scope = enter_scope(cur_scope, name);
            check_node_type(if_ctrl->_else, cur_scope, outer_type, false);
            // exit else scope
            cur_scope = exit_scope(cur_scope);

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
            return sym->type;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;

            // return operand's type
            return check_node_type(inc_expr->x, cur_scope, NULL, false);
        }
        case INDEX_EXPR: {
            struct IndexExpr *index_expr = node->data.index_expr;

            // return operand's element type, operand must be array
            struct Type *array_type = check_node_type(index_expr->x, cur_scope, NULL, false);
            if (array_type && array_type->type_code == _array_type) return array_type->data.array_type->ele_type;

            fprintf(stderr, "at %s:%d:%d:%d, illegal type, expect array\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
            exit(EXIT_FAILURE);
            break;
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

            if (type_x->type_code != _basic_type && type_x->type_code != _array_type) {
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

            if (type_x->type_code != type_y->type_code || (type_x->type_code == _basic_type && type_x->data.basic_type->code != type_y->data.basic_type->code)) {
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
        case SIZE_EXPR: {
            struct SizeExpr *size_expr = node->data.size_expr;

            struct Type *type_x = check_node_type(size_expr->x, cur_scope, NULL, false);
            struct Type *t = CREATE_STRUCT_P(Type);
            t->type_code = _basic_type;
            t->data.basic_type = &basic_types[0];
            if (type_x && type_x->type_code == _array_type) return t;

            fprintf(
                stderr, "at %s:%d:%d:%d, illegal type for sizeof(), expect: array type\n", node->pos->filename, node->pos->off, node->pos->row, node->pos->col);
            exit(EXIT_FAILURE);
            break;
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
        case ARRAY_LIT: {
            struct ArrayLit *array_lit = node->data.array_lit;

            struct Type *t = CREATE_STRUCT_P(Type);
            t->type_code = _array_type;
            struct ArrayType *array_type = CREATE_STRUCT_P(ArrayType);
            struct Type      *ele_t = CREATE_STRUCT_P(Type);
            ele_t->data.basic_type = &basic_types[array_lit->elements[0]->data.basic_lit->lk];
            ele_t->type_code = _basic_type;
            array_type->ele_type = ele_t;
            t->data.array_type = array_type;
            return t;
        }
        case ARRAY_TYPE_DECL: return create_array_type(node->data.array_type_decl);
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