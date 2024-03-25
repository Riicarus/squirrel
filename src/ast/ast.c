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
    for (int i = 0; i < n; i++) printf("  ");
}

void print_node(struct AstNode *self, int level, char *hint) {
    if (!self) {
        fprintf(stderr, "null ast node");
    }

    _indent(level);
    if (hint) printf("%s:  ", hint);
    struct Position *pos = self->pos;
    switch (self->class) {
        case CODE_FILE: {
            printf("code file<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            print_node(self->data.code_file->code_block, level + 1, NULL);
            break;
        }
        case CODE_BLOCK: {
            printf("code block<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct CodeBlock *code_block = self->data.code_block;
            for (int i = 0; i < code_block->size; i++) print_node(code_block->stmts[i], level + 1, NULL);
            break;
        }
        case EMPTY_STMT: {
            printf("empty stmt<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            break;
        }
        case FIELD_DECL: {
            printf("field decl<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct FieldDecl *field_decl = self->data.field_decl;
            print_node(field_decl->type_decl, level + 1, "field type");
            print_node(field_decl->name_expr, level + 1, "field name");
            print_node(field_decl->assign_expr, level + 1, "field init");
            break;
        }
        case FUNC_DECL: {
            printf("func decl<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct FuncDecl *func_decl = self->data.func_decl;
            print_node(func_decl->name_expr, level + 1, "func name");
            for (int i = 0; i < func_decl->param_size; i++) print_node(func_decl->param_decls[i], level + 1, "func param");
            print_node(func_decl->ret_type_decl, level + 1, "return type");
            print_node(func_decl->body, level + 1, "func body");
            break;
        }
        case ARRAY_TYPE_DECL: {
            printf("array type decl<%s:%d:%d:%d>\n", pos->filename, pos->off, pos->row, pos->col);
            struct ArrayTypeDecl *arr_type_decl = self->data.array_type_decl;
            print_node(arr_type_decl->ele_type_decl, level + 1, "element type");
            break;
        }
        case BASIC_TYPE_DECL: {
            printf("basic type decl<%s:%d:%d:%d>: %s\n", pos->filename, pos->off, pos->row, pos->col, self->data.basic_type_decl->symbol);
        }
        case BASIC_LIT: {
            struct BasicLit *basic_lit = self->data.basic_lit;
            printf("basic lit<%s:%d:%d:%d>: %s(%s)\n", pos->filename, pos->off, pos->row, pos->col, basic_lit->value, lit_kind_symbols[basic_lit->lk].symbol);
        }
        default: fprintf(stderr, "invalid ast node class");
    }
}