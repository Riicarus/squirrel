#ifndef AST_TYPE_H
#define AST_TYPE_H

#include <stdbool.h>

enum TypeCode { _basic, _signature, _any };

enum BasicTypeCode { _int_type = 1 << 5, _float_type = 1 << 4, _bool_type = 1 << 3, _char_type = 1 << 2, _string_type = 1 << 1, _void_type = 1 };

struct AstType {
    enum TypeCode type_code;
    union {
        struct BasicType     *basic_type;
        struct ArrayType     *array_type;
        struct SignatureType *signature_type;
        struct AnyType       *any_type;
    } data;
};

struct BasicType {
    enum BasicTypeCode code;
    char              *name;
};

struct ArrayType {
    struct AstType ele_type;
};

struct SignatureType {
    struct AstType *ret_type;
    int             param_size;
    struct AstType *param_types;
    char           *name;
};

struct AnyType {
    char *name;
};

extern struct BasicType basic_types[];
extern struct AnyType ANY_TYPE;

#endif