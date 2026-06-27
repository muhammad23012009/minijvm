/* 
 * This file is part of MiniJVM (https://github.com/muhammad23012009/minijvm)
 * Copyright (c) 2025 Muhammad  <thevancedgamer@mentallysanemainliners.org>
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#include "minijvm.h"
#include "reader.h"
#include "builtins/builtins.h"
#include "gc.h"
#include "dynarr.h"
#include "thread.h"

static jit_context_t s_jit_context;
static JNI *s_jni;

void destroy_dynamic_methods();

static char *help_text = "miniJVM: a stupidly simple JVM. \n\
Usage: ./miniJVM <class name>\n";

jit_context_t get_jit_context()
{
    return s_jit_context;
}

JNI *get_jni()
{
    return s_jni;
}

uint64_t get_current_time()
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    return current_time.tv_sec * 1000 + current_time.tv_nsec / 1000000;
}

void porcodio()
{
    // dump the entire stacktrace
    printf("Segmentation fault! Stack trace:\n");
    Frame *last_frame = get_current_frame();
    int i = 0;
    while (last_frame) {
        printf("\t#%d: %s::%s, (PC %d)\n", i, last_frame->method->class->name, last_frame->method->name, last_frame->pc);
        last_frame = last_frame->caller;
        i++;
    }

    exit(1);
}

static void start_main(struct arguments *args)
{
    Variant ret = method_execute(args->method, args->arg);

    // how do we handle return from main?
    free(args);
}

int main(int argc, char *argv[])
{
    pthread_t main_thread;

    if (argc < 2) {
        fprintf(stderr, "miniJVM: invalid arguments!\n");
        fprintf(stderr, "%s", help_text);
        return 1;
    }

    // Install a fault handler and print stacktrace
    struct sigaction sa;
    sa.sa_handler = porcodio;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);

    // Setup JIT
    s_jit_context = jit_context_create();
    s_jni = jni_init();
    // Load our own symbols as well.
    jni_load(s_jni, NULL);

    char filename[2048];
    snprintf(filename, 2048, "%s%s", argv[1], ".class");

    thread_list_init();

    classes_new();

    gc_create();

    /* Setup built-in classes and methods */
    classes_add_class(class_create_builtin("java/lang/Object", &java_lang_Object_builtins));
    classes_add_class(class_create_builtin("java/util/Objects", &java_util_Objects_builtins));
    //classes_add_class(class_create_builtin("java/io/PrintStream", &java_io_PrintStream_builtins));
    //classes_add_class(class_create_builtin("java/lang/System", &java_lang_System_builtins));
    classes_add_class(class_create_builtin("java/lang/String", &java_lang_String_builtins));
    classes_add_class(class_create_builtin("java/lang/Class", &java_lang_Class_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/CallSite", &java_lang_invoke_CallSite_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/StringConcatFactory", &java_lang_invoke_StringConcatFactory_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/MethodHandles", &java_lang_invoke_MethodHandles_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/MethodHandles$Lookup", &java_lang_invoke_MethodHandles_lookup_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/MethodHandle", &java_lang_invoke_MethodHandle_builtins));
    classes_add_class(class_create_builtin("java/lang/invoke/MethodType", &java_lang_invoke_MethodType_builtins));
    //classes_add_class(class_create_builtin("java/io/InputStream", &java_io_InputStream_builtins));
    //classes_add_class(class_create_builtin("java/io/StdinInputStream", &java_io_StdinInputStream_builtins));
    classes_add_class(class_create_builtin("java/lang/reflect/Field", &java_lang_reflect_Field_builtins));
    classes_add_class(class_create_builtin("java/lang/Throwable", &java_lang_Throwable_builtins));
    classes_add_class(class_create_builtin("java/lang/ArrayIndexOutOfBoundsException", &java_lang_ArrayIndexOutOfBoundsException_builtins));
    classes_add_class(class_create_builtin("sun/misc/Unsafe", &sun_misc_Unsafe_builtins));
    classes_add_class(class_create_builtin("java/lang/Runtime", &java_lang_Runtime_builtins));

    if (!classes_add_class(class_parse_file(filename))) {
        classes_free();
        return 1;
    }

    printf("miniJVM: all classes processed!\n");

    Method *main_method = classes_get_main_method();
    if (!main_method) {
        fprintf(stderr, "Failed to find main method. Exiting!\n");
        classes_free();
        return 1;
    }

    // Initialize uhhhh
    Method *something = class_get_method(classes_get_class("java/lang/System"), "initializeSystemClass", "()V");
    method_execute(something);

    Object **main_args = arr_init_with_capacity_and_type(sizeof(Object*), argc - 2, ARRAY_TYPE_OBJECT, classes_get_class("java/lang/String"));
    for (int i = 2; i < argc; i++) {
        Object *str_obj = object_new(classes_get_class("java/lang/String"));
        object_set_field(str_obj, "value", variant_make_ref(argv[i]));
        arr_push(main_args, str_obj);
    }

    Variant main_args_variant = variant_make_ref(main_args);
    struct arguments *args = malloc(sizeof(struct arguments));
    args->method = main_method;
    args->arg = main_args_variant;

    pthread_create(&main_thread, NULL, (void*(*)(void*))&start_main, args);
    pthread_join(main_thread, NULL);

    // Wait for all the threads to die
    printf("Everything returned from main, now waiting for all threads to die...\n");
    thread_list_wait();

    classes_free();
    destroy_dynamic_methods();
    jni_free(s_jni);
    gc_free();

    jit_context_destroy(s_jit_context);

    return 0;
}
