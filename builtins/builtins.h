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
typedef struct Frame Frame;

typedef struct builtins {
    char *name;
    char *descriptor;
    builtin_method method;
    int max_stack;
    int max_local;
} builtins;

extern builtins java_lang_Object_methods[];
extern int java_lang_Object_methods_length;

extern builtins java_lang_System_methods[];
extern int java_lang_System_methods_length;

extern builtins java_io_PrintStream_methods[];
extern int java_io_PrintStream_methods_length;

extern builtins java_util_Objects_methods[];
extern int java_util_Objects_methods_length;

#endif