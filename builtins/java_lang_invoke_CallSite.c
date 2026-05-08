#include "builtins.h"

static builtin_fields fields[] = {
    { "target", "Ljava/lang/invoke/MethodHandle;", 0x0000 },
};

Object *java_lang_invoke_CallSite_create(jit_function_t target)
{
    Class *callsite_class = classes_get_class("java/lang/invoke/CallSite");
    Object *callsite = object_new(callsite_class);

    object_set_field(callsite, "target", variant_make_object(java_lang_invoke_MethodHandle_create(target)));

    return callsite;
}

builtins java_lang_invoke_CallSite_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = NULL,
    .methods_length = 0,
};