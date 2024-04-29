#include "stdio.h"

extern void token_test();
extern void lexer_test();

extern void syntax_test();

int main() {
    // token_test();
    // lexer_test();

    printf("\n\n\n---------------------------------------------------------\n\n\n");
    syntax_test();
}