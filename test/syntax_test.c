#include "syntax.h"
#include "lex.h"
#include <stdio.h>

void syntax_test() {
    if (!lex_init("/home/riicarus/proj/c_proj/squirrel/test/test_lex.sl", true)) printf("lexer init failed");
    struct AstNode *x = parse();
    print_node(x, 0, NULL);
}