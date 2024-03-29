#include "global.h"
#include "syntax.h"
#include "position.h"
#include "lex.h"
#include "scope.h"
#include <string.h>

// use to store previous token info when doing token preview
static enum Token   prev_tk;                   // previous token
static enum LitKind prev_lk;                   // previous lit kind
static char         prev_lexeme[MAX_LINE_LEN]; // previous lexeme

static struct Scope s; // current scope

#define MAX_BAD_MSG_LEN 1024
char syntax_bad_msg[MAX_BAD_MSG_LEN] = {};

static void _debug(char *msg) {
    if (debug) printf("%s\n", msg);
}

static void _excp() {
    fprintf(stderr, "Syntaxer: %s\n", syntax_bad_msg);
}

static void _error_exit() {
    atexit(_excp);
    exit(EXIT_FAILURE);
}

static bool _is(enum Token _tk) {
    return tk == _tk;
}

static bool _contains(enum Token *tks, int size) {
    for (int i = 0; i < size; i++, tks++)
        if (tk == *tks) return true;

    return false;
}

static void _syntax_next() {
    if (tk == _eof) return;
    prev_tk = tk;
    prev_lk = lk;
    strcpy(prev_lexeme, lexeme);
    lex_next();
    if (debug) printf("at: %s:%d:%d:%d, lex read token: %d(%s)\n", filename, off, row, col, tk, tk_symbols[tk].symbol);
    if (_is(_comment)) _syntax_next();
}

static bool _got(enum Token _tk) {
    if (tk == _tk) {
        _syntax_next();
        return true;
    }

    return false;
}

static void _want(enum Token _tk) {
    if (!_got(_tk)) {
        sprintf(syntax_bad_msg, "expect %s, but get %s", tk_symbols[_tk].symbol, tk_symbols[tk].symbol);
        _error_exit();
    }
}

static void _list(char *context, struct AstNode *node, enum Token start, enum Token sep, enum Token close, update_list_func update_list_f) {
    if (sep != _comma && sep != _semi) {
        strcpy(syntax_bad_msg, "illegal sep argument for list method");
        _error_exit();
        return;
    }
    if (close != _rparen && close != _rbracket && close != _rbrace && close != _semi) {
        strcpy(syntax_bad_msg, "illegal close argument for list method");
        _error_exit();
        return;
    }

    if (start != -1) _want(start);

    bool done = false;
    // meet close, _eof or done to terminate
    while (close != tk && _eof != tk && !done) {
        done = update_list_f(node, tk, lk, lexeme);

        // should meet sep or close, sep is optional before close
        if (!_got(sep) && close != tk) {
            sprintf(syntax_bad_msg, "in %s, expect %s or %s, but get %s", context, tk_symbols[sep].symbol, tk_symbols[close].symbol, tk_symbols[tk].symbol);
            _error_exit();
            return;
        }
    }

    // we haven't consume close in while loop
    _want(close);
}

static void _ensure_ast_node_array_size(struct AstNode ***arr, unsigned int *size, unsigned int *cap) {
    if (*size == *cap) {
        struct AstNode **new_arr = calloc(*cap << 1, sizeof(struct AstNode *));
        if (new_arr == NULL) {
            strcpy(syntax_bad_msg, "no enough memory");
            _error_exit();
            return;
        }

        memcpy(new_arr, *arr, *size * sizeof(struct AstNode *));
        free(*arr);
        *arr = new_arr;
        *cap <<= 1;
    }
}

// ---------------------Pre Define---------------------

struct AstNode *_code_block();
struct AstNode *_expr();
struct AstNode *_stmt();

// ---------------------Expression---------------------

struct AstNode *_name_expr() {
    _debug("name expr");

    if (!_is(_ident)) {
        strcpy(syntax_bad_msg, "expect identifier");
        _error_exit();
        return NULL;
    }

    struct NameExpr *name_expr = CREATE_STRUCT_P(NameExpr);
    name_expr->value = calloc(strlen(lexeme) + 1, sizeof(char));
    if (!name_expr->value) {
        strcpy(syntax_bad_msg, "no enough memory");
        _error_exit();
        return NULL;
    }
    strcpy(name_expr->value, lexeme);

    struct AstNode *x = create_ast_node();
    x->pos = new_position(filename, off, row, col);
    x->class = NAME_EXPR;
    x->data.name_expr = name_expr;
    _syntax_next();
    return x;
}

/*
 * BasicLit     :   int_lit
 *              |   float_lit
 *              |   char_lit
 *              |   string_lit
 *              |   "true"
 *              |   "false"
 */
struct AstNode *_basic_lit() {
    _debug("basic lit");

    if (!_contains(basic_lit_tokens, BASIC_LIT_TOKEN_NUMBER)) {
        strcpy(syntax_bad_msg, "expect literal");
        _error_exit();
        return NULL;
    }

    struct BasicLit *lit = CREATE_STRUCT_P(BasicLit);
    // handle basic lit kind of keywords
    if (_is(_true) || _is(_false)) lit->lk = bool_lk;
    else lit->lk = lk;
    lit->value = calloc(strlen(lexeme) + 1, sizeof(char));
    if (!lit->value) {
        strcpy(syntax_bad_msg, "no enough memory");
        _error_exit();
        return NULL;
    }
    strcpy(lit->value, lexeme);

    struct AstNode *x = create_ast_node();
    x->class = BASIC_LIT;
    x->pos = new_position(filename, off, row, col);
    x->data.basic_lit = lit;
    _syntax_next();
    return x;
}

/*
 * Operand      :   Literal
 *              |   identifier
 *              |   "(" Expr ")"
 *
 * Literal      :   BasicLit
 */
struct AstNode *_operand() {
    _debug("operand");

    if (_contains(basic_lit_tokens, BASIC_LIT_TOKEN_NUMBER)) return _basic_lit();

    if (_is(_ident)) return _name_expr();

    if (_got(_lparen)) {
        struct AstNode *node = _expr();
        _want(_rparen);
        return node;
    }

    strcpy(syntax_bad_msg, "illegal expression");
    _error_exit();
    return NULL;
}

static bool _parse_func_call_params(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme) {
    struct CallExpr *call_expr = node->data.call_expr;
    _ensure_ast_node_array_size(&call_expr->params, &call_expr->param_size, &call_expr->param_cap);
    call_expr->params[call_expr->param_size++] = _expr();
    return false;
}

/*
 *PrimaryExpr   :   Operand
 *              |   PrimaryExpr Arguments
 *              |   PrimaryExpr { "++" | "--" }
 *              |   { "++" | "--" } PrimaryExpr
 *
 * Arguments    :   "(" [ { Expr "," }... ] ")"
 */
// x is operand
struct AstNode *_primary_expr(struct AstNode *x) {
    _debug("primary expr");

    // no operand, first come to primary expr
    if (x == NULL) {
        switch (tk) {
            case _inc:
            case _dec: {
                struct IncExpr *inc_expr = CREATE_STRUCT_P(IncExpr);
                inc_expr->is_inc = _is(_inc);
                inc_expr->is_pre = true;
                struct Position *pos = new_position(filename, off, row, col);
                _syntax_next();
                inc_expr->x = _primary_expr(NULL);

                struct AstNode *t = create_ast_node();
                t->class = INC_EXPR;
                t->pos = pos;
                t->data.inc_expr = inc_expr;
                x = t;
                break;
            }
            default: x = _operand();
        }
    }

    for (;;) {
        struct Position *pos = new_position(filename, off, row, col);

        switch (tk) {
            case _lparen: {
                _debug("func call");

                struct CallExpr *call_expr = CREATE_STRUCT_P(CallExpr);
                call_expr->params = calloc(8, sizeof(struct AstNode *));
                call_expr->param_size = 0;
                call_expr->param_cap = 8;
                call_expr->func_expr = x;
                struct AstNode *t = create_ast_node();
                t->class = CALL_EXPR;
                t->pos = pos;
                t->data.call_expr = call_expr;
                _list("func call params", t, _lparen, _comma, _rparen, _parse_func_call_params);
                x = t;
                break;
            }
            case _inc:
            case _dec: {
                struct IncExpr *inc_expr = CREATE_STRUCT_P(IncExpr);
                inc_expr->is_inc = _is(_inc);
                inc_expr->is_pre = false;
                inc_expr->x = x;
                _syntax_next();

                struct AstNode *t = create_ast_node();
                t->class = INC_EXPR;
                t->pos = pos;
                t->data.inc_expr = inc_expr;
                x = t;
                break;
            }
            default: return x;
        }
    }
}

/*
 *UnaryExpr     :   PrimaryExpr
 *              |   UnaryOp UnaryExpr
 *
 *UnaryOp       :   "!" | "~"
 */
struct AstNode *_unary_expr() {
    _debug("unary expr");

    if (_contains(unary_op_tokens, UNARY_OP_TOKEN_NUMBER)) {
        struct Position  *pos = new_position(filename, off, row, col);
        struct Operation *operation = CREATE_STRUCT_P(Operation);
        operation->op = (enum Operator)(tk - _eq);
        _syntax_next();
        operation->x = _unary_expr();

        struct AstNode *x = create_ast_node();
        x->class = OPERATION;
        x->pos = pos;
        x->data.operation = operation;
        return x;
    }

    return _primary_expr(NULL);
}

/*
 * BinaryExpr   :   UnaryExpr
 *              |   BinaryExpr BinaryOp BinaryExpr
 *
 * BinaryOp     :   "==" | "!=" | "<" | "<=" | ">" | ">="
 *              |   "+"| "-" | "*" | "/" | "%" | "|" | "^" | "<<" | ">>"
 *              |   "="
 *              |   "&&" | "||"
 */
struct AstNode *_binary_expr(struct AstNode *x, int p) {
    _debug("binary expr");

    if (x == NULL) x = _unary_expr();

    enum Operator    op = -1;
    struct Position *pos = new_position(filename, off, row, col);
    while ((_contains(unary_op_tokens, UNARY_OP_TOKEN_NUMBER) || _contains(binary_op_tokens, BINARY_OP_TOKEN_NUMBER)) &&
           op_priority_map[(op = (enum Operator)(tk - _eq))].priority > p) {
        struct Operation *operation = CREATE_STRUCT_P(Operation);
        operation->op = op;
        operation->x = x;
        int tp = op_priority_map[op].priority;
        _syntax_next();
        operation->y = _binary_expr(NULL, tp);
        struct AstNode *t = create_ast_node();
        t->class = OPERATION;
        t->pos = pos;
        t->data.operation = operation;
        x = t;
    }

    return x;
}

/*
 * Expr     :   BinaryExpr
 */
struct AstNode *_expr() {
    _debug("expr");

    return _binary_expr(NULL, -1);
}

// ---------------------Declaration---------------------

/*
 * BasicTypeDecl    :   "int"
 *                  |   "float"
 *                  |   "bool"
 *                  |   "char"
 *                  |   "string"
 *                  |   "void"
 */
struct AstNode *_basic_type_decl() {
    _debug("basic type decl");

    struct BasicTypeDecl *type_decl = CREATE_STRUCT_P(BasicTypeDecl);
    if (_contains(basic_type_tokens, BASIC_TYPE_TOKEN_NUMBER)) type_decl->tk = tk;
    else {
        strcpy(syntax_bad_msg, "illegal basic type");
        _error_exit();
        return NULL;
    }
    struct AstNode *x = create_ast_node();
    x->class = BASIC_TYPE_DECL;
    x->pos = new_position(filename, off, row, col);
    x->data.basic_type_decl = type_decl;
    _syntax_next();
    return x;
}
/*
 * TypeDecl     :   BasicTypeDecl
 */
struct AstNode *_type_decl() {
    _debug("type decl");
    return _basic_type_decl();
}

/*
 * FieldDecl    :   TypeDecl identifier [ "=" Expr ]
 */
struct AstNode *_field_decl() {
    _debug("field decl");

    struct Position  *pos = new_position(filename, off, row, col);
    struct FieldDecl *field_decl = CREATE_STRUCT_P(FieldDecl);
    field_decl->type_decl = _type_decl();
    field_decl->name_expr = _name_expr();
    if (_got(_assign)) {
        struct Operation *operation = CREATE_STRUCT_P(Operation);
        operation->x = field_decl->name_expr;
        operation->op = ASSIGN;
        operation->y = _expr();
        struct AstNode *t = create_ast_node();
        t->class = OPERATION;
        t->pos = pos;
        t->data.operation = operation;
        field_decl->assign_expr = t;
    }

    struct AstNode *x = create_ast_node();
    x->class = FIELD_DECL;
    x->pos = pos;
    x->data.field_decl = field_decl;
    return x;
}

static bool _parse_func_param_decl_elements(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme) {
    struct FuncDecl *func_decl = node->data.func_decl;
    _ensure_ast_node_array_size(&func_decl->param_decls, &func_decl->param_size, &func_decl->param_cap);
    func_decl->param_decls[func_decl->param_size++] = _field_decl();
    return false;
}

/*
 * FuncDecl     :   "func" identifier "(" [ { FieldDecl "," }... ] ")" TypeDecl CodeBlock
 */
struct AstNode *_func_decl() {
    _debug("func decl");
    _want(_func);
    struct FuncDecl *func_decl = CREATE_STRUCT_P(FuncDecl);
    func_decl->name_expr = _name_expr();
    func_decl->param_decls = calloc(8, sizeof(struct AstNode *));
    func_decl->param_size = 0;
    func_decl->param_cap = 8;
    struct AstNode *x = create_ast_node();
    x->class = FUNC_DECL;
    x->pos = new_position(filename, off, row, col);
    x->data.func_decl = func_decl;
    _list("func param list decl", x, _lparen, _comma, _rparen, _parse_func_param_decl_elements);
    func_decl->ret_type_decl = _type_decl();
    func_decl->body = _code_block();
    return x;
}

/*
 * Decl     :   FieldDecl
 *          |   FuncDecl
 */
struct AstNode *_decl() {
    _debug("decl");
    if (_is(_func)) return _func_decl();
    else return _field_decl();
}

// ---------------------Control---------------------
/*
 * Break    :   "break"
 */
struct AstNode *_break_ctrl() {
    _debug("break");

    struct BreakCtrl *break_ctrl = CREATE_STRUCT_P(BreakCtrl);
    struct AstNode   *x = create_ast_node();
    x->class = BREAK_CTRL;
    x->pos = new_position(filename, off, row, col);
    x->data.break_ctrl = break_ctrl;
    _syntax_next();
    return x;
}

/*
 * Continue     :   "continue"
 */
struct AstNode *_continue_ctrl() {
    _debug("continue");

    struct ContinueCtrl *continue_ctrl = CREATE_STRUCT_P(ContinueCtrl);
    struct AstNode      *x = create_ast_node();
    x->class = CONTINUE_CTRL;
    x->pos = new_position(filename, off, row, col);
    x->data.continue_ctrl = continue_ctrl;
    _syntax_next();
    return x;
}

/*
 *Return    :   "return" [ Expr ]
 */
struct AstNode *_return_ctrl() {
    _debug("return");

    struct ReturnCtrl *return_ctrl = CREATE_STRUCT_P(ReturnCtrl);
    struct Position   *pos = new_position(filename, off, row, col);
    struct AstNode    *x = create_ast_node();
    x->class = RETURN_CTRL;
    x->pos = pos;
    x->data.return_ctrl = return_ctrl;
    _syntax_next();

    if (_is(_semi)) return x;

    return_ctrl->ret_val = _expr();
    return x;
}

/*
 * If           :   "if" "(" Expr ")" CodeBlock [ Else ]
 *
 * Else         :   [ ElseIf... ] [ EndElse ]
 *
 * ElseIf       :   "elseif" "(" Expr ")" CodeBlock
 *
 * EndElse      :   "else" CodeBlock
 */
struct AstNode *_if_ctrl() {
    _debug("if");

    struct Position *pos = new_position(filename, off, row, col);
    _want(_if);
    struct IfCtrl  *if_ctrl = CREATE_STRUCT_P(IfCtrl);
    struct AstNode *x = create_ast_node();
    x->class = IF_CTRL;
    x->pos = pos;
    x->data.if_ctrl = if_ctrl;
    _want(_lparen);
    if_ctrl->cond = _expr();
    _want(_rparen);

    if_ctrl->then = _code_block();

    if_ctrl->else_ifs = calloc(8, sizeof(struct AstNode *));
    if_ctrl->else_if_size = 0;
    if_ctrl->else_if_cap = 8;
    while (_got(_elseif)) {
        pos = new_position(filename, off, row, col);
        struct ElseIfCtrl *else_if_ctrl = CREATE_STRUCT_P(ElseIfCtrl);
        _want(_lparen);
        else_if_ctrl->cond = _expr();
        _want(_rparen);

        else_if_ctrl->then = _code_block();

        struct AstNode *t = create_ast_node();
        t->class = ELSE_IF_CTRL;
        t->pos = pos;
        t->data.else_if_ctrl = else_if_ctrl;
        if_ctrl->else_ifs[if_ctrl->else_if_size++] = t;
    }

    if (_got(_else)) if_ctrl->_else = _code_block();

    return x;
}

static bool _parse_for_inits(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme) {
    struct ForCtrl *for_ctrl = node->data.for_ctrl;
    _ensure_ast_node_array_size(&for_ctrl->inits, &for_ctrl->inits_size, &for_ctrl->inits_cap);

    if (_contains(basic_type_tokens, BASIC_TYPE_TOKEN_NUMBER)) for_ctrl->inits[for_ctrl->inits_size++] = _field_decl();
    else for_ctrl->inits[for_ctrl->inits_size++] = _expr();
    return false;
}

static bool _parse_for_updates(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme) {
    struct ForCtrl *for_ctrl = node->data.for_ctrl;
    _ensure_ast_node_array_size(&for_ctrl->updates, &for_ctrl->updates_size, &for_ctrl->updates_cap);
    for_ctrl->updates[for_ctrl->updates_size++] = _expr();
    return false;
}

/*
 * For          :   "for" "(" [ ForInit ] ";" [ Expr ] ";" [ ForUpdate ] ")" CodeBlock
 *
 * ForInit      :   { { FieldDecl | Expr } "," }...
 *
 * ForUpdate    :   { Expr "," }...
 */
struct AstNode *_for_ctrl() {
    _debug("for");

    struct Position *pos = new_position(filename, off, row, col);
    _want(_for);

    struct ForCtrl *for_ctrl = CREATE_STRUCT_P(ForCtrl);
    struct AstNode *x = create_ast_node();
    x->class = FOR_CTRL;
    x->pos = pos;
    x->data.for_ctrl = for_ctrl;

    _want(_lparen);
    if (!_got(_semi)) {
        for_ctrl->inits = calloc(8, sizeof(struct AstNode *));
        for_ctrl->inits_size = 0;
        for_ctrl->inits_cap = 8;
        _list("for inits", x, -1, _comma, _semi, _parse_for_inits);
    }

    if (!_got(_semi)) {
        for_ctrl->cond = _expr();
        _want(_semi);
    }

    if (!_got(_rparen)) {
        for_ctrl->updates = calloc(8, sizeof(struct AstNode *));
        for_ctrl->updates_size = 0;
        for_ctrl->updates_cap = 8;
        _list("for updates", x, -1, _comma, _rparen, _parse_for_updates);
    }

    for_ctrl->body = _code_block();

    return x;
}

struct AstNode *_ctrl() {
    _debug("ctrl");

    switch (tk) {
        case _break: return _break_ctrl();
        case _continue: return _continue_ctrl();
        case _return: return _return_ctrl();
        case _if: return _if_ctrl();
        case _for: return _for_ctrl();
        default: {
            strcpy(syntax_bad_msg, "illegal statement");
            _error_exit();
            return NULL;
        }
    }
}

// ---------------------Program---------------------

struct AstNode *_empty_stmt() {
    _debug("empty stmt");

    struct EmptyStmt *empty_stmt = CREATE_STRUCT_P(EmptyStmt);
    struct AstNode   *x = create_ast_node();
    x->class = EMPTY_STMT;
    x->pos = new_position(filename, off, row, col);
    x->data.empty_stmt = empty_stmt;
    return x;
}

/*
 * Statement    :   Decl
 *              |   Expr
 *              |   Ctrl
 *              |   CodeBlock
 */
struct AstNode *_stmt() {
    _debug("statement");

    struct AstNode *x;
    if (_contains(basic_type_tokens, BASIC_TYPE_TOKEN_NUMBER) || _is(_func)) x = _decl();
    else if (_contains(expr_start_tokens, EXPR_START_TOKEN_NUMBER)) x = _expr();
    else if (_contains(ctrl_start_tokens, CTRL_START_TOKEN_NUMBER)) x = _ctrl();
    else if (_is(_lbrace)) x = _code_block();
    else if (_is(_semi)) x = _empty_stmt();
    else if (_is(_eof) || _is(_rbrace)) x = NULL;
    else {
        strcpy(syntax_bad_msg, "illegal statement");
        _error_exit();
        return NULL;
    }

    return x;
}

static bool _parse_code_block_statements(struct AstNode *node, const enum Token _tk, const enum LitKind _lk, const char *lexeme) {
    struct CodeBlock *code_block = node->data.code_block;
    _ensure_ast_node_array_size(&code_block->stmts, &code_block->size, &code_block->cap);
    code_block->stmts[code_block->size++] = _stmt();
    return false;
}

/*
 * CodeBlock    :   "{" [ { Statement ";" }... ] "}"
 */
struct AstNode *_code_block() {
    _debug("code block");

    struct CodeBlock *code_block = CREATE_STRUCT_P(CodeBlock);
    code_block->stmts = calloc(8, sizeof(struct AstNode *));
    code_block->size = 0;
    code_block->cap = 8;
    struct AstNode *x = create_ast_node();
    x->class = CODE_BLOCK;
    x->pos = new_position(filename, off, row, col);
    x->data.code_block = code_block;

    _list("code block statements", x, _lbrace, _semi, _rbrace, _parse_code_block_statements);

    return x;
}

/*
 * Program      :   CodeBlock
 */
struct AstNode *parse() {
    _debug("code file");

    // let lex read the first tk
    _syntax_next();

    struct Position *pos = new_position(filename, off, row, col);
    struct CodeFile *code_file = CREATE_STRUCT_P(CodeFile);
    code_file->code_block = _code_block();
    struct AstNode *x = create_ast_node();
    x->class = CODE_FILE;
    x->pos = pos;
    x->data.code_file = code_file;

    return x;
}