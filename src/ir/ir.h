#ifndef IR_H
#define IR_H

enum TacOpCode {
    TAC_EQ,
    TAC_NE,
    TAC_LT,
    TAC_LE,
    TAC_GT,
    TAC_GE,
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_QUO,
    TAC_REM,
    TAC_AND,
    TAC_OR,
    TAC_XOR,
    TAC_SHL,
    TAC_SHR,
    TAC_NOT,
    TAC_MOV,
    TAC_JMP,
    TAC_JE,
    TAC_JNE,
    TAC_LABEL,
    TAC_FUNC_S,
    TAC_FUNC_E,
    TAC_PARAM,
    TAC_CALL,
    TAC_RET
};

extern char *tac_op_code_symbols[];

struct TAC {
        enum TacOpCode op;
        char           x[256];
        char           y[256];
        char           res[256];

        struct TAC *next;
};

struct TAC *create_tac(struct TAC *prev_tac, enum TacOpCode op, char *x, char *y, char *res);

#endif