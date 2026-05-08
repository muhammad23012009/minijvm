#include "builtins.h"

// we'll handle everything else at some other point

// TODO: implement constructors that we can call with object_new?
Object *java_lang_invoke_MethodHandle_create(jit_function_t method)
{
    Class *methodhandle_class = classes_get_class("java/lang/invoke/MethodHandle");
    Object *method_handle = object_new(methodhandle_class);
    object_set_field(method_handle, "method", variant_make_ref(method));

    return method_handle;
}

static builtin_fields fields[] = {
    {
        // Pointer to a native Method struct
        "method", "", 0x0000
    },
};

builtins java_lang_invoke_MethodHandle_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = NULL,
    .methods_length = 0,
};