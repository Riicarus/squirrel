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

    lexer_init("/home/riicarus/proj/c_proj/squirrel/test/program.sl", true);
}