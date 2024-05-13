#include "syntax.h"
#include "lex.h"
#include "semantic.h"
#include "ir_gen.h"
#include "ir_optimize.h"

#include <stdio.h>

void syntax_test() {
    if (!lex_init("/home/riicarus/proj/c_proj/squirrel/test/test_optimize.sl", true)) printf("lexer init failed\n");
    printf("\n");
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
    tac->op = TAC_HEAD;
    struct TAC *root_tac = tac;
    gen_tac_from_ast(x, &tac, NULL);
    print_tac_list(root_tac, NULL);

    // printf("\n\n\n---------------------------------------------------------\n\n\nOptimized TAC:\n");

    // root_tac = tac_global_var_removal(tac, NULL);
    // print_tac_list(root_tac, NULL);

    printf("\n\n\n---------------------------------------------------------\n\n\nCFG:\n");
    struct CFG *cfg = create_cfg(root_tac);
    print_cfg(cfg);

    printf("\n\n\n---------------------------------------------------------\n\n\nOptimized CFG:\n");

    optimize_tac(cfg->entry);
    print_tac_list(root_tac, NULL);
}