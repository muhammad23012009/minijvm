#ifndef VARIANT_H
#define VARIANT_H

#include <stdbool.h>
#include <stdlib.h>

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
    bool owned;
    int refcnt;
    union {
        Class *class;
        Object *object;
        void *ref;
        int int_val;
    } data;
} Variant;

static inline void variant_acquire(Variant *variant)
{
    if (variant->type == VARIANT_TYPE_REF && variant->owned) {
        variant->refcnt++;
    }
}

static inline Variant variant_make_int(int value)
{
    Variant ret = (Variant){ .type = VARIANT_TYPE_INT, .data.int_val = value, .refcnt = 0 };
    variant_acquire(&ret);
    return ret;
}

static inline Variant variant_make_class(Class *class)
{
    Variant ret = (Variant){ .type = VARIANT_TYPE_CLASS, .data.class = class, .refcnt = 0 };
    variant_acquire(&ret);
    return ret;
}

static inline Variant variant_make_object(Object *object)
{
    Variant ret = (Variant){ .type = VARIANT_TYPE_OBJECT, .data.object = object, .refcnt = 0 };
    variant_acquire(&ret);
    return ret;
}

static inline Variant variant_make_ref(void *ref)
{
    Variant ret = (Variant){ .type = VARIANT_TYPE_REF, .data.ref = ref, .owned = false, .refcnt = 0 };
    variant_acquire(&ret);
    return ret;
}

static inline Variant variant_make_owned_ref(void *ref)
{
    Variant ret = (Variant){ .type = VARIANT_TYPE_REF, .owned = true, .data.ref = ref, .refcnt = 0 };
    variant_acquire(&ret);
    return ret;
}

static inline void variant_release(Variant *variant)
{
    if (!variant)
        return;

    if (variant->type == VARIANT_TYPE_REF && variant->owned && variant->data.ref && --variant->refcnt == 0) {
        free(variant->data.ref);
        variant->data.ref = NULL;
    }
}

#endif