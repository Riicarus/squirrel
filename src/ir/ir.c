#include "ir.h"
#include "global.h"
#include <stdlib.h>
#include <stdio.h>

static int block_id = 0;

struct BasicBlock *create_basic_block() {
    struct BasicBlock *block = CREATE_STRUCT_P(BasicBlock);
    if (!block) {
        fprintf(stderr, "create_basic_block(), no enough memory");
        exit(EXIT_FAILURE);
        return NULL;
    }

    block->id = block_id++;
    block->instructions = calloc(8, sizeof(struct Quad *));
    if (!block->instructions) {
        fprintf(stderr, "create_basic_block(), no enough memory");
        free(block);
        block = NULL;
        exit(EXIT_FAILURE);
        return NULL;
    }
    block->instructions_cap = 8;
    block->instructions_size = 0;

    block->successors = calloc(8, sizeof(struct BasicBlock *));
    if (!block->successors) {
        fprintf(stderr, "create_basic_block(), no enough memory");
        free(block->instructions);
        free(block);
        block = NULL;
        exit(EXIT_FAILURE);
        return NULL;
    }
    block->successors_cap = 8;
    block->successors_size = 0;

    return block;
}