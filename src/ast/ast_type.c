#include "ast_type.h"

struct BasicType basic_types[] = {
    {_int_type,    "int"   },
    {_float_type,  "float" },
    {_bool_type,   "bool"  },
    {_char_type,   "char"  },
    {_string_type, "string"},
    {_void_type,   "void"  },
};

struct AnyType ANY_TYPE = {"any"};