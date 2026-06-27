#define _GNU_SOURCE

#include "builtins/builtins.h"
#include "dynarr.h"
#include "jni.h"
#include "method.h"
#include "thread.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <signal.h>

thread_local jobject s_current_thread = NULL;

[[gnu::used]] jobject Java_java_security_AccessController_getStackAccessControlContext(JNIEnv *env)
{
    printf("Called Java_java_security_AccessController_getStackAccessControlContext\n");
    return NULL;
}

[[gnu::used]] jobject Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedAction_2(JNIEnv *env, jclass, jobject action)
{
    printf("Called Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedAction_2\n");
    Object *object = (Object*)action;
    Variant result = method_execute(class_get_method(object->class, "run", "()Ljava/lang/Object;"), variant_make_object(object));

    return result.data.object;
}

[[gnu::used]] jobject Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedExceptionAction_2(JNIEnv *env, jclass, jobject action)
{
    printf("Called Java_java_security_AccessController_doPrivileged__Ljava_1security_1PrivilegedExceptionAction_2\n");
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

[[gnu::used]] void Java_java_lang_Thread_registerNatives(JNIEnv *env)
{
    printf("Called Java_java_lang_Thread_registerNatives\n");
}

[[gnu::used]] jobject Java_java_lang_Thread_currentThread(JNIEnv *env)
{
    if (!s_current_thread) {
        Object *thread_obj = object_new(classes_get_class("java/lang/Thread"));
        Object *threadgroup_obj = object_new(classes_get_class("java/lang/ThreadGroup"));
        object_set_field(thread_obj, "group", variant_make_object(threadgroup_obj));
        object_set_field(thread_obj, "tid", variant_make_long(pthread_self()));
        object_set_field(thread_obj, "priority", variant_make_int(getpriority(PRIO_PROCESS, gettid()) + 5));

        s_current_thread = thread_obj;

        // Call the constructor for ThreadGroup
        Method *threadgroup_constructor = class_get_method(threadgroup_obj->class, "<init>", "()V");
        method_execute(threadgroup_constructor, variant_make_object(threadgroup_obj));

        // and for Thread
        Method *thread_constructor = class_get_method(thread_obj->class, "<init>", "()V");
        method_execute(thread_constructor, variant_make_object(thread_obj));
    }

    return s_current_thread;
}

[[gnu::used]] void Java_java_lang_Thread_yield(JNIEnv *env)
{
    sched_yield();
}

[[gnu::used]] void Java_java_lang_Thread_sleep(JNIEnv *env, jclass, jlong millis)
{
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

static void begin_method(struct arguments *args)
{
    pthread_cleanup_push((void (*)(void*))&java_lang_Object_notifyAll, args->arg.data.object);

    if (!s_current_thread) {
        s_current_thread = args->arg.data.object;
    }

    Variant ret = method_execute(args->method, args->arg);
    thread_list_remove(object_get_field((Object*)args->arg.data.object, "tid")->value.data.long_val);

    free(args);

    pthread_exit(NULL);

    pthread_cleanup_pop(0);
}

[[gnu::used]] void Java_java_lang_Thread_start0(JNIEnv *env, jobject this)
{
    Object *thread_obj = (Object*)this;
    Method *run_method = class_get_method(thread_obj->class, "run", "()V");
    pthread_t thread;

    struct arguments *args = malloc(sizeof(struct arguments));
    args->method = run_method;
    args->arg = variant_make_object(thread_obj);

    pthread_create(&thread, NULL, (void*(*)(void*))&begin_method, args);

    ThreadInfo *threadInfo = thread_list_add(thread);

    object_set_field(thread_obj, "tid", variant_make_long(threadInfo->id));

    pthread_detach(thread);
}

[[gnu::used]] void Java_java_lang_Thread_setPriority0(JNIEnv *env, jobject this, jint newPriority)
{
    Object *thread_obj = (Object*)this;
    object_set_field(thread_obj, "priority", variant_make_int(newPriority));
    setpriority(PRIO_PROCESS, gettid(), newPriority - 5);
}

[[gnu::used]] bool Java_java_lang_Thread_isAlive(JNIEnv *env, jobject this)
{
    printf("Checking if thread is alive\n");
    Object *thread_obj = (Object*)this;
    uint64_t tid = object_get_field(thread_obj, "tid")->value.data.long_val;
    ThreadInfo *threadInfo = thread_list_get(tid);
    return threadInfo != NULL && threadInfo->state != THREAD_STATE_TERMINATED;
}

[[gnu::used]] jobject Java_java_lang_reflect_Array_newArray(JNIEnv *env, jclass componentType, jint length)
{
    Class *component_class = object_get_field((Object*)componentType, "class")->value.data.class;
    Object **array = arr_init_with_capacity_and_type(sizeof(Object*), length, ARRAY_TYPE_OBJECT, component_class->component_type);
    Object *array_obj = object_new(classes_get_class("[Ljava/lang/Object;"));
    array_obj->array = array;
    return array_obj;
}

[[gnu::used]] void Java_java_lang_ClassLoader_registerNatives(JNIEnv *env)
{
    printf("Called Java_java_lang_ClassLoader_registerNatives\n");
}

[[gnu::used]] void Java_java_lang_System_registerNatives(JNIEnv *env)
{
    printf("Called Java_java_lang_System_registerNatives\n");
}

static Object* string_to_java_str(const char* str)
{
    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
    object_set_field(str_obj, "value", variant_make_owned_ref(strdup(str)));
    return str_obj;
}

#define PUTPROP(key, value) \
    do { \
        Object *key_obj = string_to_java_str(key); \
        Object *value_obj = string_to_java_str(value); \
        method_execute(put_method, variant_make_object(props), variant_make_object(key_obj), variant_make_object(value_obj)); \
    } while (0)

[[gnu::used]] void Java_java_lang_System_initProperties(JNIEnv *env, jclass, jobject props)
{
    Method *put_method = class_get_method(props->class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    PUTPROP("java.version", "1.8.0");
    PUTPROP("file.separator", "/");
    PUTPROP("path.separator", ":");
    PUTPROP("line.separator", "\n");
    PUTPROP("file.encoding", "ISO8859-1");
}

[[gnu::used]] void Java_java_io_FileDescriptor_initIDs(JNIEnv *env)
{
    printf("Called Java_java_io_FileDescriptor_initIDs\n");
}

[[gnu::used]] int Java_sun_reflect_Reflection_getClassAccessFlags(JNIEnv *env, jclass, jobject clazz)
{
    Class *class = object_get_field((Object*)clazz, "class")->value.data.class;
    return class->flags;
}


[[gnu::used]] void Java_java_lang_System_setIn0(JNIEnv *env, jclass cla, jobject in)
{
    Object *in_obj = (Object*)in;
    Class *class = (Class*)cla;

    class_get_static_field(class, "in")->value = variant_make_object(in_obj);
}

[[gnu::used]] void Java_java_lang_System_setOut0(JNIEnv *env, jclass cla, jobject out)
{
    Object *out_obj = (Object*)out;
    Class *class = (Class*)cla;

    class_get_static_field(class, "out")->value = variant_make_object(out_obj);    
}

[[gnu::used]] void Java_java_lang_System_setErr0(JNIEnv *env, jclass cla, jobject err)
{
    Object *err_obj = (Object*)err;
    Class *class = (Class*)cla;

    class_get_static_field(class, "err")->value = variant_make_object(err_obj);
}

[[gnu::used]] void Java_java_lang_System_arraycopy(JNIEnv *env, jclass, jobject src, jint srcPos, jobject dest, jint destPos, jint length)
{
    Object *srcobj = (Object*)src;
    Object *destobj = (Object*)dest;

    if (!srcobj || !destobj) {
        fprintf(stderr, "NullPointerException in System.arraycopy\n");
        exit(1);
    }

    void *src_array = (Object**)srcobj->array;
    void *dst_array = (Object**)destobj->array;

    if (arr_type(src_array) != arr_type(dst_array)) {
        fprintf(stderr, "ArrayStoreException in System.arraycopy: incompatible array types\n");
        exit(1);
    }

    //printf("Performing arraycopy from src %p at position %d to dest %p at position %d with length %d\n", (void*)src_array, srcPos, (void*)dst_array, destPos, length);
    if (destPos + length > arr_capacity(dst_array) || srcPos + length > arr_capacity(src_array)) {
        fprintf(stderr, "IndexOutOfBoundsException in System.arraycopy: source or destination position out of bounds\n");
        exit(1);
    }

    // ew what is this
    for (int i = 0; i < length; ++i) {
        if (arr_type(src_array) == ARRAY_TYPE_OBJECT) {
            ((Object**)dst_array)[destPos + i] = ((Object**)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_INT) {
            ((int*)dst_array)[destPos + i] = ((int*)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_LONG) {
            ((int64_t*)dst_array)[destPos + i] = ((int64_t*)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_FLOAT) {
            ((float*)dst_array)[destPos + i] = ((float*)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_DOUBLE) {
            ((double*)dst_array)[destPos + i] = ((double*)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_CHAR) {
            ((uint16_t*)dst_array)[destPos + i] = ((uint16_t*)src_array)[srcPos + i];
        } else if (arr_type(src_array) == ARRAY_TYPE_BYTE) {
            ((uint8_t*)dst_array)[destPos + i] = ((uint8_t*)src_array)[srcPos + i];
        }
    }
}

[[gnu::used]] jobject Java_java_lang_System_mapLibraryName(JNIEnv *env, jclass, jstring libname)
{
    Object *libname_obj = (Object*)libname;
    const char *name = (const char*)object_get_field(libname_obj, "value")->value.data.ref;
    char *mapped_name = malloc(strlen(name) + 7);
    sprintf(mapped_name, "lib%s.so", name);

    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
    object_set_field(str_obj, "value", variant_make_owned_ref(mapped_name));
    return str_obj;
}

[[gnu::used]] int Java_sun_misc_Signal_findSignal(JNIEnv *env, jclass, jstring name)
{
    Object *name_obj = (Object*)name;
    const char *signal_name = (const char*)object_get_field(name_obj, "value")->value.data.ref;

    if (!strcmp(signal_name, "INT")) {
        return SIGINT;
    } else if (!strcmp(signal_name, "TERM")) {
        return SIGTERM;
    } else if (!strcmp(signal_name, "HUP")) {
        return SIGHUP;
    } else if (!strcmp(signal_name, "QUIT")) {
        return SIGQUIT;
    } else if (!strcmp(signal_name, "USR1")) {
        return SIGUSR1;
    } else if (!strcmp(signal_name, "USR2")) {
        return SIGUSR2;
    } else if (!strcmp(signal_name, "ALRM")) {
        return SIGALRM;
    } else if (!strcmp(signal_name, "CHLD")) {
        return SIGCHLD;
    } else if (!strcmp(signal_name, "CONT")) {
        return SIGCONT;
    } else if (!strcmp(signal_name, "STOP")) {
        return SIGSTOP;
    } else if (!strcmp(signal_name, "TSTP")) {
        return SIGTSTP;
    } else if (!strcmp(signal_name, "TTIN")) {
        return SIGTTIN;
    } else if (!strcmp(signal_name, "TTOU")) {
        return SIGTTOU;
    }
}

static void *dispatch_signal(void *signum_ptr)
{
    int signum = (int)(intptr_t)signum_ptr;
    printf("Dispatching signal %d to Java\n", signum);

    Class *signal_class = classes_get_class("sun/misc/Signal");
    Method *handle_method = class_get_method(signal_class, "dispatch", "(I)V");
    method_execute(handle_method, variant_make_int(signum));
    return NULL;
}

static void signal_handler(int signum)
{
    pthread_t thread;
    pthread_create(&thread, NULL, &dispatch_signal, (void*)(intptr_t)signum);
    thread_list_add(thread);

    pthread_detach(thread);
}

[[gnu::used]] long Java_sun_misc_Signal_handle0(JNIEnv *env, jclass, jint num, long action)
{
    struct sigaction sa, prev_sa;

    if (num == -1) {
        fprintf(stderr, "Unknown signal: %d\n", num);
        exit(1);
    }

    // We handle this already, and we won't let Java have it
    if (num == SIGSEGV)
        return -1;

    sigaction(num, NULL, &prev_sa);

    if (action == 0) {
        signal(num, SIG_DFL);
    } else if (action == 1) {
        signal(num, SIG_IGN);
    } else if (action == 2) {
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(num, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }
    }

    return (long)prev_sa.sa_handler;
}

[[gnu::used]] void Java_java_io_FileOutputStream_writeBytes(JNIEnv *env, jobject this, jobject byteArray, jint offset, jint length, jboolean append)
{
    Object *byte_array_obj = (Object*)byteArray;
    uint8_t *bytes = (uint8_t*)byte_array_obj->array;
    Object *fd_obj = object_get_field((Object*)this, "fd")->value.data.object;
    int fd = object_get_field(fd_obj, "fd")->value.data.int_val;

    if (append)
        lseek(fd, 0, SEEK_END);

    write(fd, bytes + offset, length);
}

[[gnu::used]] void Java_java_io_FileOutputStream_initIDs(JNIEnv *env)
{
    printf("Called Java_java_io_FileOutputStream_initIDs\n");
}