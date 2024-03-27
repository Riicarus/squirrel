#include "syntax.h"
#include "lex.h"
#include "semantic.h"
#include <stdio.h>

void syntax_test() {
    if (!lex_init("/home/riicarus/proj/c_proj/squirrel/test/test_lex.sl", true)) printf("lexer init failed");
    struct AstNode *x = parse();

    printf("\n\n\n---------------------------------------------------------\n\n\nAST:\n");

    print_node(x, 0, NULL);

    printf("\n\n\n---------------------------------------------------------\n\n\nScope Manage:\n");
    manage_scope(x, NULL, false);

    printf("\n\n\n---------------------------------------------------------\n\n\nType Check:\n");
    check_node_type(x, NULL, NULL, false);
    printf("\nType Check finished\n");
}