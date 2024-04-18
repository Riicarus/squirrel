#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>

#define CREATE_STRUCT_P(T) calloc(1, sizeof(struct T))

extern bool debug;

#endif