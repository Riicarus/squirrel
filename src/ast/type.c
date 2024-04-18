#include "type.h"
#include "global.h"

// mapping to enum Token definition
struct BasicType basic_types[] = {
    {_int_type,    "int"   },
    {_float_type,  "float" },
    {_bool_type,   "bool"  },
    {_char_type,   "char"  },
    {_string_type, "string"},
    {_void_type,   "void"  },
};

struct TypeSymbol type_symbols[] = {"basic", "signature"};

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
    signature_type->param_cap = signature_type->param_size = func_decl->param_size;
    signature_type->param_types = calloc(signature_type->param_size, sizeof(struct Type *));
    if (!signature_type->param_types) {
        free(signature_type);
        signature_type = NULL;
        fprintf(stderr, "create_signature_type(), no enough memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    for (int i = 0; i < func_decl->param_size; i++) signature_type->param_types[i] = create_field_decl_type(func_decl->param_decls[i]->data.field_decl);

    // return type of func is basic type
    if (func_decl->ret_type_decl->class == BASIC_TYPE_DECL) signature_type->ret_type = create_basic_type(func_decl->ret_type_decl->data.basic_type_decl);

    t->type_code = _signature_type;
    t->data.signature_type = signature_type;
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
    basic_type->code = basic_types[basic_type_decl->tk].code;
    basic_type->name = basic_types[basic_type_decl->tk].name;
    t->type_code = _basic_type;
    t->data.basic_type = basic_type;
    return t;
}

struct Type *create_field_decl_type(struct FieldDecl *field_decl) {
    if (!field_decl) return NULL;
    switch (field_decl->type_decl->class) {
        case BASIC_TYPE_DECL: return create_basic_type(field_decl->type_decl->data.basic_type_decl);
        default: {
            fprintf(stderr, "invalid type class\n");
            exit(EXIT_FAILURE);
            return NULL;
        }
    }
}