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

void _indent(int n) {
    for (int i = 0; i < n - 1; i++) printf("|  ");
    if (n > 0) printf("|--");
}

void print_node(struct AstNode *self, int level, char *hint) {
    _indent(level);
    if (hint) printf("%s:  ", hint);

    if (!self) {
        fprintf(stderr, "null ast node\n");
        exit(EXIT_FAILURE);
    }

    struct Position *pos = self->pos;
    switch (self->class) {
        case CODE_FILE: {
            printf("[code file]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            print_node(self->data.code_file->code_block, level + 1, NULL);
            break;
        }
        case CODE_BLOCK: {
            printf("[code block]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct CodeBlock *code_block = self->data.code_block;
            for (int i = 0; i < code_block->size; i++) print_node(code_block->stmts[i], level + 1, NULL);
            break;
        }
        case EMPTY_STMT: {
            printf("[empty stmt]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case FIELD_DECL: {
            printf("[field decl]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct FieldDecl *field_decl = self->data.field_decl;
            print_node(field_decl->type_decl, level + 1, "type");
            print_node(field_decl->name_expr, level + 1, "name");
            if (field_decl->assign_expr) print_node(field_decl->assign_expr, level + 1, "init");
            break;
        }
        case FUNC_DECL: {
            printf("[func decl]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct FuncDecl *func_decl = self->data.func_decl;
            print_node(func_decl->name_expr, level + 1, "name");
            for (int i = 0; i < func_decl->param_size; i++) print_node(func_decl->param_decls[i], level + 1, "func param");
            print_node(func_decl->ret_type_decl, level + 1, "return");
            print_node(func_decl->body, level + 1, "body");
            break;
        }
        case ARRAY_TYPE_DECL: {
            printf("[array type decl]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ArrayTypeDecl *arr_type_decl = self->data.array_type_decl;
            print_node(arr_type_decl->ele_type_decl, level + 1, "ele_type");
            break;
        }
        case BASIC_TYPE_DECL: {
            printf("[basic type decl]<%s:%d:%d:%d>: %s\n", pos->filename, pos->off, pos->row, pos->col, self->data.basic_type_decl->symbol);
            break;
        }
        case BASIC_LIT: {
            struct BasicLit *basic_lit = self->data.basic_lit;
            printf("[basic lit]<%s:%d:%d:%d>: %s(%s)\n", pos->filename, pos->off, pos->row, pos->col, basic_lit->value, lit_kind_symbols[basic_lit->lk].symbol);
            break;
        }
        case ARRAY_LIT: {
            printf("[array lit]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ArrayLit *array_lit = self->data.array_lit;
            for (int i = 0; i < array_lit->size; i++) print_node(array_lit->elements[i], level + 1, "element");
            break;
        }
        case CALL_EXPR: {
            printf("[call expr]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct CallExpr *call_expr = self->data.call_expr;
            print_node(call_expr->func_expr, level, "func");
            for (int i = 0; i < call_expr->param_size; i++) print_node(call_expr->params[i], level + 1, "param");
            break;
        }
        case INC_EXPR: {
            struct IncExpr *inc_expr = self->data.inc_expr;
            printf("[inc expr]<%s:%d:%d:%d>: %s, %s\n",
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
            printf("[index expr]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct IndexExpr *index_expr = self->data.index_expr;
            print_node(index_expr->x, level + 1, "x");
            print_node(index_expr->index, level + 1, "index");
            break;
        }
        case NAME_EXPR: {
            struct NameExpr *name_expr = self->data.name_expr;
            printf("[name expr]<%s:%d:%d:%d>: %s\n", pos->filename, pos->off, pos->row, pos->col, name_expr->value);
            break;
        }
        case OPERATION: {
            struct Operation *operation = self->data.operation;
            printf("[operation]<%s:%d:%d:%d>: %s\n", pos->filename, pos->off, pos->row, pos->col, tk_symbols[(enum Token)(operation->op + _eq)].symbol);
            print_node(operation->x, level + 1, "x");
            if (operation->y) print_node(operation->y, level + 1, "y");
            break;
        }
        case SIZE_EXPR: {
            printf("[size expr]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct SizeExpr *size_expr = self->data.size_expr;
            print_node(size_expr->x, level + 1, NULL);
            break;
        }
        case BREAK_CTRL: {
            printf("[break]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case CONTINUE_CTRL: {
            printf("[continue]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case RETURN_CTRL: {
            printf("[return]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ReturnCtrl *return_ctrl = self->data.return_ctrl;
            if (return_ctrl->ret_val) print_node(return_ctrl->ret_val, level + 1, "return value");
            break;
        }
        case IF_CTRL: {
            printf("[if]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct IfCtrl *if_ctrl = self->data.if_ctrl;
            print_node(if_ctrl->cond, level + 1, "cond");
            print_node(if_ctrl->then, level + 1, "then");
            for (int i = 0; i < if_ctrl->else_if_size; i++) print_node(if_ctrl->else_ifs[i], level + 1, "elseif");
            if (if_ctrl->_else) print_node(if_ctrl->_else, level + 1, "else");
            break;
        }
        case ELSE_IF_CTRL: {
            printf("[elseif]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ElseIfCtrl *else_if_ctrl = self->data.else_if_ctrl;
            print_node(else_if_ctrl->cond, level + 1, "cond");
            print_node(else_if_ctrl->then, level + 1, "then");
            break;
        }
        case FOR_CTRL: {
            printf("[for]<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ForCtrl *for_ctrl = self->data.for_ctrl;
            for (int i = 0; i < for_ctrl->inits_size; i++) print_node(for_ctrl->inits[i], level + 1, "init");
            if (for_ctrl->cond) print_node(for_ctrl->cond, level + 1, "cond");
            for (int i = 0; i < for_ctrl->updates_size; i++) print_node(for_ctrl->updates[i], level + 1, "update");
            print_node(for_ctrl->body, level + 1, "body");
            break;
        }
        default: fprintf(stderr, "invalid ast node class\n");
    }
}