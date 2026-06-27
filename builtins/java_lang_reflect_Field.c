#include "builtins.h"

Object *java_lang_reflect_Field_getDeclaringClass(Object *this)
{
    return object_get_field(this, "declaringClass")->value.data.object;
}

Object *java_lang_reflect_Field_getName(Object *this)
{
    return object_get_field(this, "name")->value.data.object;
}

int java_lang_reflect_Field_getModifiers(Object *this)
{
    Object *class_obj = object_get_field(this, "declaringClass")->value.data.object;
    Class *class = object_get_field(class_obj, "class")->value.data.class;
    Object *name_obj = object_get_field(this, "name")->value.data.object;
    const char* name = object_get_field(name_obj, "value")->value.data.ref;
    Field *field = class_get_field(class, name);
    return field->info.access_flags;
}

Object *java_lang_reflect_Field_getType(Object *this)
{
    Object *class_obj = object_get_field(this, "clazz")->value.data.object;
    Class *class = object_get_field(class_obj, "class")->value.data.class;
    printf("Getting type of field %s, which is class %s\n", object_get_field(object_get_field(this, "name")->value.data.object, "value")->value.data.ref, class->name);
    return class_obj;
}

Object *java_lang_reflect_Field_init_custom(Object *name, Object *declaring_class, Object *clazz)
{
    Object *field = object_new(classes_get_class("java/lang/reflect/Field"));
    object_set_field(field, "name", variant_make_object(name));
    object_set_field(field, "declaringClass", variant_make_object(declaring_class));
    object_set_field(field, "clazz", variant_make_object(clazz));
    return field;
}

static builtin_methods methods[] = {
    { "getDeclaringClass", "()Ljava/lang/Class;", 0, &java_lang_reflect_Field_getDeclaringClass },
    { "getName", "()Ljava/lang/String;", 0, &java_lang_reflect_Field_getName },
    { "getModifiers", "()I", 0, &java_lang_reflect_Field_getModifiers },
    { "getType", "()Ljava/lang/Class;", 0, &java_lang_reflect_Field_getType },
};

static builtin_fields fields[] = {
    { "name", "Ljava/lang/String;", ACC_PRIVATE },
    { "declaringClass", "Ljava/lang/Class;", ACC_PRIVATE },
    { "clazz", "Ljava/lang/Class;", ACC_PRIVATE },
};

builtins java_lang_reflect_Field_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .methods = methods,
    .fields_length = ARRAY_SIZE(fields),
    .methods_length = ARRAY_SIZE(methods),
};