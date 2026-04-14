#include "builtins.h"

/* Store a map of classes with their names. */
Object *java_lang_Class_getName(Object *this)
{
    Class *class = object_get_field(this, "class")->value.data.class;
    Object *string = object_new(classes_get_class("java/lang/String"));

    object_get_field(string, "value")->value.data.ref = class->name;
    return string;
}

builtin_fields java_lang_Class_fields[] = {
    /* Internal "Class" object pointer */
    { "class", "", 0x0000 },
};

builtin_methods java_lang_Class_methods[] = {
    { "getName", "()Ljava/lang/String;", 0x0000, &java_lang_Class_getName },
};

builtins java_lang_Class_builtins = {
    .parent = "java/lang/Object",
    .fields = java_lang_Class_fields,
    .fields_length = ARRAY_SIZE(java_lang_Class_fields),
    .methods = java_lang_Class_methods,
    .methods_length = ARRAY_SIZE(java_lang_Class_methods),
};