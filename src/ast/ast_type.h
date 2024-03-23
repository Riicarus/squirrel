#ifndef AST_TYPE_H
#define AST_TYPE_H

#include <stdbool.h>

enum TypeCode { _basic, _signature, _any };

enum BasicTypeCode {
    _int_type = 1 << 5,
    _float_type = 1 << 4,
    _bool_type = 1 << 3,
    _char_type = 1 << 2,
    _string_type = 1 << 1,
    _void_type = 1
};

enum {
    _int_cast = _int_type | _float_type | _bool_type | _char_type,
    _float_cast = _int_type | _float_type | _bool_type,
    _bool_cast = _int_type | _float_type | _bool_type | _string_type,
    _char_cast = _int_type | _float_cast | _char_type | _string_type,
    _string_cast = _string_type,
    _void_cast = _void_type
};

// clang-format off
#define CAN_CAST(F, T) (_##F_cast & _##T_type)
// clang-format on

struct AstType {
    enum TypeCode type_code;
};

struct BasicType {
    struct AstType     base;
    enum BasicTypeCode code;
    char              *name;
} basic_types[] = {
    {_basic, _int_type,    "int"   },
    {_basic, _float_type,  "float" },
    {_basic, _bool_type,   "bool"  },
    {_basic, _char_type,   "char"  },
    {_basic, _string_type, "string"},
    {_basic, _void_type,   "void"  },
};

struct ArrayType {
    struct AstType ele_type;
};

struct SignatureType {
    struct AstType  base;
    struct AstType *ret_type;
    int             param_size;
    struct AstType *param_types;
    char           *name;
};

struct {
    struct AstType base;
    char          *name;
} ANY_TYPE = {_any, "any"};

#endif