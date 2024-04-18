#include "ir_optimize.h"
#include "global.h"
#include "ir.h"
#include "c_hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

hashmap tac_map;

struct TacEntry {
    char       *name;
    struct TAC *tac;
    enum { LABEL, FUNC_S, FUNC_E } type;
    struct BasicBlock *block;
};

struct TacEntry *_create_tac_naming_entry(char *name, struct TAC *tac) {
    struct TacEntry *e = CREATE_STRUCT_P(TacEntry);
    e->name = calloc(sizeof(char), strlen(name) + 1);
    if (!e->name) {
        fprintf(stderr, "_create_tac_naming_entry(), no enough memory");
        free(e);
        exit(EXIT_FAILURE);
    }

    strcpy(name, e->name);
    e->tac = tac;

    return e;
}

void *_get_tac_naming_entry_name(void *ele) {
    return ((struct TacEntry *)ele)->name;
}

void *_get_tac_naming_entry_tac(void *ele) {
    return ((struct TacEntry *)ele)->tac;
}

void _update_tac_naming_entry(void *ele1, void *ele2) {
    ((struct TacEntry *)ele1)->tac = ((struct TacEntry *)ele2)->tac;
    ((struct TacEntry *)ele1)->type = ((struct TacEntry *)ele2)->type;
    ((struct TacEntry *)ele1)->block = ((struct TacEntry *)ele2)->block;
}

bool _tac_naming_entry_eq_func(void *tac1, void *tac2) {
    return ((struct TAC *)tac1) == ((struct TAC *)tac2);
}

void _init_tac_map(struct TAC *tac) {
    tac_map = hashmap_new_default(
        _get_tac_naming_entry_name, _get_tac_naming_entry_tac, _update_tac_naming_entry, str_hash_func, str_eq_func, _tac_naming_entry_eq_func);

    struct TacEntry *e = NULL;
    while (tac) {
        switch (tac->op) {
            case TAC_LABEL: {
                e = _create_tac_naming_entry(tac->x, tac);
                e->type = LABEL;
                hashmap_put(tac_map, e);
                break;
            }
            case TAC_FUNC_E: {
                e = _create_tac_naming_entry(tac->x, tac);
                e->type = FUNC_E;
                hashmap_put(tac_map, e);
                break;
            }
            case TAC_FUNC_S: {
                e = _create_tac_naming_entry(tac->x, tac);
                e->type = FUNC_S;
                hashmap_put(tac_map, e);
                break;
            }
            default: break;
        }
        tac = tac->next;
    }
}

struct BasicBlock *create_basic_block(struct TAC *tac) {
    if (!tac) return NULL;

    if (tac->op == TAC_HEAD) return create_basic_block(tac->next);

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

    return block;
}

struct CFG *create_cfg(struct TAC *tac) {
    struct CFG *cfg = CREATE_STRUCT_P(CFG);
    if (!cfg) {
        fprintf(stderr, "create_cfg(), no enough memory");
        exit(EXIT_FAILURE);
    }

    _init_tac_map(tac);

    struct TAC        *cur_tac = tac;
    struct BasicBlock *cur_block = NULL;
    struct BasicBlock *last_block = NULL;
    while (cur_tac) {
        // start of a basic block
        if (!cur_block || cur_tac->op == TAC_LABEL || cur_tac->op == TAC_FUNC_S) {
            struct BasicBlock *new_block = create_basic_block(cur_tac);
            new_block->head = cur_tac;
            cur_tac->block = new_block;

            if (cur_tac->op == TAC_LABEL || cur_tac->op == TAC_FUNC_S) {
                struct TacEntry *e = hashmap_get(tac_map, &(struct TacEntry){.name = cur_tac->x});
                if (!e) {
                    fprintf(stderr, "create_cfg(), can not find tac with name %s", cur_tac->x);
                    exit(EXIT_FAILURE);
                }
                e->block = new_block;
            }

            if (!cfg->entry) cfg->entry = new_block;
            else last_block->next = new_block;

            last_block = new_block;
            cur_block = new_block;
        }

        // end of basic block
        cur_block->tail = cur_tac;

        if (cur_tac->op == TAC_FUNC_E || cur_tac->op == TAC_CALL || cur_tac->op == TAC_RET || cur_tac->op == TAC_JMP || cur_tac->op == TAC_JE ||
            cur_tac->op == TAC_JNE)
            cur_block = NULL;

        cur_tac = cur_tac->next;
    }

    // connect basic blocks

    cur_tac = tac;
    while (cur_tac) {
        if (cur_tac->op == TAC_CALL || cur_tac->op == TAC_RET || cur_tac->op == TAC_JMP || cur_tac->op == TAC_JE || cur_tac->op == TAC_JNE) {
            struct TacEntry   *e = hashmap_get(tac_map, &(struct TacEntry){.name = cur_tac->x});
            struct TAC        *t = e->tac;
            struct BasicBlock *successor = e->block;
            cur_block = cur_tac->block;
            cur_block->successors[cur_block->successors_size++] = successor;
        }
    }

    return cfg;
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