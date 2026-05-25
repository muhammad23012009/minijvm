#include "builtins.h"

/* Store a map of classes with their names. */
Object *java_lang_Class_getName(Object *this)
{
    Class *class = object_get_field(this, "class")->value.data.class;
    Object *string = object_new(classes_get_class("java/lang/String"));

    object_set_field(string, "value", variant_make_ref(class->name));
    return string;
}

Object *java_lang_Class_getClassLoader(Object *this)
{
    /* We don't support class loaders, so just return null. */
    return NULL;
}

builtin_fields java_lang_Class_fields[] = {
    /* Internal "Class" object pointer */
    { "class", "", 0x0000 },
};

builtin_methods java_lang_Class_methods[] = {
    { "getName", "()Ljava/lang/String;", 0x0000, &java_lang_Class_getName },
    { "getClassLoader", "()Ljava/lang/ClassLoader;", 0x0000, &java_lang_Class_getClassLoader },
};

builtins java_lang_Class_builtins = {
    .parent = "java/lang/Object",
    .fields = java_lang_Class_fields,
    .fields_length = ARRAY_SIZE(java_lang_Class_fields),
    .methods = java_lang_Class_methods,
    .methods_length = ARRAY_SIZE(java_lang_Class_methods),
};