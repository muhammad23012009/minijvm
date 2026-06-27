#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include "jni.h"
#include "dynarr.h"
#include "method.h"

jint JNI_GetEnv(JavaVM *vm, void **penv, jint version)
{
    printf("Got version request for version %d\n", version);
    *penv = &(*vm)->jni->env;
    return 0;
}

jint JNI_GetVersion(JNIEnv *env)
{
    return 0x00010008; // JNI version 1.8
}

jstring JNI_NewStringUTF(JNIEnv *env, const char *utf)
{
    Object *string = object_new(classes_get_class("java/lang/String"));
    string->initialized = true;
    object_set_field(string, "value", variant_make_ref(utf));
    return string;
}

JNI *jni_init()
{
    JNI *jni = malloc(sizeof(JNI));
    jni->loaded_libs = arr_init(sizeof(void*));
    jni->env = malloc(sizeof(JNINativeInterface));
    jni->vm = malloc(sizeof(JNIInvokeInterface));
    jni->env->jni = jni->vm->jni = jni;
    // Link the methods.

    jni->vm->GetEnv = JNI_GetEnv;

    jni->env->GetVersion = &JNI_GetVersion;
    jni->env->NewStringUTF = &JNI_NewStringUTF;

    return jni;
}

void jni_load(JNI *jni, const char* file)
{
    printf("Loading library: %s\n", file);
    void *lib = dlopen(file, RTLD_GLOBAL | RTLD_NOW);
    if (!lib)
    {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return;
    }

    arr_push(jni->loaded_libs, lib);
    printf("Lib ptr is %p\n", lib);
    void *onload = dlsym(lib, "JNI_OnLoad");
    if (onload)
    {
        printf("Found onload symbol at %p\n", onload);
    }

    // We'll resolve native functions lazily when they're called, so we don't need to do anything else here.
}

static char *mangle_object_name(const char *object_name)
{
    // Good assumption for now, since every underscore will get doubled
    char *mangled_name = malloc(strlen(object_name) * 2);
    char *mangled_name_ptr = mangled_name;
    char *start = NULL;

    memset(mangled_name, 0, strlen(object_name) * 2);

    while ((start = strchr(object_name, '/')))
    {
        mangled_name_ptr = stpncpy(mangled_name_ptr, object_name, start - object_name);
        mangled_name_ptr = stpncpy(mangled_name_ptr, "_1", 2);
        object_name = start + 1;
    }
    stpcpy(mangled_name_ptr, object_name);

    return mangled_name;
}

static void descriptors_to_native_str(Descriptors *descriptors, char **out_str)
{
    char *end_str = *out_str;

    Descriptor descriptor;
    FOREACH_DESCRIPTOR(descriptors, descriptor)
    {
        switch (descriptor.type)
        {
            case DESCRIPTOR_INT:
                end_str = stpcpy(end_str, "I");
                break;
            case DESCRIPTOR_OBJECT:
                end_str = stpcpy(end_str, "L");
                char *tmp = mangle_object_name(descriptor.object_name);
                end_str = stpcpy(end_str, tmp);
                end_str = stpcpy(end_str, "_2");
                free(tmp);
                break;
            case DESCRIPTOR_CHAR:
                end_str = stpcpy(end_str, "C");
                break;
            case DESCRIPTOR_BOOL:
                end_str = stpcpy(end_str, "Z");
                break;
            case DESCRIPTOR_VOID:
                end_str = stpcpy(end_str, "V");
                break;
            case DESCRIPTOR_FLOAT:
                end_str = stpcpy(end_str, "F");
                break;
            case DESCRIPTOR_DOUBLE:
                end_str = stpcpy(end_str, "D");
                break;
            case DESCRIPTOR_LONG:
                end_str = stpcpy(end_str, "J");
                break;
        }
    }
}

static void *find_symbol(void** libs, const char* symbol_name)
{
    for (int i = 0; i < arr_length(libs); ++i)
    {
        void *lib = libs[i];
        void *symbol = dlsym(lib, symbol_name);
        if (symbol)
        {
            return symbol;
        }
    }

    return NULL;
}

void *jni_resolve_method(JNI *jni, const char* class_name, const char* method_name, Descriptors *descriptors)
{
    // TODO: Implement a hashmap for storing libs with their name referenced inside the Java class
    char *symbol_name = NULL;
    const char format_overloaded[] = "Java_%s_%s__%s";
    const char format[] = "Java_%s_%s";
    char mangled_class_name[strlen(class_name) + 1];
    char *start = NULL;

    memset(mangled_class_name, 0, sizeof(mangled_class_name));

    char tmp_str[1024];
    char *meow = tmp_str;
    descriptors_to_native_str(descriptors, &meow);

    while ((start = strchr(class_name, '/')))
    {
        strncat(mangled_class_name, class_name, start - class_name);
        strcat(mangled_class_name, "_");
        class_name = start + 1;
    }
    strcat(mangled_class_name, class_name);

    // Check for the generic method first
    asprintf(&symbol_name, format, mangled_class_name, method_name);
    void *symbol = find_symbol(jni->loaded_libs, symbol_name);

    if (!symbol)
    {
        // Maybe the symbols are overloaded, let's try the overloaded format.
        char descriptor_str[1024];
        char *tmp = descriptor_str;
        memset(descriptor_str, 0, sizeof(descriptor_str));

        descriptors_to_native_str(descriptors, &tmp);
        asprintf(&symbol_name, format_overloaded, mangled_class_name, method_name, descriptor_str);
        symbol = find_symbol(jni->loaded_libs, symbol_name);
    }

    if (symbol)
    {
        free(symbol_name);
        return symbol;
    }

    printf("Did not find symbol %s in any loaded library, error %s\n", symbol_name, dlerror());
    free(symbol_name);
    return NULL;
}

void jni_free(JNI *jni)
{
    for (int i = 0; i < arr_length(jni->loaded_libs); ++i)
    {
        dlclose(jni->loaded_libs[i]);
    }
    arr_free(jni->loaded_libs);
    free(jni->env);
    free(jni->vm);
    free(jni);
}