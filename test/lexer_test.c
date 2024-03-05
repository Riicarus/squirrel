#include "token.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>

void token_test() {
    reserved_tk_map_init();
    token t;
    printf("test get reserved token int(exist): %d\n", (t = lookup_reserved_tk("int")) == _not_exist ? -1 : t);
    printf("test get reserved token string(exist): %d\n", (t = lookup_reserved_tk("string")) == _not_exist ? -1 : t);
    printf("test get reserved token error(not exist): %d\n", (t = lookup_reserved_tk("error")) == _not_exist ? -1 : t);
}

void lexer_test() {
    lexer_init("/home/riicarus/proj/c_proj/squirrel/test/program.sl", true);
    token tk;
    char  pos_msg[32];
    while ((tk = lexer_next()) != _eof && tk != _illegal) {
        sprintf(pos_msg, "%s:%d:%d", lexer->filename, lexer->row, lexer->col);
        printf("%-32s %-4d %-20s %s\n", pos_msg, lexer->tk, lexer->lexeme, lexer->bad_msg);
    }

    // print first illegal
    sprintf(pos_msg, "%s:%d:%d", lexer->filename, lexer->row, lexer->col);
    printf("%-32s %-4d %-20s %s\n", pos_msg, lexer->tk, lexer->lexeme, lexer->bad_msg);
}