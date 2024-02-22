#include <stdio.h>
#include "token.h"

int main() {
    printf("squirrel !\n");
    printf("%d\n", lookup_reserved_tk());
    return 0;
}
