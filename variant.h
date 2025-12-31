#ifndef VARIANT_H
#define VARIANT_H

typedef struct Class Class;
typedef struct Object Object;
typedef struct Variant Variant;

typedef enum {
    VARIANT_TYPE_NONE,
    VARIANT_TYPE_OBJECT,
    VARIANT_TYPE_REF,
    VARIANT_TYPE_INT,
    VARIANT_TYPE_CLASS,
} VariantType;

typedef struct Variant {
    VariantType type;
    union {
        Class *class;
        Object *object;
        void *ref;
        int int_val;
    } data;
} Variant;

#endif