#include "ir.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *tac_op_code_symbols[] = {"EQ",  "NE",  "LT",  "LE",  "GT",  "GE", "ADD", "SUB",   "MUL",        "QUO",      "REM",   "AND",  "OR", "XOR",
                               "SHL", "SHR", "NOT", "MOV", "JMP", "JE", "JNE", "LABEL", "FUNC_START", "FUNC_END", "PARAM", "CALL", "RET"};

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
    if (prev_tac) {
        prev_tac->next = t;
        t->prev = prev_tac;
    }

    return t;
}

void print_tac_list(struct TAC *tac_start, struct TAC *tac_end) {
    if (!tac_start) return;

    switch (tac_start->op) {
        case TAC_HEAD: break;
        case TAC_EQ:
        case TAC_NE:
        case TAC_LT:
        case TAC_LE:
        case TAC_GT:
        case TAC_GE:
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_QUO:
        case TAC_REM:
        case TAC_AND:
        case TAC_OR:
        case TAC_XOR:
        case TAC_SHL:
        case TAC_SHR:
        case TAC_NOT: {
            printf("%s %s", tac_op_code_symbols[tac_start->op], tac_start->x);
            if (*tac_start->y) printf(", %s", tac_start->y);
            printf(", %s\n", tac_start->res);
            break;
        }
        case TAC_MOV: {
            printf("MOV %s, %s\n", tac_start->x, tac_start->y);
            break;
        }
        case TAC_PARAM:
        case TAC_RET:
        case TAC_LABEL:
        case TAC_FUNC_S:
        case TAC_FUNC_E:
        case TAC_JMP: {
            printf("%s %s\n", tac_op_code_symbols[tac_start->op], tac_start->x);
            break;
        }
        case TAC_JE:
        case TAC_JNE: {
            printf("%s %s, %s, %s\n", tac_op_code_symbols[tac_start->op], tac_start->x, tac_start->y, tac_start->res);
            break;
        }
        case TAC_CALL: {
            printf("%s %s, %s", tac_op_code_symbols[tac_start->op], tac_start->x, tac_start->y);
            if (*tac_start->res) printf(", %s", tac_start->res);
            printf("\n");
        }
    }

    if (tac_start == tac_end) return;

    print_tac_list(tac_start->next, tac_end);
}