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

Object *java_lang_Class_getPrimitiveClass(Object *string)
{
    return NULL;
}

Object *java_lang_Class_getDeclaredField(Object *this, Object *name)
{
    Class *reflect_class = classes_get_class("java/lang/reflect/Field");
    Class *class = object_get_field(this, "class")->value.data.class;
    const char *field_name = object_get_field(name, "value")->value.data.ref;

    printf("Getting declared field %s from class %s\n", field_name, class->name);

    for (int i = 0; i < class->fields->fields_count; i++) {
        Field *field = &class->fields->fields[i];
        printf("Found field %s in class %s\n", field->name, class->name);
        if (!strcmp(field->name, field_name)) {
            printf("yooooo, found the field %s in class %s, creating Field object\n", field_name, class->name);
            Object *field_obj = object_new(reflect_class);
            return java_lang_reflect_Field_init_custom(name, this);
        }
    }

    return NULL;
}

Object *java_lang_Class_forName_init(Object *name, bool initialize, Object *class_loader)
{
    Object* class_obj = object_new(classes_get_class("java/lang/Class"));
    char *class_name = strdup(object_get_field(name, "value")->value.data.ref);
    char *ptr = class_name;
    while (*ptr) {
        if (*ptr == '.')
            *ptr = '/';
        ptr++;
    }

    Class* class = classes_get_class(class_name);
    object_set_field(class_obj, "class", variant_make_ref(class));
    free(class_name);
    return class_obj;
}

Object *java_lang_Class_forName(Object *name)
{
    return java_lang_Class_forName_init(name, true, NULL);
}

bool java_lang_Class_desiredAssertionStatus(Object *this)
{
    return false;
}

Object *java_lang_Class_newInstance(Object *this)
{
    Object *object = object_new(this->fields[0].value.data.class);
    printf("New instance called on class %s, created object at pointer %p\n", this->fields[0].value.data.class->name, (void*)object);
    method_execute(class_get_method(this->fields[0].value.data.class, "<init>", "()V"), variant_make_object(object));
    return object;
}

builtin_fields java_lang_Class_fields[] = {
    /* Internal "Class" object pointer */
    { "class", "", 0x0000 },
};

builtin_methods java_lang_Class_methods[] = {
    { "getName", "()Ljava/lang/String;", 0x0000, &java_lang_Class_getName },
    { "getClassLoader", "()Ljava/lang/ClassLoader;", 0x0000, &java_lang_Class_getClassLoader },
    { "getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", ACC_STATIC, &java_lang_Class_getPrimitiveClass },
    { "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;", 0x0000, &java_lang_Class_getDeclaredField },
    { "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;", ACC_STATIC, &java_lang_Class_forName_init },
    { "forName", "(Ljava/lang/String;)Ljava/lang/Class;", ACC_STATIC, &java_lang_Class_forName },
    { "newInstance", "()Ljava/lang/Object;", 0x0000, &java_lang_Class_newInstance },
    { "desiredAssertionStatus", "()Z", 0x0000, &java_lang_Class_desiredAssertionStatus },
};

builtins java_lang_Class_builtins = {
    .parent = "java/lang/Object",
    .fields = java_lang_Class_fields,
    .fields_length = ARRAY_SIZE(java_lang_Class_fields),
    .methods = java_lang_Class_methods,
    .methods_length = ARRAY_SIZE(java_lang_Class_methods),
};