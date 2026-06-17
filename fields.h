#ifndef FIELDS_H
#define FIELDS_H

#include <stdint.h>
#include "variant.h"

#define ACC_PUBLIC       0x0001
#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_FINAL        0x0010
#define ACC_SYNCHRONIZED 0x0020
#define ACC_BRIDGE       0x0040
#define ACC_VARARGS      0x0080
#define ACC_NATIVE       0x0100
#define ACC_ABSTRACT     0x0400

typedef struct Attributes Attributes;
typedef struct ConstantPool ConstantPool;
typedef struct Class Class;
typedef struct builtins builtins;
typedef struct Reader Reader;
typedef struct Descriptors Descriptors;

typedef struct ExceptionHandler {
    uintptr_t start_pc;
    uintptr_t end_pc;
    uintptr_t handler_pc;
    uint16_t catch_type;
} ExceptionHandler;

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
    int exception_table_length;
    ExceptionHandler *exception_table;
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