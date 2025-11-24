#include "builtins.h"

static builtin_fields fields[] = {
    { "value", 0x0000 }, // handle ACC_PRIVATE later
};

builtins java_lang_String_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = 1,
    .methods = NULL,
};