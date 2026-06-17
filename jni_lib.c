#include "jni.h"
#include "method.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

[[gnu::used]] jobject Java_java_security_AccessController_getStackAccessControlContext(JNIEnv *env)
{
    printf("Called Java_java_security_AccessController_getStackAccessControlContext\n");
    return NULL;
}

[[gnu::used]] jobject Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedAction_2(JNIEnv *env, jobject action)
{
    printf("Called Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedAction_2\n");
    Object *object = (Object*)action;
    Variant result = method_execute(class_get_method(object->class, "run", "()Ljava/lang/Object;"), variant_make_object(object));

    return result.data.object;
}

[[gnu::used]] jobject Java_sun_reflect_Reflection_getCallerClass__(JNIEnv *env)
{
    Object *class_obj = object_new(classes_get_class("java/lang/Class"));
    object_set_field(class_obj, "class", variant_make_ref(get_current_frame()->caller->method->class));
    return class_obj;
}

[[gnu::used]] jint Java_java_lang_Float_floatToRawIntBits(JNIEnv *env, jfloat value)
{
    union {
        float f;
        uint32_t i;
    } u = { .f = value };

    return (jint)u.i;
}

[[gnu::used]] jlong Java_java_lang_Double_doubleToRawLongBits(JNIEnv *env, jdouble value)
{
    union {
        double d;
        jlong i;
    } u = { .d = value };

    return (jlong)u.i;
}

[[gnu::used]] jdouble Java_java_lang_Double_longBitsToDouble(JNIEnv *env, jlong bits)
{
    union {
        double d;
        jlong i;
    } u = { .i = bits };

    return u.d;
}

[[gnu::used]] void Java_sun_misc_VM_initialize(JNIEnv *env)
{
    printf("Called Java_sun_misc_VM_initialize\n");
}