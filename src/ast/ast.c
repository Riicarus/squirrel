#include "global.h"
#include "type.h"
#include "ast.h"

struct OpPriority op_priority_map[] = {
    {EQ,     6 },
    {NE,     6 },
    {LT,     7 },
    {LE,     7 },
    {GT,     7 },
    {GE,     7 },
    {ADD,    9 },
    {SUB,    9 },
    {MUL,    10},
    {QUO,    10},
    {REM,    10},
    {AND,    5 },
    {OR,     3 },
    {XOR,    4 },
    {SHL,    8 },
    {SHR,    8 },
    {LAND,   2 },
    {LOR,    1 },
    {NOT,    11},
    {LNOT,   11},
    {ASSIGN, 0 }
};

static int _id = 0;

struct AstNode *create_ast_node() {
    struct AstNode *n = CREATE_STRUCT_P(AstNode);
    n->id = _id++;
    return n;
}

void _indent(int n) {
    for (int i = 0; i < n - 1; i++) printf("|  ");
    if (n > 0) printf("|--");
}

void print_node(struct AstNode *node, int level, char *hint) {
    _indent(level);
    if (hint) printf("%s:  ", hint);

    if (!node) {
        fprintf(stderr, "print_node(), null ast node\n");
        exit(EXIT_FAILURE);
    }

    struct Position *pos = node->pos;
    char            *access_msg = node->reachable ? "+" : "-";
    switch (node->class) {
        case CODE_FILE: {
            printf("(%s)[code file]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            print_node(node->data.code_file->code_block, level + 1, NULL);
            break;
        }
        case CODE_BLOCK: {
            printf("(%s)[code block]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct CodeBlock *code_block = node->data.code_block;
            for (int i = 0; i < code_block->size; i++) print_node(code_block->stmts[i], level + 1, NULL);
            break;
        }
        case EMPTY_STMT: {
            printf("(%s)[empty stmt]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case FIELD_DECL: {
            printf("(%s)[field decl]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct FieldDecl *field_decl = node->data.field_decl;
            print_node(field_decl->type_decl, level + 1, "type");
            print_node(field_decl->name_expr, level + 1, "name");
            if (field_decl->assign_expr) print_node(field_decl->assign_expr, level + 1, "init");
            break;
        }
        case FUNC_DECL: {
            printf("(%s)[func decl]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct FuncDecl *func_decl = node->data.func_decl;
            print_node(func_decl->name_expr, level + 1, "name");
            for (int i = 0; i < func_decl->param_size; i++) print_node(func_decl->param_decls[i], level + 1, "func param");
            print_node(func_decl->ret_type_decl, level + 1, "return");
            print_node(func_decl->body, level + 1, "body");
            break;
        }
        case ARRAY_TYPE_DECL: {
            printf("(%s)[array type decl]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct ArrayTypeDecl *arr_type_decl = node->data.array_type_decl;
            print_node(arr_type_decl->ele_type_decl, level + 1, "ele_type");
            break;
        }
        case BASIC_TYPE_DECL: {
            printf("(%s)[basic type decl]#%d<%s:%d:%d:%d>: %s\n",
                   access_msg,
                   node->id,
                   pos->filename,
                   pos->off,
                   pos->row,
                   pos->col,
                   basic_types[node->data.basic_type_decl->tk].name);
            break;
        }
        case BASIC_LIT: {
            struct BasicLit *basic_lit = node->data.basic_lit;
            printf("(%s)[basic lit]#%d<%s:%d:%d:%d>: %s(%s)\n",
                   access_msg,
                   node->id,
                   pos->filename,
                   pos->off,
                   pos->row,
                   pos->col,
                   basic_lit->value,
                   lit_kind_symbols[basic_lit->lk].symbol);
            break;
        }
        case ARRAY_LIT: {
            printf("(%s)[array lit]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct ArrayLit *array_lit = node->data.array_lit;
            for (int i = 0; i < array_lit->size; i++) print_node(array_lit->elements[i], level + 1, "element");
            break;
        }
        case CALL_EXPR: {
            printf("(%s)[call expr]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct CallExpr *call_expr = node->data.call_expr;
            print_node(call_expr->func_expr, level, "func");
            for (int i = 0; i < call_expr->param_size; i++) print_node(call_expr->params[i], level + 1, "param");
            break;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = node->data.inc_expr;
            printf("(%s)[inc expr]#%d<%s:%d:%d:%d>: %s, %s\n",
                   access_msg,
                   node->id,
                   pos->filename,
                   pos->off,
                   pos->row,
                   pos->col,
                   inc_expr->is_inc ? "inc" : "dec",
                   inc_expr->is_pre ? "pre" : "post");
            print_node(inc_expr->x, level + 1, NULL);
            break;
        }
        case INDEX_EXPR: {
            printf("(%s)[index expr]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct IndexExpr *index_expr = node->data.index_expr;
            print_node(index_expr->x, level + 1, "x");
            print_node(index_expr->index, level + 1, "index");
            break;
        }
        case NAME_EXPR: {
            struct NameExpr *name_expr = node->data.name_expr;
            printf("(%s)[name expr]#%d<%s:%d:%d:%d>: %s\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col, name_expr->value);
            break;
        }
        case OPERATION: {
            struct Operation *operation = node->data.operation;
            printf("(%s)[operation]#%d<%s:%d:%d:%d>: %s\n",
                   access_msg,
                   node->id,
                   pos->filename,
                   pos->off,
                   pos->row,
                   pos->col,
                   tk_symbols[(enum Token)(operation->op + _eq)].symbol);
            print_node(operation->x, level + 1, "x");
            if (operation->y) print_node(operation->y, level + 1, "y");
            break;
        }
        case SIZE_EXPR: {
            printf("(%s)[size expr]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct SizeExpr *size_expr = node->data.size_expr;
            print_node(size_expr->x, level + 1, NULL);
            break;
        }
        case BREAK_CTRL: {
            printf("(%s)[break]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case CONTINUE_CTRL: {
            printf("(%s)[continue]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case RETURN_CTRL: {
            printf("(%s)[return]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct ReturnCtrl *return_ctrl = node->data.return_ctrl;
            if (return_ctrl->ret_val) print_node(return_ctrl->ret_val, level + 1, "return value");
            break;
        }
        case IF_CTRL: {
            printf("(%s)[if]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct IfCtrl *if_ctrl = node->data.if_ctrl;
            print_node(if_ctrl->cond, level + 1, "cond");
            print_node(if_ctrl->then, level + 1, "then");
            for (int i = 0; i < if_ctrl->else_if_size; i++) print_node(if_ctrl->else_ifs[i], level + 1, "elseif");
            if (if_ctrl->_else) print_node(if_ctrl->_else, level + 1, "else");
            break;
        }
        case ELSE_IF_CTRL: {
            printf("(%s)[elseif]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct ElseIfCtrl *else_if_ctrl = node->data.else_if_ctrl;
            print_node(else_if_ctrl->cond, level + 1, "cond");
            print_node(else_if_ctrl->then, level + 1, "then");
            break;
        }
        case FOR_CTRL: {
            printf("(%s)[for]#%d<%s:%d:%d:%d>\n", access_msg, node->id, pos->filename, pos->off, pos->row, pos->col);
            struct ForCtrl *for_ctrl = node->data.for_ctrl;
            for (int i = 0; i < for_ctrl->inits_size; i++) print_node(for_ctrl->inits[i], level + 1, "init");
            if (for_ctrl->cond) print_node(for_ctrl->cond, level + 1, "cond");
            for (int i = 0; i < for_ctrl->updates_size; i++) print_node(for_ctrl->updates[i], level + 1, "update");
            print_node(for_ctrl->body, level + 1, "body");
            break;
        }
        default: fprintf(stderr, "print_node(), invalid ast node class\n");
    }
}