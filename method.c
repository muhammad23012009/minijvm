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

#include "method.h"
#include "object.h"

Frame *frame_new(int max_stack, int max_local)
{
    Frame *frame = malloc(sizeof(Frame));
    frame->max_stack = max_stack;
    frame->max_locals = max_local;

    frame->stack = stack_new(max_stack);
    frame->locals = stack_new(max_local);

    return frame;
}

void frame_free(Frame *frame)
{
    stack_free(frame->stack);
    stack_free(frame->locals);
    free(frame);
}

void method_execute(Method *method, Frame *frame)
{
    /* Access code of method */
    uint8_t *data = method->data;
    ConstantPool *pool = method->class->pool;

    /* TODO: Implement proper subframes and built-in methods */
    /* Also implement proper local variables */
    /* Also implement automatic argument parsing for methods using method descriptors */

    frame->pc = 0;
    frame->code = data;
    while (frame->pc != method->data_length) {
        uint8_t op = data[frame->pc];
        switch (op) {
            case 18: { // ldc
                uint8_t index = data[++frame->pc];
                StackData data;
                data.ref = constant_pool_resolve_string(pool, index);
                stack_push(frame->stack, STACK_ITEM_TYPE_REF, data);
                break;
            }
            case 42:
            case 43:
            case 44:
            case 45: { // aload_x
                uint8_t local_index = op - 42;
                StackItem *item = &frame->locals[local_index];
                stack_push(frame->stack, item->type, item->data);
                break;
            }
            case 87: { // dup
                stack_dup(frame->stack);
                break;
            }
            case 178: { // getstatic
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                uint16_t class_index = pool->pool[index - 1].method_ref.class_index;
                Class *class = classes_get_class_from_index(method->class->classes, class_index);
                Method *class_method;
                if (class->built_in) {
                    /* We don't have a constant pool. Parse the method manually */
                    uint16_t method_index = pool->pool[index - 1].method_ref.name_and_type_index;
                    method_index = pool->pool[method_index - 1].name_and_type_info.name_index;
                    char *method_name = constant_pool_resolve_string(pool, method_index);
                    class_method = class_get_method(class, method_name);
                } else {
                    class_method = class_get_method_from_index(method->class, index);
                }

                class_method->method(method, frame);
                break;
            }
            case 182: { // invokevirtual
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                uint16_t class_index = pool->pool[index - 1].method_ref.class_index;
                Class *class = classes_get_class_from_index(method->class->classes, class_index);
                Method *class_method;
                if (class->built_in) {
                    /* We don't have a constant pool. Parse the method manually */
                    uint16_t method_index = pool->pool[index - 1].method_ref.name_and_type_index;
                    method_index = pool->pool[method_index - 1].name_and_type_info.name_index;
                    char *method_name = constant_pool_resolve_string(pool, method_index);
                    class_method = class_get_method(class, method_name);
                } else {
                    class_method = class_get_method_from_index(method->class, index);
                }

                class_method->method(method, frame);
                break;
            }
            case 183: { // invokespecial
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                uint16_t class_index = pool->pool[index - 1].method_ref.class_index;
                Class *class = classes_get_class_from_index(method->class->classes, class_index);
                Method *class_method = class_get_method_from_index(class, index);
                printf("invokespecial: got names %s and %s\n", class->name, class_method->name);

                Frame *subframe = frame_new(class_method->max_stack, class_method->max_local);
                /* Class method, push ref into local variables */
                StackData data;
                data.ref = class;
                stack_push(subframe->locals, STACK_ITEM_TYPE_REF, data);
                method_execute(class_method, subframe);

                break;
            }
            case 187: { // new
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                Class *class = classes_get_class_from_index(method->class->classes, index);
                Object *object = object_new(class);
                StackData data;
                data.object = object;
                stack_push(frame->stack, STACK_ITEM_TYPE_OBJECT, data);
                break;
            }
        }
        printf("Opcode is %d\n", op);
        frame->pc++;
    }
    /* TODO: Clean up pointers on stack and frames */
}

/* Field creation/accession methods */
FieldInfo fields_get_field(Fields *fields, char *name)
{
    for (int i = 0; i < fields->count; i++) {
        FieldInfo info = fields->fields[i];
        if (!strcmp(info.name.name, name))
            return info;
    }
}

Fields *fields_new(Reader *reader, ConstantPool *pool)
{
    Fields *fields = malloc(sizeof(Fields));
    fields->count = reader_read_uint16_be(reader);
    if (fields->count < 0) {
        free(fields);
        return NULL;
    }

    fields->fields = malloc(sizeof(FieldInfo) * fields->count);

    for (int i = 0; i < fields->count; i++) {
        FieldInfo *field = &fields->fields[i];
        field->access_flags = reader_read_uint16_be(reader);
        field->name.index = reader_read_uint16_be(reader);
        field->name.name = constant_pool_resolve_string(pool, field->name.index);
        field->descriptor.index = reader_read_uint16_be(reader);
        field->descriptor.descriptor = constant_pool_resolve_string(pool, field->descriptor.index);
        field->attributes = attributes_new(reader, pool);
    }

    return fields;
}

void fields_free(Fields *fields)
{
    if (!fields)
        return;

    for (int i = 0; i < fields->count; i++) {
        FieldInfo info = fields->fields[i];
        attributes_free(info.attributes);
    }
    free(fields->fields);
    free(fields);
}

/* Class/methods code */
Class *class_parse_file(char *filename)
{
    Class *class = malloc(sizeof(Class));
    struct stat filestat;
    FILE *file = fopen(filename, "r");

    stat(filename, &filestat);
    class->data = malloc(filestat.st_size);
    fread(class->data, filestat.st_size, 1, file);
    fclose(file);

    class->built_in = false;
    Reader *reader = class->reader = reader_new(class->data, filestat.st_size);

    /* Start parsing the classfile */
    uint32_t magic = reader_read_uint32_be(reader);
    if (magic != 0xCAFEBABE) {
        printf("Invalid class!\n");
        return NULL;
        /* TODO: Cleanup... */
    }

    uint16_t minor_version = reader_read_uint16_be(reader);
    uint16_t major_version = reader_read_uint16_be(reader);
    class->pool = constant_pool_new(reader);
    class->flags = reader_read_uint16_be(reader);
    class->name = constant_pool_resolve_string(class->pool, reader_read_uint16_be(reader));
    /* TODO: Handle parents later */
    reader_skip(reader, 2);

    uint16_t interfaces_count = reader_read_uint16_be(reader);
    Interface *interfaces = malloc(sizeof(Interface) * interfaces_count);

    for (int i = 0; i < interfaces_count; i++) {
        Interface *iface = &interfaces[i];
        iface->index = reader_read_uint16_be(reader);
        iface->interface = constant_pool_resolve_string(class->pool, iface->index);
        printf("Parsed interface at index %d with string %s\n", iface->index, iface->interface);
    }

    /* TODO: Eventually drop these somehow */
    class->fields = fields_new(reader, class->pool);
    class->method_fields = fields_new(reader, class->pool);
    class->attributes = attributes_new(reader, class->pool);

    for (int i = 0; i < class->method_fields->count; i++) {
        FieldInfo info = class->method_fields->fields[i];
        class_add_method(class, info);
    }

    return class;
}

/* Built-in classes will have no constant pools or any other associated
 * properties set. They will only contains a name and methods.
*/
Class *class_create_builtin(char *name)
{
    Class *class = malloc(sizeof(Class));
    class->built_in = true;
    class->name = name;
    class->methods_count = 0;

    return class;
}

void class_add_method(Class *class, FieldInfo method_info)
{
    Method *method = malloc(sizeof(Method));
    AttributeInfo code = attributes_get_attribute(method_info.attributes, "Code");

    method->name = method_info.name.name;
    method->class = class;
    method->data_length = code.CodeAttribute.code_length;
    method->data = code.CodeAttribute.code;
    method->flags = method_info.access_flags;
    method->max_stack = code.CodeAttribute.max_stack;
    method->max_local = code.CodeAttribute.max_locals;

    if (class->methods)
        class->methods = realloc(class->methods, (sizeof(Method*) * (class->methods_count + 1)));
    else
        class->methods = malloc(sizeof(Method*));

    class->methods[class->methods_count] = method;
    class->methods_count++;
}

void class_add_builtin_method(Class *class, char *name, builtin_method bmethod)
{
    if (!class->built_in) {
        printf("Adding built-in method to normal class!\n");
        return;
    }

    Method *method = malloc(sizeof(Method));
    method->name = name;
    method->class = class;
    method->method = bmethod;
    method->flags = 0; // TODO: Implement method flags

    if (class->methods)
        class->methods = realloc(class->methods, (sizeof(Method*) * (class->methods_count + 1)));
    else
        class->methods = malloc(sizeof(Method*));

    class->methods[class->methods_count++] = method;
}

Method *class_get_method(Class *class, char *name)
{
    for (int i = 0; i < class->methods_count; i++) {
        Method *method = class->methods[i];
        if (!strcmp(method->name, name))
            return method;
    }

    return NULL;
}

/* Gets class method from index. Index is expected to be of type `method_ref` */
Method *class_get_method_from_index(Class *class, uint16_t index)
{
    uint16_t method_index = class->pool->pool[index - 1].method_ref.name_and_type_index;
    method_index = class->pool->pool[method_index - 1].name_and_type_info.name_index;
    char *name = constant_pool_resolve_string(class->pool, method_index);
    return class_get_method(class, name);
}

void classes_add_class(Classes *classes, Class *class)
{
    class->classes = classes;
    classes->classes = realloc(classes->classes, (sizeof(Class*) * (classes->count + 1)));
    classes->classes[classes->count] = class;
    classes->count++;
}

Class *classes_get_class(Classes *classes, char *name)
{
    for (int i = 0; i < classes->count; i++) {
        Class *class = classes->classes[i];
        if (!strcmp(class->name, name))
            return class;
    }

    return NULL;
}

Class *classes_get_class_from_index(Classes *classes, uint16_t index)
{
    char *name = constant_pool_resolve_string(classes->main_class->pool, index);
    return classes_get_class(classes, name);
}

Method *classes_get_main_method(Classes *classes)
{
    for (int i = 0; i < classes->count; i++) {
        Class *class = classes->classes[i];
        for (int j = 0; j < class->methods_count; j++) {
            Method *method = class->methods[j];
            if (!strcmp(method->name, "main")) {
                // We found the main method. Great. Mark this as our main class.
                classes->main_class = class;
                return class->methods[j];
            }
        }
    }

    return NULL;
}

Classes *classes_new()
{
    Classes *classes = malloc(sizeof(Classes));
    classes->count = 0;
    classes->classes = NULL;
}

void classes_free(Classes *classes)
{
    for (int i = 0; i < classes->count; i++) {
        Class *class = classes->classes[i];

        if (!class->built_in) {
            attributes_free(class->attributes);
            fields_free(class->fields);
            fields_free(class->method_fields);
            constant_pool_free(class->pool);
            free(class->reader);
            free(class->data);
        }

        for (int j = 0; j < class->methods_count; j++) {
            free(class->methods[j]);
        }

        free(class->methods);        
    }
    free(classes->classes);
    free(classes);
}