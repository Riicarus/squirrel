#include "type.h"
#include "global.h"

struct BasicType basic_types[] = {
    {_int_type,    "int"   },
    {_float_type,  "float" },
    {_bool_type,   "bool"  },
    {_char_type,   "char"  },
    {_string_type, "string"},
    {_void_type,   "void"  },
};

struct AnyType ANY_TYPE = {"any"};

struct TypeSymbol type_symbols[] = {
    "basic",
    "array",
    "signature",
    "any"
};

struct Type *create_signature_type(struct FuncDecl *func_decl) {
    if (!func_decl) return NULL;

    struct SignatureType *signature_type = CREATE_STRUCT_P(SignatureType);
    if (!signature_type) {
        fprintf(stderr, "create_signature_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }

    struct Type *t = CREATE_STRUCT_P(Type);
    if (!t) {
        free(signature_type);
        signature_type = NULL;
        fprintf(stderr, "create_signature_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    t->type_code = _signature_type;
    t->data.signature_type = signature_type;
    return t;
}

struct Type *create_array_type(struct ArrayTypeDecl *array_type_decl) {
    if (!array_type_decl) return NULL;

    struct ArrayType *array_type = CREATE_STRUCT_P(ArrayType);
    if (!array_type) {
        fprintf(stderr, "create_array_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }

    struct Type *t = CREATE_STRUCT_P(Type);
    if (!t) {
        free(array_type);
        array_type = NULL;
        fprintf(stderr, "create_array_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    t->type_code = _array_type;
    t->data.array_type = array_type;
    return t;
}

struct Type *create_basic_type(struct BasicTypeDecl *basic_type_decl) {
    if (!basic_type_decl) return NULL;

    struct BasicType *basic_type = CREATE_STRUCT_P(BasicType);
    if (!basic_type) {
        fprintf(stderr, "create_basic_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }

    struct Type *t = CREATE_STRUCT_P(Type);
    if (!t) {
        free(basic_type);
        basic_type = NULL;
        fprintf(stderr, "create_basic_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    t->type_code = _basic_type;
    t->data.basic_type = basic_type;
    return t;
}

struct Type *create_type(struct FieldDecl *field_decl) {
    if (!field_decl) return NULL;
    switch (field_decl->type_decl->class) {
        case ARRAY_TYPE_DECL: return create_array_type(field_decl->type_decl->data.array_type_decl);
        case BASIC_TYPE_DECL: return create_basic_type(field_decl->type_decl->data.basic_type_decl);
        default: {
            fprintf(stderr, "invalid type class\n");
            exit(EXIT_FAILURE);
            return NULL;
        }
    }
}