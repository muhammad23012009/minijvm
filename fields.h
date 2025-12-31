#ifndef FIELDS_H
#define FIELDS_H

#include <stdint.h>
#include "builtins/builtins.h"

typedef struct Attributes Attributes;
typedef struct ConstantPool ConstantPool;
typedef struct Class Class;
typedef struct builtins builtins;

typedef struct FieldInfo {
    uint16_t access_flags;
    const char *name;
    const char *descriptor;
    Attributes *attributes;
} FieldInfo;

/* Technically Java Method's are also Field's */
typedef FieldInfo MethodInfo;

typedef struct Field {
    struct Class *class;
    const char *name;
    Variant value;

    /* Additionally if something needs to parse lower-level details */
    FieldInfo info;
} Field;

typedef struct Fields {
    int fields_count;
    Field *fields;
    int static_fields_count;
    Field *static_fields;
} Fields;

typedef struct Method {
    Class *class;
    const char *name;
    uint32_t data_length;
    union {
        uint8_t *data;
        void *native_method;
    };
    Descriptors *descriptors;
    int max_stack;
    int max_local;

    MethodInfo info;
} Method;

extern Fields *fields_new(Class *class, Reader *reader, ConstantPool *pool);
extern Fields *fields_new_builtin(Class *class, builtins *class_builtins);
extern void fields_free(Fields *fields);

extern Method *methods_new(Class *class, Reader *reader, ConstantPool *pool, int *count);
extern Method *methods_new_builtin(Class *class, builtins *class_builtins);
extern void methods_free(Method *methods, int count);

#endif