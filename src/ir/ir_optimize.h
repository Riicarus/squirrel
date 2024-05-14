#ifndef IR_OPTIMIZE_H
#define IR_OPTIMIZE_H

#include <stdbool.h>

struct BasicBlock {
    struct BasicBlock **successors;
    int                 successors_size;
    int                 successors_cap;

    struct BasicBlock *next;

    bool        is_func;
    struct TAC *head;
    struct TAC *tail;

    bool visited;
    int  level;
};

struct CFG {
    struct BasicBlock *entry;
    struct BasicBlock *exit;
};

struct BasicBlock *create_basic_block(struct TAC *tac);
struct CFG        *create_cfg(struct TAC *tac);
void               print_cfg(struct CFG *cfg, bool only_reachable, bool split);

void pre_optimization(struct BasicBlock *block);
void post_optimization(struct BasicBlock *block);
void optimize_tac(struct BasicBlock *block);

#endif