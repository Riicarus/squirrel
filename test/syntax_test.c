#include "syntax.h"
#include "lex.h"
#include "semantic.h"
#include "ir_gen.h"
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

    printf("\n\n\n---------------------------------------------------------\n\n\nReachable Check:\n");
    check_stmt(x, false, false);
    printf("\nReachable Check finished\n");

    printf("\n\n\n---------------------------------------------------------\n\n\nAnalyzed AST:\n");

    print_node(x, 0, NULL);

    printf("\n\n\n---------------------------------------------------------\n\n\nTAC:\n");

    struct TAC *tac = CREATE_STRUCT_P(TAC);
    struct TAC *root_tac = tac;
    gen_tac_from_ast(x, &tac);
    struct TAC *head_tac = root_tac->next;
    print_tac(head_tac);

    printf("\n\n\n---------------------------------------------------------\n\n\nOptimized TAC:\n");

    tac_constant_optimize(tac, NULL);
    print_tac(head_tac);
}