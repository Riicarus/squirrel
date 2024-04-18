#ifndef AST_H
#define AST_H

#include "position.h"
#include "token.h"
#include <stdbool.h>

enum NodeClass {
    CODE_FILE,
    // expr
    BASIC_LIT,
    CALL_EXPR,
    INC_EXPR,
    NAME_EXPR,
    OPERATION,
    // ctrl
    BREAK_CTRL,
    CONTINUE_CTRL,
    RETURN_CTRL,
    IF_CTRL,
    ELSE_IF_CTRL,
    FOR_CTRL,
    // stmt
    EMPTY_STMT,
    CODE_BLOCK,
    // decl
    FUNC_DECL,
    FIELD_DECL,
    BASIC_TYPE_DECL,
};

enum Operator {
    // binary
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    ADD,
    SUB,
    MUL,
    QUO,
    REM,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    LAND,
    LOR,
    // unary
    NOT,
    LNOT,
    // assign
    ASSIGN
};

struct OpPriority {
    enum Operator op;
    int           priority;
};

extern struct OpPriority op_priority_map[];

#define CMP_PRIORITY(op1, op2) binary_op_map[(op1)] > binary_op_map[(op2)]

// base ast node
struct AstNode {
    int              id;
    struct Position *pos;
    bool             reachable;
    struct Scope    *scope;

    enum NodeClass class;
    union {
        struct CodeFile      *code_file;
        struct CodeBlock     *code_block;
        struct EmptyStmt     *empty_stmt;
        struct FieldDecl     *field_decl;
        struct FuncDecl      *func_decl;
        struct BasicTypeDecl *basic_type_decl;
        struct BasicLit      *basic_lit;
        struct CallExpr      *call_expr;
        struct IncExpr       *inc_expr;
        struct NameExpr      *name_expr;
        struct Operation     *operation;
        struct BreakCtrl     *break_ctrl;
        struct ContinueCtrl  *continue_ctrl;
        struct ReturnCtrl    *return_ctrl;
        struct IfCtrl        *if_ctrl;
        struct ElseIfCtrl    *else_if_ctrl;
        struct ForCtrl       *for_ctrl;
    } data;
};

struct AstNode *create_ast_node();

// stmt
struct CodeBlock {
    struct AstNode **stmts;
    unsigned int     size;
    unsigned int     cap;
};

struct CodeFile {
    struct AstNode *code_block;
};

struct EmptyStmt { };

// decl
struct FieldDecl {
    struct AstNode *type_decl;
    struct AstNode *name_expr;
    struct AstNode *assign_expr;
};

struct FuncDecl {
    struct AstNode  *name_expr;
    struct AstNode **param_decls;
    unsigned int     param_size;
    unsigned int     param_cap;
    struct AstNode  *ret_type_decl;
    struct AstNode  *body;
};

struct BasicTypeDecl {
    enum Token tk;
};

// expr
struct BasicLit {
    char        *value;
    enum LitKind lk;
};

struct CallExpr {
    struct AstNode  *func_expr;
    struct AstNode **params;
    unsigned int     param_size;
    unsigned int     param_cap;
};

struct IncExpr {
    struct AstNode *x;
    bool            is_pre;
    bool            is_inc;
};

struct NameExpr {
    char *value;
};

struct Operation {
    struct AstNode *x;
    struct AstNode *y;
    enum Operator   op;
};

// ctrl
struct BreakCtrl { };

struct ContinueCtrl { };

struct ReturnCtrl {
    struct AstNode *ret_val;
};

struct IfCtrl {
    struct AstNode  *cond;
    struct AstNode  *then;
    struct AstNode **else_ifs;
    unsigned int     else_if_size;
    unsigned int     else_if_cap;
    struct AstNode  *_else;
};

struct ElseIfCtrl {
    struct AstNode *cond;
    struct AstNode *then;
};

struct ForCtrl {
    struct AstNode **inits;
    unsigned int     inits_size;
    unsigned int     inits_cap;
    struct AstNode  *cond;
    struct AstNode **updates;
    unsigned int     updates_size;
    unsigned int     updates_cap;
    struct AstNode  *body;
};

void print_node(struct AstNode *node, int level, char *hint);

#endif