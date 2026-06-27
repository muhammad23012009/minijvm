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

#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include "../method.h"
#include "../variant.h"
#include "../dynarr.h"

/* TODO: 
 * Add proper support for fields
 * Add flags for methods (static, etc)
 * Add stack and locals sizes
 */

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

typedef struct Method Method;
typedef struct Field Field;
typedef struct Frame Frame;

/* The class can access the field itself by accessing it directly */
typedef struct builtin_fields {
    const char *name;
    const char *descriptor;
    int flags;
} builtin_fields;

typedef struct builtin_methods {
    const char *name;
    const char *descriptor;
    int flags;
    void *method;
} builtin_methods;

typedef struct builtins {
    const char *parent;
    builtin_fields *fields;
    int fields_length;
    builtin_methods *methods;
    int methods_length;

    int max_stack;
    int max_local;
} builtins;

extern builtins java_lang_Object_builtins;

extern builtins java_lang_System_builtins;

extern builtins java_io_PrintStream_builtins;

extern builtins java_util_Objects_builtins;

extern builtins java_lang_invoke_StringConcatFactory_builtins;
extern builtins java_lang_invoke_CallSite_builtins;
extern builtins java_lang_invoke_MethodHandles_lookup_builtins;
extern builtins java_lang_invoke_MethodHandles_builtins;
extern builtins java_lang_invoke_MethodHandle_builtins;
extern builtins java_lang_invoke_MethodType_builtins;

extern Object *java_lang_invoke_CallSite_create(jit_function_t target);
extern Object *java_lang_invoke_MethodHandle_create(jit_function_t method);
extern Object *java_lang_reflect_Field_init_custom(Object *name, Object *declaring_class, Object *clazz);
extern void java_lang_Object_notifyAll(Object *this);
extern Object *getClassObject(Class *class);
extern Object *java_lang_String_new(const char *str);

extern builtins java_lang_String_builtins;
extern builtins java_lang_Class_builtins;
extern builtins java_lang_reflect_Field_builtins;

extern builtins java_io_InputStream_builtins;
extern builtins java_io_StdinInputStream_builtins;
extern builtins sun_misc_Unsafe_builtins;

extern builtins java_lang_Throwable_builtins;
extern builtins java_lang_ArrayIndexOutOfBoundsException_builtins;

extern builtins java_lang_Runtime_builtins;

#endif