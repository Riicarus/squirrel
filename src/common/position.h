#ifndef POSITION_H
#define POSITION_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct Position {
        char *filename;
        int   off;
        int   row;
        int   col;
};

static struct Position *new_position(char *filename, int off, int row, int col) {
    if (!filename) {
        fprintf(stderr, "empty filename\n");
        return NULL;
    }

    struct Position *pos = malloc(sizeof(struct Position));
    if (!pos) {
        fprintf(stderr, "no enough memory\n");
        return NULL;
    }
    pos->off = off;
    pos->row = row;
    pos->col = col;
    pos->filename = calloc(strlen(filename) + 1, sizeof(char));
    if (!pos->filename) {
        fprintf(stderr, "no enough memory\n");
        free(pos);
        return NULL;
    }
    strcpy(pos->filename, filename);
    return pos;
}

#endif
