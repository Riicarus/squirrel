#ifndef AST_H
#define AST_H

#include "position.h"
#include "ast_type.h"
#include "token.h"
#include <stdbool.h>

enum NodeClass {
    CODE_FILE,
    // expr
    BASIC_LIT,
    ARRAY_LIT,
    ARR_EXPR,
    ASSIGN_EXPR,
    CALL_EXPR,
    INC_EXPR,
    INDEX_EXPR,
    NAME_EXPR,
    OPERATION,
    SIZE_EXPR,
    // ctrl
    BREAK_CTRL,
    CONTINUE_CTRL,
    RETURN_CTRL,
    IF_CTRL,
    ELSE_IF_CTRL,
    FOT_CTRL,
    // stmt
    EMPTY_STMT,
    CODE_BLOCK,
    // decl
    FUNC_DECL,
    FIELD_DECL,
    ARRAY_TYPE_DECL,
    BASIC_TYPE_DECL,
};

enum Operator {
    // binary
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
    ASSIGN,
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    LAND,
    LOR,
    // unary
    NOT,
    LNOT
};

struct {
    enum Operator op;
    unsigned int  priority;
} binary_op_map[] = {
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
    {ASSIGN, 0 },
    {EQ,     6 },
    {NE,     6 },
    {LT,     7 },
    {LE,     7 },
    {GT,     7 },
    {GE,     7 },
    {LAND,   2 },
    {LOR,    1 },
    {NOT,    11},
    {LNOT,   11}
};

// clang-format off
#define CMP_PRIORITY(op1, op2) binary_op_map[(op1)] > binary_op_map[(op2)]
#define NEW_AST_NODE(T) (struct T *)malloc(sizeof(struct T))
// clang-format on

// base ast node
struct AstNode {
    struct Position pos;
    bool     reachable;
    enum NodeClass class;
    struct AstType (*type_check_func)(struct AstNode node, struct AstType outer_type);
};

struct CodeFile {
    struct AstNode   base;
    struct AstNode **stmts;
    struct Scope    *scope;
};

// stmt
struct CodeBlock {
    struct AstNode   base;
    struct AstNode **stmts;
};

struct EmptyStmt {
    struct AstNode base;
};

// decl
struct FieldDecl {
    struct AstNode  base;
    struct AstNode *type_decl;
    struct AstNode *name_expr;
    struct AstNode *assign_expr;
};

struct FuncDecl {
    struct AstNode  base;
    struct AstNode *name_expr;
    struct AstNode *func_lit;
};

struct ArrayTypeDecl {
    struct AstNode  base;
    struct AstNode *ele_type;
    int             size;
};

struct BasicTypeDecl {
    struct AstNode   base;
    char            *name;
    struct BasicType type;
};

// expr
struct BasicLit {
    struct AstNode base;
    char          *value;
    enum LitKind   lk;
};

struct ArrayLit {
    struct AstNode   base;
    struct AstNode **elements;
};

struct ArrayExpr {
    struct AstNode       base;
    struct ArrayTypeDecl arr_type_decl;
    struct AstNode      *array_lit;
};

struct AssignExpr {
    struct AstNode  base;
    struct AstNode *x;
    struct AstNode *y;
};

struct CallExpr {
    struct AstNode   base;
    struct AstNode  *func;
    struct AstNode **params;
};

struct IncExpr {
    struct AstNode  base;
    struct AstNode *x;
    bool            pre_or_post;
    bool            inc_or_dec;
};

struct IndexExpr {
    struct AstNode  base;
    struct AstNode *x;
    struct AstNode *index;
};

struct NameExpr {
    struct AstNode base;
    char          *value;
};

struct Operation {
    struct AstNode  base;
    struct AstNode *x;
    struct AstNode *y;
    enum Operator   op;
};

struct SizeExpr {
    struct AstNode  base;
    struct AstNode *x;
};

// ctrl
struct BreakCtrl {
    struct AstNode base;
};

struct ContinueCtrl {
    struct AstNode base;
};

struct ReturnCtrl {
    struct AstNode  base;
    struct AstNode *ret_val;
};

struct IfCtrl {
    struct AstNode   base;
    struct AstNode  *cond;
    struct AstNode  *then;
    struct AstNode **else_ifs;
    struct AstNode  *_else;
};

struct ElseIfCtrl {
    struct AstNode  base;
    struct AstNode *cond;
    struct AstNode *then;
};

struct ForCtrl {
    struct AstNode   base;
    struct AstNode **inits;
    struct AstNode  *cond;
    struct AstNode **updates;
    struct AstNode  *body;
};

#endif