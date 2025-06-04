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
#include "attribute.h"
#include "constantpool.h"
#include "reader.h"
#include "stack.h"

typedef struct Attributes Attributes;
typedef struct ConstantPool ConstantPool;
typedef struct Method Method;

typedef struct FieldInfo {
    uint16_t access_flags;
    struct {
        uint16_t index;
        char *name;
    } name;
    struct {
        uint16_t index;
        char *descriptor;
    } descriptor;
    Attributes *attributes;
} FieldInfo;

typedef struct Fields {
    uint16_t count;
    FieldInfo *fields;
} Fields;

/* Helper functions */
extern FieldInfo fields_get_field(Fields *fields, char *name);

extern Fields *fields_new(Reader *reader, ConstantPool *pool);
extern void fields_free(Fields *fields);

/* Method execution */

/* Each frame is created whenever we execute a new method.
 * It consists of its own stack and local variables. These values
 * are cloned onto a new frame whenever a new method is executed.
 */

typedef struct Frame {
    int pc;
    int max_stack;
    int max_locals;
    Stack *stack;
    Stack *locals;
    uint8_t *code;
} Frame;

extern Frame *frame_new(int max_stack, int max_local);
extern void frame_free(Frame *frame);

/* TODO: Figure out built-in methods and methods associated with classes */
typedef void (*builtin_method)(Method *method, Frame *frame);

typedef struct Method {
    struct Class *class;
    char *name;
    uint16_t flags;
    uint32_t data_length;
    union {
        uint8_t *data;
        builtin_method method;
    };
    int max_stack;
    int max_local;
} Method;

typedef struct Class {
    struct Classes *classes;
    /* Each class has an associated Reader to read data */
    void *data;
    Reader *reader;

    uint16_t flags;
    char *name;
    struct Class *parent;
    bool built_in;

    /* Each class has its own constant pool, except built-ins */
    ConstantPool *pool;

    uint16_t methods_count;
    /* TODO: Maybe make this a single pointer? */
    Method **methods;

    /* These are not meant to be used by any functions except our own */
    Fields *fields;
    Fields *method_fields;
    Attributes *attributes;
} Class;

typedef struct Classes {
    uint16_t count;
    Class **classes;
    Class *main_class;
} Classes;

extern Class *class_parse_file(char *filename);
extern Class *class_create_builtin(char *name);
extern void class_free(Class *class);

extern void class_add_method(Class *class, FieldInfo method_info);
extern void class_add_builtin_method(Class *class, char *name, builtin_method bmethod);
extern Method *class_get_method(Class *class, char *name);
extern Method *class_get_method_from_index(Class *class, uint16_t index);

extern void classes_add_class(Classes *classes, Class *class);
extern Class *classes_get_class(Classes *classes, char *name);
extern Class *classes_get_class_from_index(Classes *classes, uint16_t index);

extern Method *classes_get_main_method(Classes *classes);

extern Classes *classes_new();
extern void classes_free(Classes *classes);

extern void method_execute(Method *method, Frame *frame);

#endif