#include "builtins.h"

static builtin_methods methods[] = {
    { "methodType", "(Ljava/lang/Class;Ljava/lang/Class;)Ljava/lang/invoke/MethodType;", 0x0008, NULL },
    { "methodType", "(Ljava/lang/Class;[Ljava/lang/Class;)Ljava/lang/invoke/MethodType;", 0x0008, NULL },
};

static builtin_fields fields[] = {
    // Stores the raw descriptor string.
    { "descriptor", "", 0 }
};

builtins java_lang_invoke_MethodType_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};