#include "builtins.h"

Object *java_lang_Runtime_getRuntime()
{
    Class *runtime_class = classes_get_class("java/lang/Runtime");
    return class_get_static_field(runtime_class, "INSTANCE")->value.data.object;
}

void java_lang_Runtime_loadLibrary(Object *this, Object *name)
{
    // Very crude resolution, but it works.
    const char format[] = "lib%s.so";
    char *lib_name;

    asprintf(&lib_name, format, object_get_field(name, "value")->value.data.ref);
    jni_load(get_jni(), lib_name);
    free(lib_name);
}

void java_lang_Runtime_clinit(Class *class)
{
    Object *instance = object_new(class);
    class_get_static_field(class, "INSTANCE")->value.data.object = instance;
}

static builtin_fields fields[] = {
    {
        "INSTANCE", "Ljava/lang/Runtime;", 0x0008
    }
};

static builtin_methods methods[] = {
    {
        "getRuntime", "()Ljava/lang/Runtime;", 0x0008, &java_lang_Runtime_getRuntime
    },
    {
        "loadLibrary", "(Ljava/lang/String;)V", 0x0000, &java_lang_Runtime_loadLibrary
    },
    {
        "<clinit>", "()V", 0x0008, &java_lang_Runtime_clinit
    }
};

builtins java_lang_Runtime_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};