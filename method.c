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

/* TODO: 
 * Implement exceptions
 * Implement proper garbage collection
 * Implement overloaded methods (for both built-in and parsed methods) and superclasses
 * Free up objects created in a method when it returns
 * Clean up everything on stacks and locals when we exit
 * Implement objects and fields properly
 * Implement init functions for built-in classes to create objects and initialize fields, maybe somehow store the list of objects too?
 * Improve built-in APIs/classes
 * Improve stack operations (push, pop, etc)
 * Implement arrays
 * Implement a proper types system
 * Figure out how to represent stored constants, do we represent them as objects or something else?
 * Does Java have a stack/heap system for variables?
 * ...etc
 */

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

Method *get_method(ConstantPool *pool, Class *class, uint16_t index, char **descriptor)
{
    Method *class_method;

    uint16_t name_and_type_index = pool->pool[index - 1].method_ref.name_and_type_index;
    uint16_t method_index = pool->pool[name_and_type_index - 1].name_and_type_info.name_index;
    uint16_t descriptor_index = pool->pool[name_and_type_index - 1].name_and_type_info.descriptor_index;
    char *method_name = constant_pool_resolve_string(pool, method_index);
    if (class->built_in)
        *descriptor = constant_pool_resolve_string(pool, descriptor_index);

    class_method = class_get_method(class, method_name);

    return class_method;
}

void method_execute(Method *method, Frame *frame)
{
    //printf("Beginning execution of method %s\n", method->name);
    /* Access code of method */
    uint8_t *data = method->data;
    ConstantPool *pool = method->class->pool;

    frame->pc = 0;
    frame->code = data;
    while (frame->pc != method->data_length) {
        uint8_t op = data[frame->pc];
        switch (op) {
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8: { // iconst_x
                /* These ops range from -1 to 5 */
                int8_t value = op - 3;
                stack_push_int(frame->stack, value);
                break;
            }
            case 16: { // bipush
                uint8_t byte = data[++frame->pc];
                stack_push_int(frame->stack, byte);
                break;
            }
            case 17: { // sipush
                uint16_t shrt = (data[++frame->pc] << 8) | data[++frame->pc];
                stack_push_int(frame->stack, shrt);
                break;
            }
            case 18: { // ldc
                uint8_t index = data[++frame->pc];
                uint8_t tag = constant_pool_get_tag(pool, index);
                Variant variant;

                switch (tag) {
                    case CONSTANT_INT: {
                        variant.data.int_val = constant_pool_resolve_int(pool, index);
                        variant.type = VARIANT_TYPE_INT;
                        break;
                    }
                    case CONSTANT_STRING:
                    case CONSTANT_UTF8: {
                        variant.data.ref = constant_pool_resolve_string(pool, index);
                        variant.type = VARIANT_TYPE_REF;
                        break;
                    }
                }

                stack_push(frame->stack, variant);
                break;
            }
            case 21: { // iload
                uint8_t index = data[++frame->pc];
                stack_push(frame->stack, frame->locals->items[index]);
                break;
            }
            case 26:
            case 27:
            case 28:
            case 29: { // iload_x
                uint8_t local_index = op - 26;
                Variant item = frame->locals->items[local_index];
                stack_push(frame->stack, item);
                break;
            }
            case 42:
            case 43:
            case 44:
            case 45: { // aload_x
                uint8_t local_index = op - 42;
                Variant item = frame->locals->items[local_index];
                stack_push(frame->stack, item);
                break;
            }
            case 54: { // istore
                uint8_t local_index = data[++frame->pc];
                frame->locals->items[local_index] = stack_pop(frame->stack);
                break;
            }
            case 59:
            case 60:
            case 61:
            case 62: { // istore_x
                uint8_t local_index = op - 59;
                frame->locals->items[local_index] = stack_pop(frame->stack);
                break;
            }
            case 75:
            case 76:
            case 77:
            case 78: { // astore_x
                uint8_t local_index = op - 75;
                frame->locals->items[local_index] = stack_pop(frame->stack);
                break;
            }
            case 87: { // pop
                stack_pop(frame->stack);
                break;
            }
            case 89: { // dup
                stack_dup(frame->stack);
                break;
            }
            case 96: { // iadd
                int a1 = stack_pop(frame->stack).data.int_val;
                int a2 = stack_pop(frame->stack).data.int_val;
                stack_push_int(frame->stack, a1 + a2);
                break;
            }
            case 132: { // iinc
                uint8_t index = data[++frame->pc];
                int8_t const_val = data[++frame->pc];
                frame->locals->items[index].data.int_val += const_val;
                break;
            }
            case 167: { // goto
                int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
                /* Skip two bytes to offset the bytes we just read */
                frame->pc += branch_offset - 2;
                continue;
            }
            case 159: // if_icmpeq
            case 160: // if_icmpne
            case 161: // if_icmplt
            case 162: // if_icmpge
            case 163: // if_icmpgt
            case 164: // if_icmple
            {
                uint8_t cond = op - 159;
                int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
                int value2 = stack_pop(frame->stack).data.int_val;
                int value1 = stack_pop(frame->stack).data.int_val;

                static void *conditions[] = {
                    &&do_eq, &&do_ne, &&do_lt, &&do_ge, &&do_gt, &&do_le,
                };

                goto *conditions[cond];

                do_eq:
                    if (value1 == value2) goto branch;
                    break;

                do_ne:
                    if (value1 != value2) goto branch;
                    break;

                do_lt:
                    if (value1 < value2) goto branch;
                    break;

                do_ge:
                    if (value1 >= value2) goto branch;
                    break;

                do_gt:
                    if (value1 > value2) goto branch;
                    break;

                do_le:
                    if (value1 <= value2) goto branch;
                    break;

                branch:
                    frame->pc += branch_offset - 2;
                    continue;
            }
            case 172: { // ireturn
                /* TODO: Cleanup? */
                return;
            }
            case 177: { // return
                /* TODO: Cleanup */
                return;
            }
            case 178: { // getstatic
                char *descriptor_str;

                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                char *class_name = constant_pool_resolve_class_name(pool, index);
                Class *class = classes_get_class(method->class->classes, class_name);
                Method *class_method = get_method(pool, class, index, &descriptor_str);

                class_method->method(method, frame, descriptor_str);
                break;
            }
            case 180: { // getfield
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                Object *object = stack_pop(frame->stack).data.object;
                char *field_name = constant_pool_resolve_field_name(object->class->pool, index);
                Field *field = object_get_field(object, field_name);

                stack_push(frame->stack, field->value);
                break;
            }
            case 181: { // putfield
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                Variant value = stack_pop(frame->stack);
                Object *object = stack_pop(frame->stack).data.object;

                char *field_name = constant_pool_resolve_field_name(object->class->pool, index);
                Field *field = object_get_field(object, field_name);

                field->value = value;
                break;
            }
            case 182: { // invokevirtual
                char *descriptor_str;

                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                char *class_name = constant_pool_resolve_class_name(pool, index);
                Class *class = classes_get_class(method->class->classes, class_name);
                Method *class_method = get_method(pool, class, index, &descriptor_str);

                uint16_t max_stack, max_local;
                if (class->built_in) {
                    /* TODO: Implement static functions so we can drop this extra stack arg */
                    /* Each built-in method gets a reference to its own class */
                    max_stack = get_descriptor_count(descriptor_str);
                    max_local = max_stack + 1;
                } else {
                    max_stack = class_method->max_stack;
                    max_local = class_method->max_local;
                }

                Frame *subframe = frame_new(max_stack, max_local);
                //printf("made new subframe for submethod %s in class %s with max stack %d\n", class_method->name, class->name, max_local);

                int arguments_count = class->built_in ?
                                    (class_method->descriptors ? class_method->descriptors->arguments_count : get_descriptor_count(descriptor_str)) :
                                    class_method->descriptors->arguments_count;

                for (int i = 1; i <= arguments_count; i++) {
                    Variant item = stack_pop(frame->stack);
                    subframe->locals->items[i] = item;
                }
                Variant item = stack_pop(frame->stack);
                subframe->locals->items[0] = item;

                if (class->built_in) {
                    class_method->method(method, subframe, descriptor_str);
                } else {
                    method_execute(class_method, subframe);
                }

                //printf("finished execution of submethod %s\n", class_method->name);
                /* TODO: Instead of doing this, pass the frame of the invoker 
                 * into the method being executed, and then push the result into
                 * the invoker frame on return.
                */
                /* Take the returned value from the method, if it had one */
                if (class_method->descriptors &&
                    class_method->descriptors->return_descriptor.type != DESCRIPTOR_VOID) {
                    Variant item = stack_pop(subframe->stack);
                    stack_push(frame->stack, item);
                }

                frame_free(subframe);
                break;
            }
            case 183: { // invokespecial
                /* TODO: https://docs.oracle.com/javase/specs/jvms/se14/html/jvms-6.html#jvms-6.5.invokespecial 
                 * Implement all of this
                 */

                char *descriptor_str;

                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];

                Class *class = classes_get_class_from_index(method->class->classes, index);
                Method *class_method = get_method(pool, class, index, &descriptor_str);
                uint16_t max_stack, max_local;
                if (class->built_in) {
                    /* TODO: Implement static functions so we can drop this extra stack arg */
                    /* Each built-in method gets a reference to its own class */
                    max_stack = get_descriptor_count(descriptor_str);
                    max_local = max_stack + 1;
                } else {
                    max_stack = class_method->max_stack;
                    max_local = class_method->max_local;
                }

                Frame *subframe = frame_new(max_stack, max_local);
                //printf("made new subframe with max stack %d\n", max_stack);

                int arguments_count = class->built_in ?
                                    (class_method->descriptors ? class_method->descriptors->arguments_count : get_descriptor_count(descriptor_str)) :
                                    class_method->descriptors->arguments_count;

                for (int i = 1; i <= arguments_count; i++) {
                    Variant item = stack_pop(frame->stack);
                    subframe->locals->items[i] = item;
                }

                Variant item = stack_pop(frame->stack);
                subframe->locals->items[0] = item;

                if (class->built_in) {
                    class->pool = pool;
                    class_method->method(class_method, subframe, descriptor_str);
                } else {
                    method_execute(class_method, subframe);
                }

                /* Take the returned value from the method, if it had one */
                if (class_method->descriptors &&
                    class_method->descriptors->return_descriptor.type != DESCRIPTOR_VOID) {
                    Variant item = stack_pop(subframe->stack);
                    printf("got return\n");
                    stack_push(frame->stack, item);
                }

                frame_free(subframe);

                break;
            }
            case 184: { // invokestatic
                // skip for now
                frame->pc += 2;
                break;
            }
            case 187: { // new
                uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
                Class *class = classes_get_class_from_index(method->class->classes, index);
                Object *object = object_new(class);
                stack_push_object(frame->stack, object);
                break;
            }
            case 189: { // anewarray
                /* TODO: Implement arrays somehow */
            }
        }
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
    if (fields->count <= 0) {
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
Class *class_parse_file(Classes *classes, char *filename)
{
    Class *class = malloc(sizeof(Class));
    struct stat filestat;
    FILE *file;

    int status = stat(filename, &filestat);
    if (status < 0) {
        printf("File not found!\n");
        free(class);
        return NULL;
    }

    file = fopen(filename, "r");
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
    }

    /* TODO: Eventually drop these somehow */
    class->class_fields = fields_new(reader, class->pool);
    class->method_fields = fields_new(reader, class->pool);
    class->attributes = attributes_new(reader, class->pool);

    for (int i = 0; i < class->method_fields->count; i++) {
        FieldInfo info = class->method_fields->fields[i];
        class_add_method(class, info);
    }

    if (!constant_pool_resolve_unknowns(class->pool, classes, class)) {
        class_free(class);
        return NULL;
    }

    return class;
}

void class_free(Class *class)
{
    if (!class->built_in) {
        attributes_free(class->attributes);
        fields_free(class->class_fields);
        fields_free(class->method_fields);
        constant_pool_free(class->pool);
        free(class->reader);
        free(class->data);
    }

    for (int j = 0; j < class->methods_count; j++) {
        descriptors_free(class->methods[j]->descriptors);
        free(class->methods[j]);
    }

    free(class->methods);
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
    method->descriptors = descriptors_new(method_info.descriptor.descriptor);

    if (class->methods)
        class->methods = realloc(class->methods, (sizeof(Method*) * (class->methods_count + 1)));
    else
        class->methods = malloc(sizeof(Method*));

    class->methods[class->methods_count] = method;
    class->methods_count++;
}

void class_add_builtin_method(Class *class, char *name, char *descriptor_str, builtin_method bmethod)
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

    if (strcmp(descriptor_str, "") != 0) {
        /* This built-in method has predefined arguments and returns */
        method->descriptors = descriptors_new(descriptor_str);
    }

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

bool classes_add_class(Classes *classes, Class *class)
{
    if (!class)
        return false;

    class->classes = classes;
    classes->classes = realloc(classes->classes, (sizeof(Class*) * (classes->count + 1)));
    classes->classes[classes->count] = class;
    classes->count++;

    return true;
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

/* TODO: Fix this method */
Class *classes_get_class_from_index(Classes *classes, uint16_t index)
{
    char *name = constant_pool_resolve_class_name(classes->main_class->pool, index);
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
        class_free(class);
    }
    free(classes->classes);
    free(classes);
}