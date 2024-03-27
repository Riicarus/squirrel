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
                sprintf(name, "anonymous#%d", node->id);
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

struct Type *check_node_type(struct AstNode *node) {
    if (!node) {
        fprintf(stderr, "check_node_type(), null ast node\n");
        exit(EXIT_FAILURE);
    }

    switch (node->class) {
        case CODE_FILE:
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
        case IF_CTRL:
        case ELSE_IF_CTRL:
        case FOR_CTRL:
        case EMPTY_STMT:
        case CODE_BLOCK:
        case FUNC_DECL:
        case FIELD_DECL:
        case ARRAY_TYPE_DECL:
        case BASIC_TYPE_DECL: break;
        default: fprintf(stderr, "check_node_type(), invalid ast node class\n");
    }

    return NULL;
}