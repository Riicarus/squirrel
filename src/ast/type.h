#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include "ast.h"

enum TypeCode { _basic_type, _signature_type };

enum BasicTypeCode { _int_type = 1 << 5, _float_type = 1 << 4, _bool_type = 1 << 3, _char_type = 1 << 2, _string_type = 1 << 1, _void_type = 1 };

struct Type {
        enum TypeCode type_code;
        union {
                struct BasicType     *basic_type;
                struct SignatureType *signature_type;
                struct AnyType       *any_type;
        } data;
};

struct BasicType {
        enum BasicTypeCode code;
        char              *name;
};

struct SignatureType {
        struct Type  *ret_type;
        struct Type **param_types;
        unsigned int  param_size;
        unsigned int  param_cap;
};

struct TypeSymbol {
        char *symbol;
};

extern struct BasicType  basic_types[];
extern struct TypeSymbol type_symbols[];

struct Type *create_signature_type(struct FuncDecl *func_decl);
struct Type *create_basic_type(struct BasicTypeDecl *basic_type_decl);
struct Type *create_field_decl_type(struct FieldDecl *field_decl);

#endif