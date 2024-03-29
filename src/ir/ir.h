#ifndef IR_H
#define IR_H

enum QuadOpCode {
    QUAD_ADD,
    QUAD_SUB,
    QUAD_MUL,
    QUAD_QUO,
    QUAD_REM,
    QUAD_AND,
    QUAD_OR,
    QUAD_NOT,
    QUAD_XOR,
    QUAD_MOV,
    QUAD_LOAD,
    QUAD_STORE,
    QUAD_JMP,
    QUAD_JE,
    QUAD_JNE,
    QUAD_JLT,
    QUAD_JLE,
    QUAD_JGT,
    QUAD_JGE,
    QUAD_CALL,
    QUAD_RET,
    QUAD_LABEL,
    QUAD_FUNC,
    QUAD_SET_ELE,
    QUAD_GET_ELE
};

struct Quad {
        enum QuadOpCode op;
        char           *x;
        char           *y;
        char           *res;

        struct Quad *next;
};

struct BasicBlock {
        int id;

        struct Quad **instructions;
        unsigned int  instructions_size;
        unsigned int  instructions_cap;

        struct BasicBlock **successors;
        unsigned int        successors_size;
        unsigned int        successors_cap;
};

struct CFG {
        struct BasicBlock *entry;
        struct BasicBlock *exit;
};

struct BasicBlock *create_basic_block();

#endif