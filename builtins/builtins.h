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
    char *name;
    int flags;
    /* TODO: Maybe add descriptor too? */
} builtin_fields;

typedef struct builtin_methods {
    char *name;
    char *descriptor;
    int max_stack;
    builtin_method method;
} builtin_methods;

typedef struct builtins {
    char *parent;
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

extern builtins java_lang_String_builtins;

#endif