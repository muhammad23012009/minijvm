#include "builtins.h"

Object *java_lang_Throwable_getMessage(Object *this)
{
    return object_get_field(this, "message")->value.data.object;
}

void java_lang_Throwable_init(Object *this, Object* message)
{
    object_set_field(this, "message", variant_make_object(message));
}

static builtin_methods methods[] = {
    { "getMessage", "()Ljava/lang/String;", 0, java_lang_Throwable_getMessage },
    { "<init>", "(Ljava/lang/String;)V", 0, java_lang_Throwable_init },
};

static builtin_fields fields[] = {
    { "message", "Ljava/lang/String;", 0 },
};

builtins java_lang_Throwable_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};