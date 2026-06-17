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

#ifndef METHOD_H
#define METHOD_H

/* This handles both fields and methods, alongside converting the FieldInfo
 * type into our own custom Method type
 */

#include <stddef.h>
#include <sys/stat.h>
#include "minijvm.h"
#include "constantpool.h"
#include "reader.h"
#include "stack.h"
#include "descriptor.h"
#include "fields.h"

typedef struct Attributes Attributes;
typedef struct ConstantPool ConstantPool;
typedef struct Variant Variant;
typedef struct Method Method;
typedef struct Fields Fields;
typedef struct Field Field;
typedef struct builtins builtins;
 
/* Method execution */

/* Each frame is created whenever we execute a new method.
 * It consists of its own stack and local variables. These values
 * are cloned onto a new frame whenever a new method is executed.
 */

typedef struct Frame {
    struct Frame *caller;
    struct Method *method;
    volatile int pc;
    int max_stack;
    int max_locals;
    Stack *stack;
    Variant *locals;
    uint8_t *code;
} Frame;

extern Frame *get_current_frame();
extern Frame *frame_new(int max_stack, int max_local);
extern void frame_free(Frame *frame);

typedef struct Interface {
    uint16_t index;
    const char *interface;
} Interface;

typedef struct Class {
    struct Classes *classes;
    /* Each class has an associated Reader to read data */
    void *data;
    Reader *reader;

    uint16_t flags;
    const char *name;
    struct Class *parent;
    bool built_in;

    /* Each class has its own constant pool, except built-ins */
    ConstantPool *pool;

    Fields *fields;

    int methods_count;
    Method *methods;

    // All the interfaces this class implements, or extends.
    int interfaces_count;
    Interface *interfaces;

    bool static_initialized;

    /* These are not meant to be used by any functions except our own */
    Attributes *attributes;
} Class;

typedef struct Classes {
    uint16_t count;
    Class **classes;
    Class *main_class;
} Classes;

extern Class *class_parse_file(char *filename);
extern Class *class_create_builtin(char *name, builtins *class_builtins);
extern void class_initialize_static(Class *class);
extern void class_free(Class *class);

extern Method *class_get_method(Class *class, const char *name, const char *descriptor);
extern Method *class_get_method_from_index(Class *class, uint16_t index);
extern Field *class_get_static_field(Class *class, const char *name);
extern bool class_is_subclass(Class *child, Class *parent);

extern bool classes_add_class(Class *class);
extern Class *classes_get_class(const char *name);
extern Class *classes_get_class_from_index(ConstantPool *pool, uint16_t index);

extern Method *classes_get_main_method();

extern void classes_new();
extern void classes_free();

extern Variant call_native_method(Method *method, ...);
extern Variant method_execute(Method *method, ...);

#endif