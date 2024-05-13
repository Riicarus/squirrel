#include "ir_optimize.h"
#include "global.h"
#include "ir.h"
#include "c_hashmap.h"
#include "ir_gen.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

hashmap branch_block_tac_map;

struct BranchBlockTacEntryValue {
    struct TAC        *tac;
    struct BasicBlock *block;
};

struct BranchBlockTacEntry {
    char                           *name;
    struct BranchBlockTacEntryValue value;
};

struct BranchBlockTacEntry *_create_branch_block_tac_naming_entry(char *name, struct TAC *tac) {
    struct BranchBlockTacEntry *e = CREATE_STRUCT_P(BranchBlockTacEntry);
    e->name = calloc(sizeof(char), strlen(name) + 1);
    if (!e->name) {
        fprintf(stderr, "_create_tac_naming_entry(), no enough memory");
        free(e);
        exit(EXIT_FAILURE);
    }

    strcpy(e->name, name);
    e->value = (struct BranchBlockTacEntryValue){.tac = tac};

    return e;
}

void *_get_tac_naming_entry_name(void *ele) {
    return ((struct BranchBlockTacEntry *)ele)->name;
}

void *_get_tac_naming_entry_tac(void *ele) {
    return &((struct BranchBlockTacEntry *)ele)->value;
}

void _update_tac_naming_entry(void *ele1, void *ele2) {
    ((struct BranchBlockTacEntry *)ele1)->value = ((struct BranchBlockTacEntry *)ele2)->value;
}

bool _tac_naming_entry_eq_func(void *inner_entry1, void *inner_entry2) {
    return ((struct BranchBlockTacEntryValue *)inner_entry1)->tac == ((struct BranchBlockTacEntryValue *)inner_entry2)->tac;
}

void _init_branch_block_tac_map(struct TAC *tac) {
    branch_block_tac_map = hashmap_new_default(
        _get_tac_naming_entry_name, _get_tac_naming_entry_tac, _update_tac_naming_entry, str_hash_func, str_eq_func, _tac_naming_entry_eq_func);

    while (tac) {
        switch (tac->op) {
            case TAC_LABEL: {
                struct BranchBlockTacEntry *e = _create_branch_block_tac_naming_entry(tac->x, tac);
                hashmap_put(branch_block_tac_map, e);
                break;
            }
            default: break;
        }
        tac = tac->next;
    }
}

struct BasicBlock *create_basic_block(struct TAC *tac) {
    if (!tac) return NULL;

    // if (tac->op == TAC_HEAD) return create_basic_block(tac->next);

    struct BasicBlock *block = CREATE_STRUCT_P(BasicBlock);
    if (!block) {
        fprintf(stderr, "create_basic_block(), no enough memory");
        exit(EXIT_FAILURE);
    }

    block->successors = calloc(8, sizeof(struct BasicBlock *));
    block->successors_cap = 8;
    block->successors_size = 0;
    if (!block->successors) {
        fprintf(stderr, "create_basic_block(), no enough memory");
        free(block);
        exit(EXIT_FAILURE);
    }
    block->head = tac;
    block->visited = false;
    block->level = INT32_MAX;

    return block;
}

void _connect_basic_block(struct BasicBlock *block, int level) {
    if (!block) return;

    if (block->level > level) block->level = level;
    struct TAC *tail = block->tail;
    if (tail->op == TAC_CALL || tail->op == TAC_RET || tail->op == TAC_JMP || tail->op == TAC_JE || tail->op == TAC_JNE) {
        char *name = NULL;
        if (tail->op == TAC_CALL || tail->op == TAC_JMP) name = tail->x;
        else name = tail->res;
        struct BranchBlockTacEntryValue *v = hashmap_get(branch_block_tac_map, &(struct BranchBlockTacEntry){.name = name});
        struct BasicBlock               *successor = v->block;
        bool                             added = false;
        for (int i = 0; i < block->successors_size; i++)
            if (block->successors[i] == successor) {
                added = true;
                break;
            }
        if (!added) {
            block->successors[block->successors_size++] = successor;
            _connect_basic_block(successor, level + 1);
        }

        if (tail->op == TAC_JE || tail->op == TAC_JNE || tail->op == TAC_CALL) goto NEXT_BLOCK;
        return;
    }

NEXT_BLOCK:
    if (!block->next) return;

    struct BasicBlock *successor = NULL;
    if (block->next->head->op != TAC_LABEL || block->next->head->x[0] != 'S') successor = block->next;
    else {
        // func body, ignore
        char *func_name = calloc(strlen(block->next->head->x) + 1, sizeof(char));
        if (!func_name) {
            fprintf(stderr, "_connect_basic_block(), no enough memory");
            exit(EXIT_FAILURE);
        }
        strcpy(func_name, block->next->head->x);
        func_name[0] = 'E';
        struct BranchBlockTacEntryValue *v = hashmap_get(branch_block_tac_map, &(struct BranchBlockTacEntry){.name = func_name});
        // jump to func end block
        successor = v->tac->block->next;
    }

    // connect successor
    if (!successor) return;
    bool added = false;
    for (int i = 0; i < block->successors_size; i++)
        if (block->successors[i] == successor) {
            added = true;
            break;
        }
    if (!added) {
        block->successors[block->successors_size++] = successor;
        _connect_basic_block(successor, level + 1);
    }
}

struct CFG *create_cfg(struct TAC *tac) {
    struct CFG *cfg = CREATE_STRUCT_P(CFG);
    if (!cfg) {
        fprintf(stderr, "create_cfg(), no enough memory");
        exit(EXIT_FAILURE);
    }

    _init_branch_block_tac_map(tac);

    struct TAC        *cur_tac = tac;
    struct BasicBlock *cur_block = NULL;
    struct BasicBlock *last_block = NULL;
    while (cur_tac) {
        // start of a basic block
        if (!cur_block || (cur_tac->op == TAC_LABEL && cur_tac->x[0] != 'E')) {
            struct BasicBlock *new_block = create_basic_block(cur_tac);
            new_block->head = cur_tac;
            cur_tac->block = new_block;

            if (cur_tac->op == TAC_LABEL && cur_tac->x[0] != 'E') {
                struct BranchBlockTacEntryValue *v = hashmap_get(branch_block_tac_map, &(struct BranchBlockTacEntry){.name = cur_tac->x});
                if (!v) {
                    fprintf(stderr, "create_cfg(), can not find tac with name %s", cur_tac->x);
                    exit(EXIT_FAILURE);
                }
                v->block = new_block;
            }

            if (!cfg->entry) cfg->entry = new_block;
            else last_block->next = new_block;

            last_block = new_block;
            cur_block = new_block;
        } else cur_tac->block = cur_block;

        // end of basic block
        cur_block->tail = cur_tac;

        if ((cur_tac->op == TAC_LABEL && cur_tac->x[0] == 'E') || cur_tac->op == TAC_CALL || cur_tac->op == TAC_RET || cur_tac->op == TAC_JMP ||
            cur_tac->op == TAC_JE || cur_tac->op == TAC_JNE)
            cur_block = NULL;

        cur_tac = cur_tac->next;
    }

    _connect_basic_block(cfg->entry, 0);

    // do not need
    hashmap_free(branch_block_tac_map);
    return cfg;
}

hashmap pre_tac_map;

struct PreTacEntry {
    char *name;
    char *value;
};

void *_get_pre_tac_entry_name(void *ele) {
    return ((struct PreTacEntry *)ele)->name;
}

void *_get_pre_tac_entry_value(void *ele) {
    return ((struct PreTacEntry *)ele)->value;
}

void _update_pre_tac_entry(void *ele1, void *ele2) {
    ((struct PreTacEntry *)ele1)->value = ((struct PreTacEntry *)ele2)->value;
}

void _init_pre_tac_map() {
    pre_tac_map = hashmap_new_default(_get_pre_tac_entry_name, _get_pre_tac_entry_value, _update_pre_tac_entry, str_hash_func, str_eq_func, str_eq_func);
}

void pre_optimization(struct BasicBlock *block) {
    struct TAC *tac = block->head;

    while (tac) {
        switch (tac->op) {
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
                if (tac->x[0] == 'V') {
                    char *v = hashmap_get(pre_tac_map, &(struct PreTacEntry){.name = tac->x});
                    if (v) strcpy(tac->x, v);
                }

                if (tac->y[0] == 'V') {
                    char *v = hashmap_get(pre_tac_map, &(struct PreTacEntry){.name = tac->y});
                    if (v) strcpy(tac->y, v);
                }

                if (tac->x[0] == 'L' && tac->y[0] == 'L') {
                    // compute literal
                    int v_x = atoi(unpack_name(tac->x));
                    int v_y = atoi(unpack_name(tac->y));

                    char *res = calloc(256, sizeof(char));
                    if (!res) {
                        fprintf(stderr, "pre_optimization(), no enough memory");
                        exit(EXIT_FAILURE);
                    }
                    if (tac->op == TAC_EQ) strcpy(res, pack_str_arg(v_x == v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_NE) strcpy(res, pack_str_arg(v_x != v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_LT) strcpy(res, pack_str_arg(v_x < v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_LE) strcpy(res, pack_str_arg(v_x <= v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_GT) strcpy(res, pack_str_arg(v_x > v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_GE) strcpy(res, pack_str_arg(v_x >= v_y ? "true" : "false", LIT_PREFIX, false));
                    else if (tac->op == TAC_ADD) strcpy(res, pack_int_arg(v_x + v_y));
                    else if (tac->op == TAC_SUB) strcpy(res, pack_int_arg(v_x - v_y));
                    else if (tac->op == TAC_MUL) strcpy(res, pack_int_arg(v_x * v_y));
                    else if (tac->op == TAC_QUO) strcpy(res, pack_int_arg(v_x / v_y));
                    else if (tac->op == TAC_REM) strcpy(res, pack_int_arg(v_x % v_y));
                    else if (tac->op == TAC_AND) strcpy(res, pack_int_arg(v_x & v_y));
                    else if (tac->op == TAC_OR) strcpy(res, pack_int_arg(v_x | v_y));
                    else if (tac->op == TAC_XOR) strcpy(res, pack_int_arg(v_x ^ v_y));
                    else if (tac->op == TAC_SHL) strcpy(res, pack_int_arg(v_x << v_y));
                    else if (tac->op == TAC_SHR) strcpy(res, pack_int_arg(v_x >> v_y));
                    else if (tac->op == TAC_NOT) strcpy(res, pack_int_arg(!v_x));

                    tac->op = TAC_MOV;
                    strcpy(tac->x, tac->res);
                    strcpy(tac->y, res);
                    tac->res[0] = '\0';

                    struct PreTacEntry *e = CREATE_STRUCT_P(PreTacEntry);
                    e->name = tac->x;
                    e->value = res;
                    hashmap_put(pre_tac_map, e);
                } else hashmap_remove(pre_tac_map, &(struct PreTacEntry){.name = tac->res});

                break;
            }
            case TAC_MOV: {
                if (tac->y[0] == 'L') {
                    struct PreTacEntry *e = CREATE_STRUCT_P(PreTacEntry);
                    e->name = tac->x;
                    e->value = calloc(strlen(tac->y) + 1, sizeof(char));
                    if (!e->value) {
                        fprintf(stderr, "pre_optimization(), no enough memory");
                        exit(EXIT_FAILURE);
                    }
                    strcpy(e->value, tac->y);
                    hashmap_put(pre_tac_map, e);
                    break;
                }
                char *v = hashmap_get(pre_tac_map, &(struct PreTacEntry){.name = tac->y});
                if (v) {
                    strcpy(tac->y, v);

                    // put new entry
                    struct PreTacEntry *e = CREATE_STRUCT_P(PreTacEntry);
                    e->name = tac->x;
                    e->value = v;
                    hashmap_put(pre_tac_map, e);
                } else hashmap_remove(pre_tac_map, &(struct PreTacEntry){.name = tac->x});

                break;
            }
            case TAC_RET:
            case TAC_PARAM: {
                if (tac->x[0] == 'V') {
                    char *v = hashmap_get(pre_tac_map, &(struct PreTacEntry){.name = tac->x});
                    if (v) strcpy(tac->x, v);
                }
                break;
            }
            case TAC_JMP:
            case TAC_JE:
            case TAC_JNE:
            case TAC_LABEL:
            case TAC_CALL: break;
        }

        if (tac == block->tail) break;
        tac = tac->next;
    }
}

hashmap post_tac_map;

#define MW_USE_OFFSET 8
#define MW_ASSIGN_OFFSET 24
#define MW_OP_USE 0
#define MW_OP_ASSIGN 1

struct PostTacEntry {
    char *name;
    int   mark_word;
};

void _set_last_op(int *mark_word, int last_op) {
    if (last_op == MW_OP_USE) *mark_word &= 0xfffffff0;
    else if (last_op == MW_OP_ASSIGN) *mark_word |= 0x00000001;
}

int _get_last_op(int *mark_word) {
    return *mark_word & 0x0000000f;
}

void _inc_use(int *mark_word) {
    *mark_word += 1 << MW_USE_OFFSET;
}

void _set_assigned(int *mark_word) {
    *mark_word |= 1 << MW_ASSIGN_OFFSET;
}

int _get_use(int *mark_word) {
    return (*mark_word & 0x00ffff00) >> MW_USE_OFFSET;
}

bool _is_assigned(int *mark_word) {
    return *mark_word & 1 << MW_ASSIGN_OFFSET;
}

void *_get_post_tac_entry_name(void *ele) {
    return ((struct PostTacEntry *)ele)->name;
}

void *_get_post_tac_entry_value(void *ele) {
    return &((struct PostTacEntry *)ele)->mark_word;
}

void _update_post_tac_entry(void *ele1, void *ele2) {
    ((struct PostTacEntry *)ele1)->mark_word = ((struct PostTacEntry *)ele2)->mark_word;
}

bool _post_tac_value_eq_func(void *value1, void *value2) {
    return *((int *)value1) == *((int *)value1);
}

void _init_post_tac_map() {
    post_tac_map =
        hashmap_new_default(_get_post_tac_entry_name, _get_post_tac_entry_value, _update_post_tac_entry, str_hash_func, str_eq_func, _post_tac_value_eq_func);
}

void post_optimization(struct BasicBlock *block) {
    struct TAC *tac = block->tail;

    while (tac) {
        switch (tac->op) {
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
                int *mark_word_x = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->x});
                if (mark_word_x) {
                    _inc_use(mark_word_x);
                    _set_last_op(mark_word_x, MW_OP_USE);
                } else {
                    struct PostTacEntry *e_x = CREATE_STRUCT_P(PostTacEntry);
                    e_x->name = tac->x;
                    _inc_use(&e_x->mark_word);
                    _set_last_op(&e_x->mark_word, MW_OP_USE);
                    hashmap_put(post_tac_map, e_x);
                }

                if (*tac->y) {
                    int *mark_word_y = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->y});
                    if (mark_word_y) {
                        _inc_use(mark_word_y);
                        _set_last_op(mark_word_y, MW_OP_USE);
                    } else {
                        struct PostTacEntry *e_y = CREATE_STRUCT_P(PostTacEntry);
                        e_y->name = tac->y;
                        _inc_use(&e_y->mark_word);
                        _set_last_op(&e_y->mark_word, MW_OP_USE);
                        hashmap_put(post_tac_map, e_y);
                    }
                }

                int *mark_word_res = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->res});
                if (mark_word_res && _get_use(mark_word_res)) {
                    _set_assigned(mark_word_res);
                    _set_last_op(mark_word_res, MW_OP_ASSIGN);
                } else {
                    // not used, remove MOV
                    if (tac->next) {
                        tac->prev->next = tac->next;
                        tac->next->prev = tac->prev;
                    } else tac->prev->next = NULL;
                    if (tac == block->head) return;
                }

                break;
            }
            case TAC_MOV: {
                int *mark_word_x = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->x});
                if (mark_word_x && _get_use(mark_word_x) && _get_last_op(mark_word_x) != MW_OP_ASSIGN) {
                    _set_assigned(mark_word_x);
                    _set_last_op(mark_word_x, MW_OP_ASSIGN);
                } else {
                    // not used, remove MOV
                    if (tac->next) {
                        tac->prev->next = tac->next;
                        tac->next->prev = tac->prev;
                    } else tac->prev->next = NULL;
                    if (tac == block->head) return;
                }

                if (tac->y[0] == 'V') {
                    int *mark_word_y = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->y});

                    if (mark_word_y) {
                        _inc_use(mark_word_y);
                        _set_last_op(mark_word_y, MW_OP_USE);
                    } else {
                        struct PostTacEntry *e_y = CREATE_STRUCT_P(PostTacEntry);
                        e_y->name = tac->y;
                        _inc_use(&e_y->mark_word);
                        _set_last_op(&e_y->mark_word, MW_OP_USE);
                        hashmap_put(post_tac_map, e_y);
                    }
                }

                break;
            }
            case TAC_JE:
            case TAC_JNE:
            case TAC_PARAM:
            case TAC_RET: {
                int *mark_word_x = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->x});

                if (mark_word_x) {
                    _inc_use(mark_word_x);
                    _set_last_op(mark_word_x, MW_OP_USE);
                } else {
                    struct PostTacEntry *e_x = CREATE_STRUCT_P(PostTacEntry);
                    e_x->name = tac->x;
                    _inc_use(&e_x->mark_word);
                    _set_last_op(&e_x->mark_word, MW_OP_USE);
                    hashmap_put(post_tac_map, e_x);
                    int *m_test = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->x});
                    int  i = 0;
                }
                break;
            }
            case TAC_CALL: {
                if (!*tac->res) break;

                int *mark_word_res = hashmap_get(post_tac_map, &(struct PostTacEntry){.name = tac->res});
                if (mark_word_res && _get_use(mark_word_res)) {
                    _set_assigned(mark_word_res);
                    _set_last_op(mark_word_res, MW_OP_ASSIGN);
                } else tac->res[0] = '\0';

                break;
            }
            case TAC_JMP:
            case TAC_LABEL: break;
        }

        if (tac == block->head) return;
        tac = tac->prev;
    }
}

#define BLOCK_Q_SIZE 100000
struct BasicBlock *block_q[BLOCK_Q_SIZE] = {};
int                q_h = 0, q_t = 0;

void _block_q_push(struct BasicBlock *block) {
    int next_q_t = (q_t + 1) % BLOCK_Q_SIZE;
    if (next_q_t != q_h) block_q[q_t++] = block;
    else fprintf(stderr, "_block_q_push(), block_q is full\n");
}

struct BasicBlock *_block_q_pop() {
    if (q_t == q_h) return NULL;
    struct BasicBlock *b = block_q[q_h];
    q_h = (q_h + 1) % BLOCK_Q_SIZE;
    return b;
}

void _do_optimize_tac(struct BasicBlock *block) {
    if (!block) return;

    // computation & propagation optimization
    pre_optimization(block);
    block->visited = true;

    // choose one not-visited successor of block
    if (!block->successors_size) return;
    struct BasicBlock *next_block = NULL;
    int                i = 0;
    for (; i < block->successors_size; i++) {
        if (block->successors[i]->visited) continue;
        next_block = block->successors[i];
        _do_optimize_tac(next_block);
    }

    // redundancy optimization
    // post_optimization(block);
    for (i = 0; i < block->successors_size; i++) {
        if (block->successors[i]->level < block->level) continue;
        post_optimization(block->successors[i]);
    }
}

void optimize_tac(struct BasicBlock *block) {
    if (!block) return;

    _init_pre_tac_map();
    _init_post_tac_map();

    _do_optimize_tac(block);
    post_optimization(block);

    hashmap_free(pre_tac_map);
    hashmap_free(post_tac_map);
}

void print_cfg(struct CFG *cfg) {
    if (!cfg) return;

    struct BasicBlock *block = cfg->entry;
    while (block) {
        print_tac_list(block->head, block->tail);
        printf("\n");
        block = block->next;
    }
}