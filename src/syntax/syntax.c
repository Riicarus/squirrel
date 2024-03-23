#include "syntax.h"
#include "scope.h"
#include <string.h>

// use to store previous token info when doing token preview
static enum Token   prev_tk;                   // previous token
static enum LitKind prev_lk;                   // previous lit kind
static char         prev_lexeme[MAX_LINE_LEN]; // previous lexeme

static struct Scope s; // current scope

char *syntax_bad_msg;

static void _excp() {
    printf("Syntaxer: %s", syntax_bad_msg);
}

static void _error_exit() {
    atexit(_excp);
    exit(EXIT_FAILURE);
}

static void _syntax_next() {
    prev_tk = tk;
    lex_next();
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

static struct Position _list(char *context, enum Token sep, enum Token close, update_list_func update_list_f) {
    if (sep != _comma && sep != _semi) {
        syntax_bad_msg = "illegal sep argument for list method";
        _error_exit();
    }
    if (close != _rparen && close != _rbracket && close != _rbrace) {
        syntax_bad_msg = "illegal close argument for list method";
        _error_exit();
    }

    bool done = false;
    // meet close, _eof or done to terminate
    while (close != tk && _eof != tk && !done) {
        done = update_list_f(tk);

        // should meet sep or close, sep is optional before close
        if (!_got(sep) && close != tk) {
            sprintf(syntax_bad_msg,
                    "in %s, expect %s or %s, but get %s",
                    context,
                    tk_symbols[sep].symbol,
                    tk_symbols[close].symbol,
                    tk_symbols[tk].symbol);
            _error_exit();
        }
    }

    struct Position pos = (struct Position){filename, off, row, col};

    // we haven't consume close in while loop
    _want(close);

    return pos;
}

static bool _is(enum Token _tk) {
    return tk == _tk;
}

static bool _contains(enum Token *tks, int size) {
    for (int i = 0; i < size; i++, tks++)
        if (tk == *tks) return true;

    return false;
}

static void _next() {
    if (tk == _eof) return;
    prev_tk = tk;
    prev_lk = lk;
    memcpy(prev_lexeme, lexeme, strlen(lexeme));
    lex_next();
};
