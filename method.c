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

#include <avcall.h>

#include "builtins/builtins.h"
#include "array.h"
#include "method.h"
#include "object.h"
#include "gc.h"

/* TODO: 
 * Implement exceptions
 * Improve garbage collection
 * Improve method and class lookups with hashmaps
 * Improve object creation with a global list of classes
 * Improve object field accession and value assignments
 * Implement overloaded methods for superclasses
 * Implement a proper types system
 * Figure out how to represent stored constants, do we represent them as objects or something else?
 * Does Java have a stack/heap system for variables?
 * ...etc
 */

#define DISPATCH()          \
    ++frame->pc;            \
    op = data[frame->pc];   \
    goto *opcodes[op]       \

Frame *frame_new(int max_stack, int max_local)
{
    Frame *frame = malloc(sizeof(Frame));
    frame->max_stack = max_stack;
    frame->max_locals = max_local;

    frame->stack = stack_new(max_stack);
    frame->locals = calloc(max_local, sizeof(Variant));
    memset(frame->locals, 0, sizeof(Variant) * max_local);

    gc_track_frame(frame);

    return frame;
}

void frame_free(Frame *frame)
{
    gc_untrack_frame(frame);

    stack_free(frame->stack);
    free(frame->locals);
    free(frame);
}

Method *get_method(ConstantPool *pool, Class *class, uint16_t index)
{
    Method *class_method;

    uint16_t name_and_type_index = pool->pool[index].method_ref.name_and_type_index;
    uint16_t method_index = pool->pool[name_and_type_index].name_and_type_info.name_index;
    uint16_t descriptor_index = pool->pool[name_and_type_index].name_and_type_info.descriptor_index;
    const char *method_name = constant_pool_resolve_string(pool, method_index);
    const char *descriptor = constant_pool_resolve_string(pool, descriptor_index);

    class_method = class_get_method(class, method_name, descriptor);

    return class_method;
}

static inline VariantType descriptor_to_variant_type(DescriptorType type)
{
    switch (type) {
        case DESCRIPTOR_INT:
            return VARIANT_TYPE_INT;
        case DESCRIPTOR_OBJECT:
            return VARIANT_TYPE_OBJECT;
        default:
            return VARIANT_TYPE_NONE;
    }
}

void call_native_method(Method *method, Frame *frame)
{
    /* Convert all of the arguments from the method's descriptors to native arguments */
    av_alist list;
    Variant return_value;
    int j = 1;

    if (!strcmp(method->name, "<clinit>")) {
        /* Static initializer */
        av_start_void(list, (void(*)())method->native_method);
        av_ptr(list, Class *, method->class);
        goto call;
    }

    /* Check return type of the native method. */
    switch (DESCRIPTORS_GET_RETURN_TYPE(method->descriptors))
    {
        case DESCRIPTOR_VOID:
            av_start_void(list, (void(*)())method->native_method);
        case DESCRIPTOR_INT:
            av_start_int(list, (int(*)())method->native_method, &return_value.data.int_val);
            return_value.type = VARIANT_TYPE_INT;
        case DESCRIPTOR_OBJECT:
            av_start_ptr(list, (Object*(*)())method->native_method, Object *, &return_value.data.object);
            return_value.type = VARIANT_TYPE_OBJECT;
    }

    /* If the function isn't static, push the "this" pointer */
    if (!(method->info.access_flags & 0x0008))
        av_ptr(list, Object *, frame->locals[0].data.object);

    FOREACH_DESCRIPTOR(method->descriptors) {
        switch (descriptor.type) {
            case DESCRIPTOR_INT:
                av_int(list, frame->locals[j].data.int_val);
            case DESCRIPTOR_OBJECT:
                av_ptr(list, Object *, frame->locals[j].data.object);
        }
        j++;
    }

call:
    av_call(list);

    if (DESCRIPTORS_GET_RETURN_TYPE(method->descriptors) != DESCRIPTOR_VOID)
        stack_push(frame->stack, return_value);
}

void method_execute(Method *method, Frame *frame)
{
    printf("Beginning execution of method %s\n", method->name);
    /* Access code of method */
    uint8_t *data = method->data;
    ConstantPool *pool = method->class->pool;
    uint8_t op;

    static void *opcodes[] = {
        [2 ... 8] = &&iconst_x,
        [16] = &&bipush,
        [17] = &&sipush,
        [18] = &&ldc,
        [21] = &&iload,
        [25] = &&aload,
        [26 ... 29] = &&iload_x,
        [42 ... 45] = &&aload_x,
        [50] = &&aaload,
        [54] = &&istore,
        [58] = &&astore,
        [59 ... 62] = &&istore_x,
        [75 ... 78] = &&astore_x,
        [83] = &&aastore,
        [87] = &&pop,
        [89] = &&dup,
        [96] = &&iadd,
        [108] = &&idiv,
        [132] = &&iinc,
        [159 ... 164] = &&if_cmpx,
        [167] = &&j_goto,
        [172] = &&ireturn,
        [177] = &&j_return,
        [178] = &&getstatic,
        [179] = &&putstatic,
        [180] = &&getfield,
        [181] = &&putfield,
        [182] = &&invokevirtual,
        [183] = &&invokespecial,
        [184] = &&invokestatic,
        [186] = &&invokedynamic,
        [187] = &&new,
        [189] = &&anewarray,
        [190] = &&arraylength,
    };

    frame->pc = 0;
    frame->code = data;

    op = data[frame->pc];
    goto *opcodes[op];

    iconst_x: {
        int8_t const_int = op - 3;
        stack_push_int(frame->stack, const_int);
        DISPATCH();
    }

    bipush: {
        uint8_t byte = data[++frame->pc];
        stack_push_int(frame->stack, byte);
        DISPATCH();
    }

    sipush:
        uint16_t shrt = (data[++frame->pc] << 8) | data[++frame->pc];
        stack_push_int(frame->stack, shrt);
        DISPATCH();

    ldc: {
        uint8_t index = data[++frame->pc];
        uint8_t tag = constant_pool_get_tag(pool, index);
        Variant variant;

        switch (tag) {
            case CONSTANT_CLASS: {
                Class *class = classes_get_class_from_index(method->class->classes, pool, index);
                variant.data.class = class;
                variant.type = VARIANT_TYPE_CLASS;
                break;
            }

            case CONSTANT_INT: {
                variant.data.int_val = constant_pool_resolve_int(pool, index);
                variant.type = VARIANT_TYPE_INT;
                break;
            }

            case CONSTANT_STRING: {
                Object *str_obj = object_new(classes_get_class(method->class->classes, "java/lang/String"));
                object_get_field(str_obj, "value")->value.data.ref = constant_pool_resolve_string(pool, index);
                variant.data.object = str_obj;
                variant.type = VARIANT_TYPE_OBJECT;
                break;
            }

            case CONSTANT_UTF8: {
                variant.data.ref = constant_pool_resolve_string(pool, index);
                variant.type = VARIANT_TYPE_REF;
                break;
            }
        }

        stack_push(frame->stack, variant);
        DISPATCH();
    }

    iload: {
        uint8_t index = data[++frame->pc];
        stack_push(frame->stack, frame->locals[index]);
        DISPATCH();
    }

    aload: {
        uint8_t index = data[++frame->pc];
        stack_push(frame->stack, frame->locals[index]);
        DISPATCH();
    }

    iload_x: {
        uint8_t local_index = op - 26;
        Variant item = frame->locals[local_index];
        stack_push(frame->stack, item);
        DISPATCH();
    }

    aload_x: {
        uint8_t local_index = op - 42;
        stack_push(frame->stack, frame->locals[local_index]);
        DISPATCH();
    }

    aaload: {
        int index = stack_pop(frame->stack).data.int_val;
        Array *array = stack_pop(frame->stack).data.ref;

        stack_push(frame->stack, array->value[index]);
        DISPATCH();
    }

    istore: {
        uint8_t local_index = data[++frame->pc];
        frame->locals[local_index] = stack_pop(frame->stack);
        DISPATCH();
    }

    astore: {
        uint8_t local_index = data[++frame->pc];
        frame->locals[local_index] = stack_pop(frame->stack);
        DISPATCH();
    }

    istore_x: {
        uint8_t local_index = op - 59;
        frame->locals[local_index] = stack_pop(frame->stack);
        DISPATCH();
    }

    astore_x: {
        uint8_t local_index = op - 75;
        frame->locals[local_index] = stack_pop(frame->stack);
        DISPATCH();
    }

    aastore: {
        Variant value = stack_pop(frame->stack);
        int index = stack_pop(frame->stack).data.int_val;
        Array *array = stack_pop(frame->stack).data.ref;

        array_set_value(array, index, value);
        DISPATCH();
    }

    pop:
        stack_pop(frame->stack);
        DISPATCH();

    dup:
        stack_dup(frame->stack);
        DISPATCH();

    iadd:
        int a1 = stack_pop(frame->stack).data.int_val;
        int a2 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, a1 + a2);
        DISPATCH();

    idiv:
        int d2 = stack_pop(frame->stack).data.int_val;
        int d1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, d1 / d2);
        DISPATCH();

    iinc:
        uint8_t index = data[++frame->pc];
        int8_t const_val = data[++frame->pc];
        frame->locals[index].data.int_val += const_val;
        DISPATCH();

    if_cmpx: {
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
            DISPATCH();

        do_ne:
            if (value1 != value2) goto branch;
            DISPATCH();

        do_lt:
            if (value1 < value2) goto branch;
            DISPATCH();

        do_ge:
            if (value1 >= value2) goto branch;
            DISPATCH();

        do_gt:
            if (value1 > value2) goto branch;
            DISPATCH();

        do_le:
            if (value1 <= value2) goto branch;
            DISPATCH();

        branch:
            frame->pc += branch_offset - 3;
            DISPATCH();
    }

    j_goto: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        /* Two bytes for the branch offset, and one byte for DISPATCH */
        frame->pc += branch_offset - 3;
        DISPATCH();
    }

    ireturn:
        return;

    j_return:
        return;

    getstatic: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        if (class->fields->static_fields_count && !class->static_initialized) {
            class_initialize_static(class);
        }

        const char *field_name = constant_pool_resolve_field_name(pool, index);
        Field *field = class_get_static_field(class, field_name);

        stack_push(frame->stack, field->value);
        DISPATCH();
    }

    putstatic: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        /* Check if the class has been initialized yet. */
        if (class->fields->static_fields_count && !class->static_initialized) {
            class_initialize_static(class);
        }

        Field *field = class_get_static_field(class, constant_pool_resolve_field_name(class->pool, index));

        /* TODO: Implement value conversion */
        field->value = stack_pop(frame->stack);
        if (field->value.type == VARIANT_TYPE_OBJECT) {
            field->value.data.object->parent_field = field;
        }
        DISPATCH();
    }

    getfield: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Object *object = stack_pop(frame->stack).data.object;
        const char *field_name = constant_pool_resolve_field_name(object->class->pool, index);
        Field *field = object_get_field(object, field_name);

        stack_push(frame->stack, field->value);
        DISPATCH();
    }

    putfield: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value = stack_pop(frame->stack);
        Object *object = stack_pop(frame->stack).data.object;

        const char *field_name = constant_pool_resolve_field_name(object->class->pool, index);
        Field *field = object_get_field(object, field_name);

        field->value = value;
        if (value.type == VARIANT_TYPE_OBJECT) {
            value.data.object->parent_field = field;
        }
        DISPATCH();
    }

    invokevirtual: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        Method *class_method = get_method(pool, class, index);

        Frame *subframe = frame_new(class_method->max_stack, class_method->max_local);

        int arguments_count = class_method->descriptors->arguments_count;

        for (int i = 1; i <= arguments_count; i++) {
            Variant item = stack_pop(frame->stack);
            subframe->locals[i] = item;
        }
        Variant item = stack_pop(frame->stack);
        subframe->locals[0] = item;

        if (class->built_in) {
            call_native_method(class_method, subframe);
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
        DISPATCH();
    }

    invokespecial: {
        /* TODO: https://docs.oracle.com/javase/specs/jvms/se14/html/jvms-6.html#jvms-6.5.invokespecial 
         * Implement all of this
         */

        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        Method *class_method = get_method(pool, class, index);

        Frame *subframe = frame_new(class_method->max_stack, class_method->max_local);
        //printf("made new subframe with max stack %d\n", class_method->max_stack);

        int arguments_count = class_method->descriptors->arguments_count;

        for (int i = 1; i <= arguments_count; i++) {
            Variant item = stack_pop(frame->stack);
            subframe->locals[i] = item;
        }

        Variant item = stack_pop(frame->stack);
        subframe->locals[0] = item;

        if (class->built_in) {
            class->pool = pool;
            call_native_method(class_method, subframe);
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
        DISPATCH();
    }

    invokestatic: {
        
    }

    invokedynamic: {
        DISPATCH();
    }

    new: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        Object *object = object_new(class);
        stack_push_object(frame->stack, object);
        DISPATCH();
    }

    anewarray: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(method->class->classes, pool, index);
        printf("Creating new array of class %s\n", class->name);

        int count = stack_pop(frame->stack).data.int_val;
        Array *array = array_new(class, count);
        stack_push_ref(frame->stack, array);

        DISPATCH();
    }

    arraylength: {
        Array *array = stack_pop(frame->stack).data.ref;
        stack_push_int(frame->stack, array->count);

        DISPATCH();
    }
}

/* Class/methods code */
Class *class_parse_file(Classes *classes, char *filename)
{
    Class *class = malloc(sizeof(Class));
    Method *static_init = NULL;
    struct stat filestat;
    FILE *file;
    memset(class, 0, sizeof(Class));

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

    printf("class processing started!\n");

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
    class->parent = classes_get_class(classes, constant_pool_resolve_string(class->pool, reader_read_uint16_be(reader)));

    printf("some basic information...\n");

    uint16_t interfaces_count = reader_read_uint16_be(reader);
    Interface *interfaces = malloc(sizeof(Interface) * interfaces_count);

    for (int i = 0; i < interfaces_count; i++) {
        Interface *iface = &interfaces[i];
        iface->index = reader_read_uint16_be(reader);
        iface->interface = constant_pool_resolve_string(class->pool, iface->index);
    }

    free(interfaces); // TODO: Implement interfaces

    /* TODO: Eventually drop these somehow */
    class->fields = fields_new(class, reader, class->pool);
    class->methods = methods_new(class, reader, class->pool, &class->methods_count);
    class->attributes = attributes_new(reader, class->pool);
    class->static_initialized = false;

    printf("fields and methods...\n");

    if (!constant_pool_resolve_unknowns(class->pool, classes, class)) {
        class_free(class);
        return NULL;
    }

    printf("...and done!\n");
    return class;
}

void class_initialize_static(Class *class)
{
    Method *static_init = NULL;

    if (class->fields->static_fields_count && (static_init = class_get_method(class, "<clinit>", "()V"))) {
        class->static_initialized = true;
        Frame *frame = frame_new(static_init->max_stack, static_init->max_local);

        if (class->built_in)
            call_native_method(static_init, frame);
        else
            method_execute(static_init, frame);

        frame_free(frame);
    }
}

void class_free(Class *class)
{
    if (!class->built_in) {
        attributes_free(class->attributes);
        fields_free(class->fields);
        methods_free(class->methods, class->methods_count);
        constant_pool_free(class->pool);
        free(class->reader);
        free(class->data);
    }
    free(class);
}

/* Built-in classes will have no constant pools or any other associated
 * properties set. They will only contains a name and methods.
*/
Class *class_create_builtin(char *name, builtins *class_builtins, Classes *classes)
{
    Class *class = malloc(sizeof(Class));
    memset(class, 0, sizeof(Class));
    class->built_in = true;
    class->name = name;

    if (class_builtins->parent)
        class->parent = classes_get_class(classes, class_builtins->parent);

    class->methods_count = class_builtins->methods_length;
    class->methods = methods_new_builtin(class, class_builtins);
    class->fields = fields_new_builtin(class, class_builtins);

    class->static_initialized = false;

    return class;
}

Method *class_get_method(Class *class, const char *name, const char *descriptor)
{
    for (int i = 0; i < class->methods_count; i++) {
        Method *method = &class->methods[i];
        if (!strcmp(method->name, name) && !strcmp(method->descriptors->descriptor, descriptor))
            return method;
    }

    return NULL;
}

Field *class_get_static_field(Class *class, const char *name)
{
    for (int i = 0; i < class->fields->static_fields_count; i++) {
        Field *f = &class->fields->static_fields[i];
        if (!strcmp(name, f->name))
            return f;
    }

    return NULL;
}

/* Gets class method from index. Index is expected to be of type `method_ref` */
Method *class_get_method_from_index(Class *class, uint16_t index)
{
    uint16_t method_index = class->pool->pool[index].method_ref.name_and_type_index;
    uint16_t name_index = class->pool->pool[method_index].name_and_type_info.name_index;
    uint16_t descriptor_index = class->pool->pool[method_index].name_and_type_info.descriptor_index;

    const char *name = constant_pool_resolve_string(class->pool, name_index);
    const char *descriptor = constant_pool_resolve_string(class->pool, descriptor_index);
    return class_get_method(class, name, descriptor);
}

bool classes_add_class(Classes *classes, Class *class)
{
    if (!classes || !class)
        return false;

    class->classes = classes;
    classes->classes = realloc(classes->classes, (sizeof(Class*) * (classes->count + 1)));
    classes->classes[classes->count++] = class;

    return true;
}

Class *classes_get_class(Classes *classes, const char *name)
{
    if (!classes || !name)
        return NULL;

    for (int i = 0; i < classes->count; i++) {
        Class *class = classes->classes[i];
        if (!strcmp(class->name, name))
            return class;
    }

    return NULL;
}

/* TODO: Fix this method */
Class *classes_get_class_from_index(Classes *classes, ConstantPool *pool, uint16_t index)
{
    const char *name = constant_pool_resolve_class_name(pool, index);
    return classes_get_class(classes, name);
}

Method *classes_get_main_method(Classes *classes)
{
    for (int i = 0; i < classes->count; i++) {
        Class *class = classes->classes[i];
        for (int j = 0; j < class->methods_count; j++) {
            Method *method = &class->methods[j];
            if (!strcmp(method->name, "main")) {
                // We found the main method. Great. Mark this as our main class.
                printf("Found method main in class %s\n", class->name);
                classes->main_class = class;
                return &class->methods[j];
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

    return classes;
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
