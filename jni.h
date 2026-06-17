#ifndef JNI_H
#define JNI_H

#include <stdbool.h>
#include <stdarg.h>
#include "descriptor.h"

#define JNICALL

/* All the types */
typedef int jint;
typedef char jbyte;
typedef short jchar;
typedef short jshort;
typedef float jfloat;
typedef long jlong;
typedef double jdouble;
typedef bool jboolean;
typedef jint jsize;

typedef struct Object* jobject;
typedef jobject jclass;
typedef jobject jthrowable;
typedef jobject jstring;

struct _jmethodID;
struct _jfieldID;
typedef struct _jmethodID* jmethodID;
typedef struct _jfieldID* jfieldID;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

typedef const struct JNINativeInterface* JNIEnv;
typedef const struct JNIInvokeInterface* JavaVM;

typedef struct JNI {
    void **loaded_libs;
    struct JNINativeInterface *env;
    struct JNIInvokeInterface *vm;
} JNI;

typedef struct JNINativeInterface {
    JNI *jni;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    jint (*GetVersion)(JNIEnv *env);

    jclass (JNICALL *DefineClass)
      (JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
       jsize len);
    jclass (JNICALL *FindClass)
      (JNIEnv *env, const char *name);

    jmethodID (JNICALL *FromReflectedMethod)
      (JNIEnv *env, jobject method);
    jfieldID (JNICALL *FromReflectedField)
      (JNIEnv *env, jobject field);

    jobject (JNICALL *ToReflectedMethod)
      (JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic);

    jclass (JNICALL *GetSuperclass)
      (JNIEnv *env, jclass sub);
    jboolean (JNICALL *IsAssignableFrom)
      (JNIEnv *env, jclass sub, jclass sup);

    jobject (JNICALL *ToReflectedField)
      (JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic);

    jint (JNICALL *Throw)
      (JNIEnv *env, jthrowable obj);
    jint (JNICALL *ThrowNew)
      (JNIEnv *env, jclass clazz, const char *msg);
    jthrowable (JNICALL *ExceptionOccurred)
      (JNIEnv *env);
    void (JNICALL *ExceptionDescribe)
      (JNIEnv *env);
    void (JNICALL *ExceptionClear)
      (JNIEnv *env);
    void (JNICALL *FatalError)
      (JNIEnv *env, const char *msg);

    jint (JNICALL *PushLocalFrame)
      (JNIEnv *env, jint capacity);
    jobject (JNICALL *PopLocalFrame)
      (JNIEnv *env, jobject result);

    jobject (JNICALL *NewGlobalRef)
      (JNIEnv *env, jobject lobj);
    void (JNICALL *DeleteGlobalRef)
      (JNIEnv *env, jobject gref);
    void (JNICALL *DeleteLocalRef)
      (JNIEnv *env, jobject obj);
    jboolean (JNICALL *IsSameObject)
      (JNIEnv *env, jobject obj1, jobject obj2);
    jobject (JNICALL *NewLocalRef)
      (JNIEnv *env, jobject ref);
    jint (JNICALL *EnsureLocalCapacity)
      (JNIEnv *env, jint capacity);

    jobject (JNICALL *AllocObject)
      (JNIEnv *env, jclass clazz);
    jobject (JNICALL *NewObject)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject (JNICALL *NewObjectV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject (JNICALL *NewObjectA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jclass (JNICALL *GetObjectClass)
      (JNIEnv *env, jobject obj);
    jboolean (JNICALL *IsInstanceOf)
      (JNIEnv *env, jobject obj, jclass clazz);

    jmethodID (JNICALL *GetMethodID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject (JNICALL *CallObjectMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jobject (JNICALL *CallObjectMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jobject (JNICALL *CallObjectMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    jboolean (JNICALL *CallBooleanMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jboolean (JNICALL *CallBooleanMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jboolean (JNICALL *CallBooleanMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    jbyte (JNICALL *CallByteMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jbyte (JNICALL *CallByteMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jbyte (JNICALL *CallByteMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jchar (JNICALL *CallCharMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jchar (JNICALL *CallCharMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jchar (JNICALL *CallCharMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jshort (JNICALL *CallShortMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jshort (JNICALL *CallShortMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jshort (JNICALL *CallShortMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jint (JNICALL *CallIntMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jint (JNICALL *CallIntMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jint (JNICALL *CallIntMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jlong (JNICALL *CallLongMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jlong (JNICALL *CallLongMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jlong (JNICALL *CallLongMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jfloat (JNICALL *CallFloatMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jfloat (JNICALL *CallFloatMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jfloat (JNICALL *CallFloatMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    jdouble (JNICALL *CallDoubleMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    jdouble (JNICALL *CallDoubleMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    jdouble (JNICALL *CallDoubleMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args);

    void (JNICALL *CallVoidMethod)
      (JNIEnv *env, jobject obj, jmethodID methodID, ...);
    void (JNICALL *CallVoidMethodV)
      (JNIEnv *env, jobject obj, jmethodID methodID, va_list args);
    void (JNICALL *CallVoidMethodA)
      (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args);

    jobject (JNICALL *CallNonvirtualObjectMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jobject (JNICALL *CallNonvirtualObjectMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jobject (JNICALL *CallNonvirtualObjectMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue * args);

    jboolean (JNICALL *CallNonvirtualBooleanMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jboolean (JNICALL *CallNonvirtualBooleanMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jboolean (JNICALL *CallNonvirtualBooleanMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue * args);

    jbyte (JNICALL *CallNonvirtualByteMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jbyte (JNICALL *CallNonvirtualByteMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jbyte (JNICALL *CallNonvirtualByteMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jchar (JNICALL *CallNonvirtualCharMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jchar (JNICALL *CallNonvirtualCharMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jchar (JNICALL *CallNonvirtualCharMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jshort (JNICALL *CallNonvirtualShortMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jshort (JNICALL *CallNonvirtualShortMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jshort (JNICALL *CallNonvirtualShortMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jint (JNICALL *CallNonvirtualIntMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jint (JNICALL *CallNonvirtualIntMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jint (JNICALL *CallNonvirtualIntMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jlong (JNICALL *CallNonvirtualLongMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jlong (JNICALL *CallNonvirtualLongMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jlong (JNICALL *CallNonvirtualLongMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jfloat (JNICALL *CallNonvirtualFloatMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jfloat (JNICALL *CallNonvirtualFloatMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jfloat (JNICALL *CallNonvirtualFloatMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    jdouble (JNICALL *CallNonvirtualDoubleMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    jdouble (JNICALL *CallNonvirtualDoubleMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    jdouble (JNICALL *CallNonvirtualDoubleMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue *args);

    void (JNICALL *CallNonvirtualVoidMethod)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
    void (JNICALL *CallNonvirtualVoidMethodV)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       va_list args);
    void (JNICALL *CallNonvirtualVoidMethodA)
      (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
       const jvalue * args);

    jfieldID (JNICALL *GetFieldID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject (JNICALL *GetObjectField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jboolean (JNICALL *GetBooleanField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jbyte (JNICALL *GetByteField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jchar (JNICALL *GetCharField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jshort (JNICALL *GetShortField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jint (JNICALL *GetIntField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jlong (JNICALL *GetLongField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jfloat (JNICALL *GetFloatField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);
    jdouble (JNICALL *GetDoubleField)
      (JNIEnv *env, jobject obj, jfieldID fieldID);

    void (JNICALL *SetObjectField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jobject val);
    void (JNICALL *SetBooleanField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean val);
    void (JNICALL *SetByteField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jbyte val);
    void (JNICALL *SetCharField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jchar val);
    void (JNICALL *SetShortField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jshort val);
    void (JNICALL *SetIntField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jint val);
    void (JNICALL *SetLongField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jlong val);
    void (JNICALL *SetFloatField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jfloat val);
    void (JNICALL *SetDoubleField)
      (JNIEnv *env, jobject obj, jfieldID fieldID, jdouble val);

    jmethodID (JNICALL *GetStaticMethodID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);

    jobject (JNICALL *CallStaticObjectMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jobject (JNICALL *CallStaticObjectMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jobject (JNICALL *CallStaticObjectMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jboolean (JNICALL *CallStaticBooleanMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jboolean (JNICALL *CallStaticBooleanMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jboolean (JNICALL *CallStaticBooleanMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jbyte (JNICALL *CallStaticByteMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jbyte (JNICALL *CallStaticByteMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jbyte (JNICALL *CallStaticByteMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jchar (JNICALL *CallStaticCharMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jchar (JNICALL *CallStaticCharMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jchar (JNICALL *CallStaticCharMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jshort (JNICALL *CallStaticShortMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jshort (JNICALL *CallStaticShortMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jshort (JNICALL *CallStaticShortMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jint (JNICALL *CallStaticIntMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jint (JNICALL *CallStaticIntMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jint (JNICALL *CallStaticIntMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jlong (JNICALL *CallStaticLongMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jlong (JNICALL *CallStaticLongMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jlong (JNICALL *CallStaticLongMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jfloat (JNICALL *CallStaticFloatMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jfloat (JNICALL *CallStaticFloatMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jfloat (JNICALL *CallStaticFloatMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    jdouble (JNICALL *CallStaticDoubleMethod)
      (JNIEnv *env, jclass clazz, jmethodID methodID, ...);
    jdouble (JNICALL *CallStaticDoubleMethodV)
      (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args);
    jdouble (JNICALL *CallStaticDoubleMethodA)
      (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args);

    void (JNICALL *CallStaticVoidMethod)
      (JNIEnv *env, jclass cls, jmethodID methodID, ...);
    void (JNICALL *CallStaticVoidMethodV)
      (JNIEnv *env, jclass cls, jmethodID methodID, va_list args);
    void (JNICALL *CallStaticVoidMethodA)
      (JNIEnv *env, jclass cls, jmethodID methodID, const jvalue * args);

    jfieldID (JNICALL *GetStaticFieldID)
      (JNIEnv *env, jclass clazz, const char *name, const char *sig);
    jobject (JNICALL *GetStaticObjectField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jboolean (JNICALL *GetStaticBooleanField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jbyte (JNICALL *GetStaticByteField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jchar (JNICALL *GetStaticCharField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jshort (JNICALL *GetStaticShortField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jint (JNICALL *GetStaticIntField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jlong (JNICALL *GetStaticLongField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jfloat (JNICALL *GetStaticFloatField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);
    jdouble (JNICALL *GetStaticDoubleField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID);

    void (JNICALL *SetStaticObjectField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value);
    void (JNICALL *SetStaticBooleanField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value);
    void (JNICALL *SetStaticByteField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value);
    void (JNICALL *SetStaticCharField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value);
    void (JNICALL *SetStaticShortField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value);
    void (JNICALL *SetStaticIntField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jint value);
    void (JNICALL *SetStaticLongField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value);
    void (JNICALL *SetStaticFloatField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value);
    void (JNICALL *SetStaticDoubleField)
      (JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value);

    jstring (JNICALL *NewString)
      (JNIEnv *env, const jchar *unicode, jsize len);
    jsize (JNICALL *GetStringLength)
      (JNIEnv *env, jstring str);
    const jchar *(JNICALL *GetStringChars)
      (JNIEnv *env, jstring str, jboolean *isCopy);
    void (JNICALL *ReleaseStringChars)
      (JNIEnv *env, jstring str, const jchar *chars);

    jstring (JNICALL *NewStringUTF)
      (JNIEnv *env, const char *utf);
    jsize (JNICALL *GetStringUTFLength)
      (JNIEnv *env, jstring str);
    const char* (JNICALL *GetStringUTFChars)
      (JNIEnv *env, jstring str, jboolean *isCopy);
    void (JNICALL *ReleaseStringUTFChars)
      (JNIEnv *env, jstring str, const char* chars);
} JNINativeInterface;

typedef struct JNIInvokeInterface {
    JNI *jni;
    void *reserved1;
    void *reserved2;

    jint (*DestroyJavaVM)(JavaVM *vm);
    jint (*AttachCurrentThread)(JavaVM *vm, void **penv, void *args);
    jint (*DetachCurrentThread)(JavaVM *vm);
    jint (*GetEnv)(JavaVM *vm, void **penv, jint version);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args);
} JNIInvokeInterface;

JNI *jni_init();
void *jni_resolve_method(JNI *jni, const char* class_name, const char* method_name, Descriptors *descriptors);
void jni_load(JNI *jni, const char* file);
void jni_free(JNI *jni);

#endif // JNI_H