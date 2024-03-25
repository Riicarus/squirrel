#include "lex.h"
#include "token.h"
#include <stdio.h>

void token_test() {
    reserved_tk_map_init();
    enum Token t;
    printf("test get reserved token int(exist): %d\n", (t = lookup_reserved_tk("int")) == _not_exist ? -1 : t);
    printf("test get reserved token string(exist): %d\n", (t = lookup_reserved_tk("string")) == _not_exist ? -1 : t);
    printf("test get reserved token error(not exist): %d\n", (t = lookup_reserved_tk("error")) == _not_exist ? -1 : t);
}

void lexer_test() {
    if (!lex_init("/home/riicarus/proj/c_proj/squirrel/test/test_lex.sl", true)) printf("lexer init failed");
    enum Token tk;
    char       pos_msg[32];
    while ((tk = lex_next()) != _eof && tk != _illegal) {
        sprintf(pos_msg, "%s:%d:%d", filename, row, col);
        printf("%-32s %-8s %-20s %s\n", pos_msg, tk_symbols[tk].symbol, lexeme, lex_bad_msg);
    }

    // print first illegal
    sprintf(pos_msg, "%s:%d:%d", filename, row, col);
    printf("%-32s %-8s %-20s %s\n", pos_msg, tk_symbols[tk].symbol, lexeme, lex_bad_msg);
}