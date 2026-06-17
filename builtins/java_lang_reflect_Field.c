#include "builtins.h"

Object *java_lang_reflect_Field_getDeclaringClass(Object *this)
{
    return object_get_field(this, "declaringClass")->value.data.object;
}

Object *java_lang_reflect_Field_getName(Object *this)
{
    return object_get_field(this, "name")->value.data.object;
}

Object *java_lang_reflect_Field_init_custom(Object *name, Object *declaring_class)
{
    Object *field = object_new(classes_get_class("java/lang/reflect/Field"));
    object_set_field(field, "name", variant_make_object(name));
    object_set_field(field, "declaringClass", variant_make_object(declaring_class));
    return field;
}

static builtin_methods methods[] = {
    { "getDeclaringClass", "()Ljava/lang/Class;", 0, &java_lang_reflect_Field_getDeclaringClass },
    { "getName", "()Ljava/lang/String;", 0, &java_lang_reflect_Field_getName },
};

static builtin_fields fields[] = {
    { "name", "Ljava/lang/String;", ACC_PRIVATE },
    { "declaringClass", "Ljava/lang/Class;", ACC_PRIVATE },
};

builtins java_lang_reflect_Field_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .methods = methods,
    .fields_length = ARRAY_SIZE(fields),
    .methods_length = ARRAY_SIZE(methods),
};