#include "ir.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *tac_op_code_symbols[] = {
    "EQ", "NE", "LT", "LE", "GT", "GE", "ADD", "SUB", "MUL", "QUO", "REM", "AND", "OR", "XOR", "SHL", "SHR", "NOT", "MOV", "JMP", "JE", "JNE", "LABEL", "TAC_FUNC_S", "TAC_FUNC_E", "PARAM", "CALL", "RET"};

struct TAC *create_tac(struct TAC *prev_tac, enum TacOpCode op, char *x, char *y, char *res) {
    struct TAC *t = CREATE_STRUCT_P(TAC);
    if (!t) {
        fprintf(stderr, "create_tac(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }

    t->op = op;
    if (x) {
        strncpy(t->x, x, 255);
        t->x[255] = '\0';
    }
    if (y) {
        strncpy(t->y, y, 255);
        t->y[255] = '\0';
    }
    if (res) {
        strncpy(t->res, res, 255);
        t->res[255] = '\0';
    }
    if (prev_tac) prev_tac->next = t;

    printf("%s, %s, %s, %s\n", tac_op_code_symbols[op], x ? x : "_", y ? y : "_", res ? res : "_");

    return t;
}