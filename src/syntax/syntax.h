#ifndef SYNTAXER_H
#define SYNTAXER_H

#include "lex.h"
#include "position.h"
#include <stdbool.h>

typedef bool (*update_list_func)(enum Token _tk);

#endif