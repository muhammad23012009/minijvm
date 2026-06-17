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

#include <stddef.h>
#include <avcall.h>

#include "builtins/builtins.h"
#include "array.h"
#include "method.h"
#include "object.h"
#include "gc.h"
#include "dynarr.h"

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

static Classes *s_classes;
static Method **s_dynamic_methods = NULL;
static Frame *s_current_frame = NULL;

#define DISPATCH()          \
    ++frame->pc;            \
    op = data[frame->pc];   \
    goto *opcodes[op]       \

static inline float ieee754_bits_to_float(uint32_t bits)
{
    // TODO: Handle for platforms without IEEE 754 floats
    union {
        uint32_t i;
        float f;
    } u;
    u.i = bits;
    return u.f;
}

Frame *frame_new(int max_stack, int max_local)
{
    Frame *frame = malloc(sizeof(Frame));
    frame->max_stack = max_stack;
    frame->max_locals = max_local;

    frame->stack = stack_new(max_stack);
    frame->locals = calloc(max_local, sizeof(Variant));

    gc_track_frame(frame);

    return frame;
}

void frame_free(Frame *frame)
{
    gc_untrack_frame(frame);

    // Go back to the old frame
    s_current_frame = frame->caller;

    for (int i = 0; i < frame->max_locals; i++) {
        variant_release(&frame->locals[i]);
    }

    stack_free(frame->stack);
    free(frame->locals);
    free(frame);
}

Frame *get_current_frame()
{
    return s_current_frame;
}

Method *add_dynamic_method_native(const char* name, const char* descriptor, jit_function_t function)
{
    Method *method = malloc(sizeof(Method));
    memset(method, 0, sizeof(Method));
    method->name = name;
    method->descriptors = descriptors_new(descriptor);
    method->class = NULL;
    method->native_method = jit_function_to_closure(function);
    // TODO: can this assumption be false?
    method->info.access_flags = ACC_STATIC;

    if (!s_dynamic_methods)
    {
        s_dynamic_methods = arr_init(Method*);
    }

    arr_push(s_dynamic_methods, method);
    return method;
}

void destroy_dynamic_methods()
{
    if (!s_dynamic_methods)
        return;

    for (int i = 0; i < arr_length(s_dynamic_methods); ++i)
    {
        descriptors_free(s_dynamic_methods[i]->descriptors);
        free(s_dynamic_methods[i]);
    }
    arr_free(s_dynamic_methods);
}

Method *get_method(ConstantPool *pool, Class *class, uint16_t index)
{
    Method *class_method;

    uint16_t name_and_type_index = pool->pool[index].method_ref.name_and_type_index;
    uint16_t method_index = pool->pool[name_and_type_index].name_and_type_info.name_index;
    uint16_t descriptor_index = pool->pool[name_and_type_index].name_and_type_info.descriptor_index;
    const char *method_name = constant_pool_resolve_string(pool, method_index);
    const char *descriptor = constant_pool_resolve_string(pool, descriptor_index);

    //printf("Resolving method %s with descriptor %s in class %s\n", method_name, descriptor, class->name);

    class_method = class_get_method(class, method_name, descriptor);

    if (!class_method) {
        fprintf(stderr, "Failed to resolve method %s with descriptor %s in class %s\n", method_name, descriptor, class->name);
        exit(1);
    }

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

static Variant resolve_bootstrap_argument(ConstantPool *pool, uint16_t index)
{
    switch (constant_pool_get_tag(pool, index))
    {
        case CONSTANT_CLASS:
        {
            Class *resolved_class = classes_get_class_from_index(pool, index);
            Object *class_object = object_new(classes_get_class("java/lang/Class"));
            object_get_field(class_object, "class")->value.data.class = resolved_class;
            return variant_make_object(class_object);
        }

        case CONSTANT_INT:
            return variant_make_int(constant_pool_resolve_int(pool, index));

        case CONSTANT_STRING:
        {
            Object *str_obj = object_new(classes_get_class("java/lang/String"));
            object_set_field(str_obj, "value", variant_make_ref(constant_pool_resolve_string(pool, index)));
            return variant_make_object(str_obj);
        }

        case CONSTANT_METHODTYPE:
        {
            Object *method_type = object_new(classes_get_class("java/lang/invoke/MethodType"));
            object_set_field(method_type, "descriptor", variant_make_ref(constant_pool_resolve_field_descriptor(pool, index)));
            return variant_make_object(method_type);
        }

        case CONSTANT_METHODHANDLE:
        {
            Object *method_handle = object_new(classes_get_class("java/lang/invoke/MethodHandle"));
            return variant_make_object(method_handle);
        }

        default:
            fprintf(stderr, "Unsupported bootstrap constant tag 0x%x at index %u\n",
                    constant_pool_get_tag(pool, index), index);
            return (Variant){0};
    }
}

static void throw_exception(Object *exception)
{
    Frame *frame = s_current_frame;
    Frame *next_frame;
    while (frame) {
        if (frame->method->exception_table_length > 0) {
            for (int i = 0; i < frame->method->exception_table_length; i++) {
                ExceptionHandler *handler = &frame->method->exception_table[i];
                if (frame->pc >= handler->start_pc && frame->pc < handler->end_pc) {
                    if (handler->catch_type == 0) {
                        // Catch all exceptions
                        frame->pc = handler->handler_pc - 1;
                        printf("Caught a finally block exception at pc %d in method %s in class %s\n", frame->pc + 1, frame->method->name, frame->method->class->name);
                        stack_clear(frame->stack);
                        stack_push(frame->stack, variant_make_object(exception));
                        return;
                    } else {
                        Class *catch_class = classes_get_class_from_index(frame->method->class->pool, handler->catch_type);
                        if (class_is_subclass(exception->class, catch_class)) {
                            frame->pc = handler->handler_pc - 1;
                            printf("Caught an exception of class %s at pc %d in method %s in class %s\n", exception->class->name, frame->pc + 1, frame->method->name, frame->method->class->name);
                            stack_clear(frame->stack);
                            stack_push(frame->stack, variant_make_object(exception));
                            return;
                        }
                    }
                }
            }
        }

        next_frame = frame->caller;
        if (next_frame)
            printf("Did not find an exception handler, now looking in method %s in class %s\n", next_frame->method->name, next_frame->method->class->name);
        frame_free(frame);
        frame = next_frame;
    }

    fprintf(stderr, "Uncaught exception of type %s, message: %s\n", exception->class->name, object_get_field(exception, "message")->value.data.ref);
    exit(1);
}

Variant call_native_method(Method *method, ...)
{
    /* Convert all of the arguments from the method's descriptors to native arguments */
    av_alist list;
    Variant return_value = (Variant){0};
    Variant args[method->descriptors->arguments_count + !(method->info.access_flags & 0x0008)];
    int j = 0;

    va_list args_list;
    va_start(args_list, method);

    for (int i = 0; i < method->descriptors->arguments_count + !(method->info.access_flags & 0x0008); i++) {
        args[i] = va_arg(args_list, Variant);
    }

    va_end(args_list);

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
            break;
        case DESCRIPTOR_INT:
            av_start_int(list, (int(*)())method->native_method, &return_value.data.int_val);
            return_value.type = VARIANT_TYPE_INT;
            break;
        case DESCRIPTOR_CHAR:
            av_start_short(list, (short(*)())method->native_method, &return_value.data.int_val);
            return_value.type = VARIANT_TYPE_INT;
            break;
        case DESCRIPTOR_LONG:
            av_start_long(list, (long(*)())method->native_method, &return_value.data.long_val);
            return_value.type = VARIANT_TYPE_LONG;
            break;
        case DESCRIPTOR_BOOL:
            av_start_uchar(list, (unsigned char(*)())method->native_method, &return_value.data.int_val);
            return_value.type = VARIANT_TYPE_INT;
            break;
        case DESCRIPTOR_FLOAT:
            av_start_float(list, (float(*)())method->native_method, &return_value.data.float_val);
            return_value.type = VARIANT_TYPE_FLOAT;
            break;
        case DESCRIPTOR_DOUBLE:
            av_start_double(list, (double(*)())method->native_method, &return_value.data.double_val);
            return_value.type = VARIANT_TYPE_DOUBLE;
            break;
        case DESCRIPTOR_OBJECT:
            av_start_ptr(list, (Object*(*)())method->native_method, Object *, &return_value.data.object);
            return_value.type = VARIANT_TYPE_OBJECT;
            break;
    }

    if (method->info.access_flags & ACC_NATIVE)
    {
        if (!method->class->static_initialized)
        {
            class_initialize_static(method->class);
        }
        av_ptr(list, JNIEnv *, &get_jni()->env);
    }

    /* If the function isn't static, push the "this" pointer */
    if (!(method->info.access_flags & 0x0008)) {
        av_ptr(list, Object *, args[j++].data.object);
    }

    Descriptor descriptor;
    FOREACH_DESCRIPTOR(method->descriptors, descriptor) {
        // If the current argument is an array, just pass the Array pointer
        if (descriptor.array_dimesions_count > 0) {
            av_ptr(list, Array *, args[j].data.ref);
            j++;
            continue;
        }

        switch (descriptor.type) {
            case DESCRIPTOR_INT:
                av_int(list, args[j].data.int_val);
                break;
            case DESCRIPTOR_OBJECT:
                av_ptr(list, Object *, args[j].data.object);
                break;
            case DESCRIPTOR_CHAR:
                av_short(list, args[j].data.int_val);
                break;
            case DESCRIPTOR_LONG:
                av_long(list, args[j].data.long_val);
                break;
            case DESCRIPTOR_BOOL:
                av_uchar(list, args[j].data.int_val);
                break;
            case DESCRIPTOR_FLOAT:
                av_float(list, args[j].data.float_val);
                break;
            case DESCRIPTOR_DOUBLE:
                av_double(list, args[j].data.double_val);
                break;
            case DESCRIPTOR_VOID:
                break;
        }

        j++;
    }

call:
    av_call(list);

    return return_value;
}

/* Return the result of executing the method */
Variant method_execute(Method *method, ...)
{
    /* Access code of method */
    Frame *frame = frame_new(method->max_stack, method->max_local);
    uint8_t *data = method->data;
    ConstantPool *pool = method->class->pool;
    uint8_t op;
    va_list args;

    va_start(args, method);

    // Improve later
    // Note: This is currently hardcoded to pull arguments like it would from the stack, i.e. backwards
    // Maybe improve this later?
    if (method->info.access_flags & ACC_STATIC) {
        for (int i = 0; i < method->descriptors->arguments_count; ++i) {
            frame->locals[i] = va_arg(args, Variant);
        }
    } else {
        for (int i = 0; i < method->descriptors->arguments_count + 1; ++i) {
            frame->locals[i] = va_arg(args, Variant);
        }
    }
    va_end(args);

    frame->caller = s_current_frame;
    frame->method = method;
    s_current_frame = frame;

    printf("Beginning execution of method %s in class %s in frame %p\n", method->name, method->class->name, (void*)frame);
    const static void *opcodes[] = {
        [1] = &&aconst_null,
        [2] = &&iconst_m1,
        [3] = &&iconst_0,
        [4] = &&iconst_1,
        [5] = &&iconst_2,
        [6] = &&iconst_3,
        [7] = &&iconst_4,
        [8] = &&iconst_5,
        [9] = &&lconst_0,
        [10] = &&lconst_1,
        [11] = &&fconst_0,
        [12] = &&fconst_1,
        [13] = &&fconst_2,
        [14] = &&dconst_0,
        [15] = &&dconst_1,
        [16] = &&bipush,
        [17] = &&sipush,
        [18] = &&ldc,
        [19] = &&ldc_w,
        [20] = &&ldc2_w,
        [21] = &&iload,
        [22] = &&lload,
        [23] = &&fload,
        [24] = &&dload,
        [25] = &&aload,
        [26 ... 29] = &&iload_x,
        [34 ... 37] = &&fload_x,
        [42 ... 45] = &&aload_x,
        [46] = &&iaload,
        [50] = &&aaload,
        [54] = &&istore,
        [58] = &&astore,
        [59 ... 62] = &&istore_x,
        [75 ... 78] = &&astore_x,
        [79] = &&iastore,
        [83] = &&aastore,
        [85] = &&castore,
        [87] = &&pop,
        [89] = &&dup,
        [90] = &&dup_x1,
        [96] = &&iadd,
        [97] = &&ladd,
        [100] = &&isub,
        [104] = &&imul,
        [106] = &&fmul,
        [108] = &&idiv,
        [120] = &&ishl,
        [121] = &&lshl,
        [122] = &&ishr,
        [123] = &&lshr,
        [124] = &&iushr,
        [126] = &&iand,
        [127] = &&land,
        [128] = &&ior,
        [130] = &&ixor,
        [132] = &&iinc,
        [133] = &&i2l,
        [134] = &&i2f,
        [139] = &&f2i,
        [146] = &&i2c,
        [149 ... 150] = &&fcmp_x,
        [153] = &&ifeq,
        [154] = &&ifne,
        [155] = &&iflt,
        [156] = &&ifge,
        [157] = &&ifgt,
        [158] = &&ifle,
        [159 ... 164] = &&if_cmpx,
        [165] = &&if_acmpeq,
        [166] = &&if_acmpne,
        [167] = &&j_goto,
        [172] = &&ireturn,
        [173] = &&lreturn,
        [174] = &&freturn,
        [175] = &&dreturn,
        [176] = &&areturn,
        [177] = &&j_return,
        [178] = &&getstatic,
        [179] = &&putstatic,
        [180] = &&getfield,
        [181] = &&putfield,
        [182] = &&invokevirtual,
        [183] = &&invokespecial,
        [184] = &&invokestatic,
        [185] = &&invokeinterface,
        [186] = &&invokedynamic,
        [187] = &&new,
        [188] = &&newarray,
        [189] = &&anewarray,
        [190] = &&arraylength,
        [191] = &&athrow,
        [192] = &&checkcast,
        [193] = &&instanceof,
        [194] = &&monitorenter,
        [195] = &&monitorexit,
        [197] = &&multianewarray,
        [198] = &&ifnull,
        [199] = &&ifnonnull,
    };

    frame->pc = 0;
    frame->code = data;

    op = data[frame->pc];
    goto *opcodes[op];

    aconst_null:
        stack_push(frame->stack, variant_make_ref(NULL));
        DISPATCH();

    iconst_m1:
        stack_push_int(frame->stack, -1);
        DISPATCH();

    iconst_0:
        stack_push_int(frame->stack, 0);
        DISPATCH();

    iconst_1:
        stack_push_int(frame->stack, 1);
        DISPATCH();

    iconst_2:
        stack_push_int(frame->stack, 2);
        DISPATCH();

    iconst_3:
        stack_push_int(frame->stack, 3);
        DISPATCH();

    iconst_4:
        stack_push_int(frame->stack, 4);
        DISPATCH();

    iconst_5:
        stack_push_int(frame->stack, 5);
        DISPATCH();

    lconst_0:
        stack_push(frame->stack, variant_make_long(0));
        DISPATCH();

    lconst_1:
        stack_push(frame->stack, variant_make_long(1));
        DISPATCH();

    fconst_0:
        stack_push(frame->stack, variant_make_float(0.0f));
        DISPATCH();

    fconst_1:
        stack_push(frame->stack, variant_make_float(1.0f));
        DISPATCH();

    fconst_2:
        stack_push(frame->stack, variant_make_float(2.0f));
        DISPATCH();

    dconst_0:
        stack_push(frame->stack, variant_make_double(0.0));
        DISPATCH();

    dconst_1:
        stack_push(frame->stack, variant_make_double(1.0));
        DISPATCH();

    bipush: {
        uint8_t byte = data[++frame->pc];
        stack_push_int(frame->stack, byte);
        DISPATCH();
    }

    sipush:
        uint16_t shrt = (data[++frame->pc] << 8) | data[++frame->pc];
        stack_push_int(frame->stack, shrt);
        DISPATCH();

    // Maybe combine the LDC calls
    ldc: {
        uint8_t index = data[++frame->pc];
        uint8_t tag = constant_pool_get_tag(pool, index);
        Variant variant = (Variant){0};

        switch (tag) {
            case CONSTANT_CLASS: {
                Class *class = classes_get_class_from_index(pool, index);
                Object *class_object = object_new(classes_get_class("java/lang/Class"));
                object_get_field(class_object, "class")->value.data.class = class;
                variant.type = VARIANT_TYPE_OBJECT;
                variant.data.object = class_object;
                break;
            }

            case CONSTANT_INT: {
                variant.data.int_val = constant_pool_resolve_int(pool, index);
                variant.type = VARIANT_TYPE_INT;
                break;
            }

            case CONSTANT_FLOAT: {
                uint32_t float_bits = constant_pool_resolve_float(pool, index);
                variant.data.float_val = ieee754_bits_to_float(float_bits);
                variant.type = VARIANT_TYPE_FLOAT;
                break;
            }

            case CONSTANT_STRING: {
                Object *str_obj = object_new(classes_get_class("java/lang/String"));
                object_set_field(str_obj, "value", variant_make_ref(constant_pool_resolve_string(pool, index)));
                variant.data.object = str_obj;
                variant.type = VARIANT_TYPE_OBJECT;
                break;
            }

            case CONSTANT_UTF8: {
                variant.data.ref = constant_pool_resolve_string(pool, index);
                variant.type = VARIANT_TYPE_REF;
                break;
            }

            default:
                fprintf(stderr, "Unsupported constant tag 0x%x at index %u\n", tag, index);
                exit(1);
        }

        stack_push(frame->stack, variant);
        DISPATCH();
    }

    ldc_w: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        uint8_t tag = constant_pool_get_tag(pool, index);
        Variant variant = {0};

        switch (tag) {
            case CONSTANT_CLASS: {
                Class *class = classes_get_class_from_index(pool, index);
                Object *class_object = object_new(classes_get_class("java/lang/Class"));
                object_get_field(class_object, "class")->value.data.class = class;
                variant.type = VARIANT_TYPE_OBJECT;
                variant.data.object = class_object;
                break;
            }

            case CONSTANT_INT: {
                variant.data.int_val = constant_pool_resolve_int(pool, index);
                variant.type = VARIANT_TYPE_INT;
                break;
            }

            case CONSTANT_FLOAT: {
                uint32_t float_bits = constant_pool_resolve_float(pool, index);
                variant.data.float_val = ieee754_bits_to_float(float_bits);
                variant.type = VARIANT_TYPE_FLOAT;
                break;
            }

            case CONSTANT_STRING: {
                Object *str_obj = object_new(classes_get_class("java/lang/String"));
                object_set_field(str_obj, "value", variant_make_ref(constant_pool_resolve_string(pool, index)));
                variant.data.object = str_obj;
                variant.type = VARIANT_TYPE_OBJECT;
                break;
            }

            case CONSTANT_UTF8: {
                variant.data.ref = constant_pool_resolve_string(pool, index);
                variant.type = VARIANT_TYPE_REF;
                break;
            }

            default:
                fprintf(stderr, "Unsupported constant tag 0x%x at index %u\n", tag, index);
                exit(1);
        }

        stack_push(frame->stack, variant);
        DISPATCH();
    }

    ldc2_w: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        uint8_t tag = constant_pool_get_tag(pool, index);
        Variant variant = {0};

        switch (tag) {
            case CONSTANT_LONG: {
                uint64_t long_val;
                memcpy(&long_val, &pool->pool[index].long_val.low_bytes, sizeof(uint32_t));
                memcpy((uint8_t*)&long_val + 4, &pool->pool[index].long_val.high_bytes, sizeof(uint32_t));
                variant = variant_make_long(long_val);
                break;
            }
    
            case CONSTANT_DOUBLE: {
                uint64_t double_bits;
                memcpy(&double_bits, &pool->pool[index].long_val.low_bytes, sizeof(uint32_t));
                memcpy((uint8_t*)&double_bits + 4, &pool->pool[index].long_val.high_bytes, sizeof(uint32_t));
                union {
                    uint64_t i;
                    double d;
                } u = { .i = double_bits };
                variant = variant_make_double(u.d);
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

    lload: {
        uint8_t index = data[++frame->pc];
        stack_push(frame->stack, frame->locals[index]);
        DISPATCH();
    }

    fload: {
        uint8_t index = data[++frame->pc];
        stack_push(frame->stack, frame->locals[index]);
        DISPATCH();
    }

    dload: {
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

    fload_x: {
        uint8_t local_index = op - 34;
        Variant item = frame->locals[local_index];
        stack_push(frame->stack, item);
        DISPATCH();
    }

    aload_x: {
        uint8_t local_index = op - 42;
        stack_push(frame->stack, frame->locals[local_index]);
        DISPATCH();
    }

    iaload: {
        int index = stack_pop(frame->stack).data.int_val;
        int* array = stack_pop(frame->stack).data.ref;

        if (index < 0 || index >= arr_capacity(array)) {
            Object *exception = object_new(classes_get_class("java/lang/ArrayIndexOutOfBoundsException"));
            char *str;
            asprintf(&str, "Index %d out of bounds for length %zu", index, arr_capacity(array));
            Object *str_obj = object_new(classes_get_class("java/lang/String"));
            object_set_field(str_obj, "value", variant_make_owned_ref(str));
            object_set_field(exception, "message", variant_make_object(str_obj));
            throw_exception(exception);
        } else {
            stack_push_int(frame->stack, array[index]);
        }

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
        Variant value = stack_pop(frame->stack);
        frame->locals[local_index] = value;
        DISPATCH();
    }

    iastore: {
        int value = stack_pop(frame->stack).data.int_val;
        int index = stack_pop(frame->stack).data.int_val;
        int* array = stack_pop(frame->stack).data.ref;

        if (index < 0 || index >= arr_capacity(array)) {
            Object *exception = object_new(classes_get_class("java/lang/ArrayIndexOutOfBoundsException"));
            char *str;
            asprintf(&str, "Index %d out of bounds for length %zu", index, arr_capacity(array));
            Object *str_obj = object_new(classes_get_class("java/lang/String"));
            object_set_field(str_obj, "value", variant_make_owned_ref(str));
            object_set_field(exception, "message", variant_make_object(str_obj));
            throw_exception(exception);
        } else {
            arr_push(array, value);
        }

        DISPATCH();
    }
    aastore: {
        Variant value = stack_pop(frame->stack);
        int index = stack_pop(frame->stack).data.int_val;
        Array *array = stack_pop(frame->stack).data.ref;

        array_set_value(array, index, value);
        DISPATCH();
    }

    castore: {
        uint16_t value = stack_pop(frame->stack).data.int_val;
        int index = stack_pop(frame->stack).data.int_val;
        uint16_t* array = stack_pop(frame->stack).data.ref;

        if (index < 0 || index >= arr_capacity(array)) {
            Object *exception = object_new(classes_get_class("java/lang/ArrayIndexOutOfBoundsException"));
            char *str;
            asprintf(&str, "Index %d out of bounds for length %zu", index, arr_capacity(array));
            Object *str_obj = object_new(classes_get_class("java/lang/String"));
            object_set_field(str_obj, "value", variant_make_owned_ref(str));
            object_set_field(exception, "message", variant_make_object(str_obj));
            throw_exception(exception);
        } else {
            arr_push(array, value);
        }

        DISPATCH();
    }

    pop:
        stack_pop(frame->stack);
        DISPATCH();

    dup:
        stack_dup(frame->stack);
        DISPATCH();

    dup_x1: {
        // Genuinely fuck you oracle what is this fucking mess
        Variant val1 = stack_pop(frame->stack);
        Variant val2 = stack_pop(frame->stack);
        stack_push(frame->stack, val1);
        stack_push(frame->stack, val2);
        stack_push(frame->stack, val1);
        DISPATCH();
    }

    iadd:
        int a1 = stack_pop(frame->stack).data.int_val;
        int a2 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, a1 + a2);
        DISPATCH();

    ladd:
        long la2 = stack_pop(frame->stack).data.long_val;
        long la1 = stack_pop(frame->stack).data.long_val;
        stack_push(frame->stack, variant_make_long(la1 + la2));
        DISPATCH();

    isub:
        int sub2 = stack_pop(frame->stack).data.int_val;
        int sub1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, sub1 - sub2);
        DISPATCH();

    imul:
        int m2 = stack_pop(frame->stack).data.int_val;
        int m1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, m1 * m2);
        DISPATCH();

    fmul:
        float fm2 = stack_pop(frame->stack).data.float_val;
        float fm1 = stack_pop(frame->stack).data.float_val;
        stack_push(frame->stack, variant_make_float(fm1 * fm2));
        DISPATCH();

    idiv:
        int d2 = stack_pop(frame->stack).data.int_val;
        int d1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, d1 / d2);
        DISPATCH();

    ishl:
        int shl2 = stack_pop(frame->stack).data.int_val;
        int shl1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, shl1 << shl2);
        DISPATCH();

    lshl:
        long lshl2 = stack_pop(frame->stack).data.long_val;
        long lshl1 = stack_pop(frame->stack).data.long_val;
        stack_push(frame->stack, variant_make_long(lshl1 << lshl2));
        DISPATCH();

    ishr:
        int sh2 = stack_pop(frame->stack).data.int_val;
        int sh1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, sh1 >> sh2);
        DISPATCH();

    lshr:
        long lsh2 = stack_pop(frame->stack).data.long_val;
        long lsh1 = stack_pop(frame->stack).data.long_val;
        stack_push(frame->stack, variant_make_long(lsh1 >> lsh2));
        DISPATCH();

    iushr:
        int s2 = stack_pop(frame->stack).data.int_val;
        int s1 = stack_pop(frame->stack).data.int_val;
        // TODO: Check if we're shifting properly
        stack_push_int(frame->stack, (unsigned int)s1 >> s2);
        DISPATCH();

    iand:
        int and2 = stack_pop(frame->stack).data.int_val;
        int and1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, and1 & and2);
        DISPATCH();

    land:
        long land2 = stack_pop(frame->stack).data.long_val;
        long land1 = stack_pop(frame->stack).data.long_val;
        stack_push(frame->stack, variant_make_long(land1 & land2));
        DISPATCH();

    ior:
        int or2 = stack_pop(frame->stack).data.int_val;
        int or1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, or1 | or2);
        DISPATCH();

    ixor:
        int x2 = stack_pop(frame->stack).data.int_val;
        int x1 = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, x1 ^ x2);
        DISPATCH();

    iinc:
        uint8_t index = data[++frame->pc];
        int8_t const_val = data[++frame->pc];
        frame->locals[index].data.int_val += const_val;
        DISPATCH();

    i2l: {
        int value = stack_pop(frame->stack).data.int_val;
        stack_push(frame->stack, variant_make_long((int64_t)value));
        DISPATCH();
    }

    i2f: {
        int value = stack_pop(frame->stack).data.int_val;
        stack_push(frame->stack, variant_make_float((float)value));
        DISPATCH();
    }

    f2i: {
        float value = stack_pop(frame->stack).data.float_val;
        stack_push_int(frame->stack, (int)value);
        DISPATCH();
    }

    i2c: {
        int value = stack_pop(frame->stack).data.int_val;
        stack_push_int(frame->stack, (uint16_t)(value & 0xFFFF));
        DISPATCH();
    }

    fcmp_x: {
        // TODO: Handle NaN
        float f2 = stack_pop(frame->stack).data.float_val;
        float f1 = stack_pop(frame->stack).data.float_val;
        if (f1 < f2) {
            stack_push_int(frame->stack, -1);
        } else if (f1 == f2) {
            stack_push_int(frame->stack, 0);
        } else {
            stack_push_int(frame->stack, 1);
        }
        DISPATCH();
    }

    ifeq: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value == 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    ifne: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value != 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    iflt: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value < 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    ifge: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value >= 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    ifgt: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value > 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    ifle: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        int value = stack_pop(frame->stack).data.int_val;
        if (value <= 0) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

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

    if_acmpeq: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value2 = stack_pop(frame->stack);
        Variant value1 = stack_pop(frame->stack);

        // TODO: Compare types
        if (value1.data.ref == value2.data.ref) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    if_acmpne: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value2 = stack_pop(frame->stack);
        Variant value1 = stack_pop(frame->stack);

        if (value1.data.ref != value2.data.ref) {
            frame->pc += branch_offset - 3;
        }

        DISPATCH();
    }

    j_goto: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        /* Two bytes for the branch offset, and one byte for DISPATCH */
        frame->pc += branch_offset - 3;
        DISPATCH();
    }

    ireturn: {
        Variant ret = stack_pop(frame->stack);
        // Sanity check
        if (ret.type != VARIANT_TYPE_INT) {
            fprintf(stderr, "Expected int return type but got variant type %d\n", ret.type);
            exit(1);
        }
        frame_free(frame);

        return ret;
    }

    lreturn: {
        Variant ret = stack_pop(frame->stack);
        // Sanity check
        if (ret.type != VARIANT_TYPE_LONG) {
            fprintf(stderr, "Expected long return type but got variant type %d\n", ret.type);
            exit(1);
        }
        frame_free(frame);

        return ret;
    }

    freturn: {
        Variant ret = stack_pop(frame->stack);
        // Sanity check
        if (ret.type != VARIANT_TYPE_FLOAT) {
            fprintf(stderr, "Expected float return type but got variant type %d\n", ret.type);
            exit(1);
        }
        frame_free(frame);

        return ret;
    }

    dreturn: {
        Variant ret = stack_pop(frame->stack);
        // Sanity check
        if (ret.type != VARIANT_TYPE_DOUBLE) {
            fprintf(stderr, "Expected double return type but got variant type %d\n", ret.type);
            exit(1);
        }
        frame_free(frame);

        return ret;
    }

    areturn: {
        Variant ret = stack_pop(frame->stack);
        frame_free(frame);

        return ret;
    }

    j_return: {
        frame_free(frame);
        return (Variant){0};
    }

    getstatic: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(pool, index);
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
        Class *class = classes_get_class_from_index(pool, index);
        /* Check if the class has been initialized yet. */
        if (class->fields->static_fields_count && !class->static_initialized) {
            class_initialize_static(class);
        }

        Field *field = class_get_static_field(class, constant_pool_resolve_field_name(class->pool, index));

        /* TODO: Implement value conversion */
        field->value = stack_pop(frame->stack);
        if (field->value.type == VARIANT_TYPE_OBJECT && field->value.data.object) {
            field->value.data.object->parent_field = field;
        }
        DISPATCH();
    }

    getfield: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Object *object = stack_pop(frame->stack).data.object;
        const char *field_name = constant_pool_resolve_field_name(pool, index);
        Field *field = object_get_field(object, field_name);

        stack_push(frame->stack, field->value);
        DISPATCH();
    }

    putfield: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value = stack_pop(frame->stack);
        Object *object = stack_pop(frame->stack).data.object;

        const char *field_name = constant_pool_resolve_field_name(pool, index);
        object_set_field(object, field_name, value);

        if (value.type == VARIANT_TYPE_OBJECT) {
            value.data.object->parent_field = object_get_field(object, field_name);
        }
        DISPATCH();
    }

    invokevirtual:
    invokespecial: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(pool, index);
        // Fake method, we'll find the real one from the object we were passed
        Method *class_method = get_method(pool, class, index);
        Variant ret;
        av_alist list;
        Variant args[class_method->descriptors->arguments_count + 1];

        for (int i = class_method->descriptors->arguments_count; i >= 0; --i) {
            args[i] = stack_pop(frame->stack);
        }

        if (class_method->info.access_flags & ACC_ABSTRACT) {
            class = args[0].data.object->class;
            const char* method_name = class_method->name;
            const char* method_descriptor = class_method->descriptors->descriptor;
            while (class) {
                printf("Class name is %s\n", class->name);
                printf("Class parent name is %s\n", class->parent ? class->parent->name : "NULL");
                printf("Looking for method %s with descriptor %s in class %s\n", method_name, method_descriptor, class->name);
                class_method = class_get_method(class, method_name, method_descriptor);
                if (class_method) {
                    break;
                }
                class = class->parent;
            }

            if (class_method == NULL) {
                fprintf(stderr, "Failed to find implementation for abstract method %s with descriptor %s\n", class_method->name, class_method->descriptors->descriptor);
                exit(1);
            }
        }

        // This feels really ugly, I guess we can clean it up later
        if (class->built_in)
            av_start_struct(list, call_native_method, Variant, 3, &ret);
        else
            av_start_struct(list, method_execute, Variant, 3, &ret);

        av_ptr(list, Method *, class_method);

        // Now make the arguments in a "sane" order
        for (int i = 0; i < class_method->descriptors->arguments_count + 1; ++i) {
            av_struct(list, Variant, args[i]);
        }

        av_call(list);

        //printf("finished execution of submethod %s\n", class_method->name);

        if (class_method->descriptors->return_descriptor.type != DESCRIPTOR_VOID)
            stack_push(frame->stack, ret);

        DISPATCH();
    }

    invokestatic: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(pool, index);
        Method *class_method = get_method(pool, class, index);
        av_alist list;
        Variant ret;

        if (!class->static_initialized)
            class_initialize_static(class);

        if (class->built_in || class_method->info.access_flags & ACC_NATIVE)
            av_start_struct(list, call_native_method, Variant, 3, &ret);
        else
            av_start_struct(list, method_execute, Variant, 3, &ret);

        av_ptr(list, Method *, class_method);

        Variant args[class_method->descriptors->arguments_count];
        for (int i = class_method->descriptors->arguments_count - 1; i >= 0; --i) {
            args[i] = stack_pop(frame->stack);
        }

        for (int i = 0; i < class_method->descriptors->arguments_count; i++) {
            av_struct(list, Variant, args[i]);
        }

        av_call(list);

        if (class_method->descriptors->return_descriptor.type != DESCRIPTOR_VOID)
            stack_push(frame->stack, ret);

        DISPATCH();
    }

    invokeinterface: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        frame->pc += 2;
        Class *interface = classes_get_class_from_index(pool, index);
        Method *interface_method = get_method(pool, interface, index);
        Class *real_class = NULL;

        int arguments_count = interface_method->descriptors->arguments_count;
        Variant *temp_locals = NULL;

        temp_locals = calloc((arguments_count + !(interface_method->info.access_flags & ACC_STATIC)), sizeof(Variant));
        // Arguments, and one for "this" if it's not a static method
        for (int i = arguments_count - !!(interface_method->info.access_flags & ACC_STATIC); i >= 0; --i) {
            temp_locals[i] = stack_pop(frame->stack);
        }

        real_class = temp_locals[0].data.object->class;

        Method *methodd = NULL;
        while (real_class) {
            methodd = class_get_method(real_class, interface_method->name, interface_method->descriptors->descriptor);
            if (methodd) {
                break;
            }
            real_class = real_class->parent;
        }

        if (!methodd) {
            fprintf(stderr, "Failed to find implementation for interface method %s with descriptor %s\n", interface_method->name, interface_method->descriptors->descriptor);
            exit(1);
        }

        av_alist list;
        Variant ret;

        if (methodd->class->built_in || methodd->info.access_flags & ACC_NATIVE)
            av_start_struct(list, call_native_method, Variant, 3, &ret);
        else
            av_start_struct(list, method_execute, Variant, 3, &ret);

        av_ptr(list, Method *, methodd);

        for (int i = 0; i < arguments_count + !(interface_method->info.access_flags & 0x0008); i++) {
            Variant arg = temp_locals[i];
            av_struct(list, Variant, arg);
        }

        free(temp_locals);

        av_call(list);

        if (methodd->descriptors->return_descriptor.type != DESCRIPTOR_VOID)
            stack_push(frame->stack, ret);

        DISPATCH();
    }

    invokedynamic: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        frame->pc += 2; /* Skip the two bytes of zeroes */

        ConstantPoolInfo *info = &pool->pool[index];
        Method *dynamic_method = NULL;

        // Check if a dynamic method has already been linked for this invokedynamic instruction.
        for (int i = 0; s_dynamic_methods && i < arr_length(s_dynamic_methods); ++i) {
            dynamic_method = s_dynamic_methods[i];
            if (i == info->dynamic_info.bootstrap_index &&
                !strcmp(dynamic_method->name, constant_pool_resolve_field_name(pool, info->dynamic_info.name_and_type_info)) &&
                !strcmp(dynamic_method->descriptors->descriptor, constant_pool_resolve_field_descriptor(pool, info->dynamic_info.name_and_type_info))) {
                // If we find a matching dynamic method, just execute it.
                printf("Found existing dynamic method %s with descriptor %s for invokedynamic instruction at index %u, executing it\n",
                       dynamic_method->name, dynamic_method->descriptors->descriptor, index);
                goto execute_dynamic_method;
            }
        }

        dynamic_method = NULL;

        AttributeInfo *bootstrap_methods = attributes_get_attribute(method->class->attributes, "BootstrapMethods");
        uint16_t bootstrap_index = info->dynamic_info.bootstrap_index;
        uint16_t bootstrap_arguments_count = bootstrap_methods->BootstrapMethodsAttribute.bootstrap_methods[bootstrap_index].num_bootstrap_arguments;
        info = &pool->pool[bootstrap_methods->BootstrapMethodsAttribute.bootstrap_methods[bootstrap_index].bootstrap_method_ref];

        uint16_t ref_index = info->method_handle_info.ref_index;
        uint16_t ref_kind = info->method_handle_info.ref_kind;

        info = &pool->pool[index];

        // Try to resolve this dynamic method now.
        Class *class = classes_get_class_from_index(pool, ref_index);
        Method *class_method = get_method(pool, class, ref_index);
        av_alist bootstrap_list;
        Variant bootstrap_ret;
        Object *method_string = object_new(classes_get_class("java/lang/String"));
        Object *lookup = object_new(classes_get_class("java/lang/invoke/MethodHandles$Lookup"));

        // Create a MethodType from the NameAndType
        Object* method_type = object_new(classes_get_class("java/lang/invoke/MethodType"));
        const char* field_descriptor = constant_pool_resolve_field_descriptor(pool, info->dynamic_info.name_and_type_info);
        object_set_field(method_type, "descriptor", variant_make_ref(field_descriptor));
        object_set_field(method_string, "value", variant_make_ref(constant_pool_resolve_field_name(pool, info->dynamic_info.name_and_type_info)));

        if (class->built_in)
            av_start_struct(bootstrap_list, call_native_method, Variant, 3, &bootstrap_ret);
        else
            av_start_struct(bootstrap_list, method_execute, Variant, 3, &bootstrap_ret);

        av_ptr(bootstrap_list, Method *, class_method);

        // I'm sorry for this gore, but I don't feel like fixing this right now. Arguments are pushed backwards
        {
            // First argument is a reference to MethodHandles.Lookup object, obtained as by calling MethodHandles.lookup().
            Variant arg = variant_make_object(lookup);
            av_struct(bootstrap_list, Variant, arg);

            // Second argument is a reference to a string containing the name of the method to be linked, obtained from the constant pool.
            arg = variant_make_object(method_string);
            av_struct(bootstrap_list, Variant, arg);

            // Third argument is a reference to a MethodType object representing the type of this method, obtained while resolving the bootstrap method.
            arg = variant_make_object(method_type);
            av_struct(bootstrap_list, Variant, arg);
        }

        // And finally, now we push the bootstrap arguments according to the bootstrap method descriptor.
        uint16_t remaining_bootstrap_arguments = bootstrap_arguments_count;
        uint16_t bootstrap_argument_cursor = 0;

        for (int j = 3; j < class_method->descriptors->arguments_count; j++) {
            Descriptor *descriptor = &class_method->descriptors->arguments[j];

            // Only the last argument can be a vaargs array.
            if (descriptor->array_dimesions_count > 0 && j == class_method->descriptors->arguments_count - 1) {
                Array *args_array = array_new(classes_get_class("java/lang/Object"), remaining_bootstrap_arguments);

                for (uint16_t k = 0; k < remaining_bootstrap_arguments; k++) {
                    uint16_t bootstrap_argument_index = bootstrap_methods->BootstrapMethodsAttribute.bootstrap_methods[bootstrap_index].bootstrap_arguments[bootstrap_argument_cursor++];
                    array_set_value(args_array, k, resolve_bootstrap_argument(pool, bootstrap_argument_index));
                }

                Variant arr = variant_make_ref(args_array);
                av_struct(bootstrap_list, Variant, arr);
                remaining_bootstrap_arguments = 0;
                continue;
            }

            if (bootstrap_argument_cursor >= bootstrap_arguments_count) {
                fprintf(stderr, "Bootstrap method descriptor expects more arguments than the BootstrapMethods entry provides\n");
                break;
            }

            uint16_t bootstrap_argument_index = bootstrap_methods->BootstrapMethodsAttribute.bootstrap_methods[bootstrap_index].bootstrap_arguments[bootstrap_argument_cursor++];
            Variant arg = resolve_bootstrap_argument(pool, bootstrap_argument_index);
            av_struct(bootstrap_list, Variant, arg);
            remaining_bootstrap_arguments--;
        }

        // Finally, call the method.
        av_call(bootstrap_list);

        // Now we've received a CallSite object. Link the target method handle.
        Object *callsite = bootstrap_ret.data.object;
        Object *target_method_handle = object_get_field(callsite, "target")->value.data.object;
        dynamic_method = add_dynamic_method_native(constant_pool_resolve_field_name(pool, info->dynamic_info.name_and_type_info),
                                                   constant_pool_resolve_field_descriptor(pool, info->dynamic_info.name_and_type_info),
                                                   object_get_field(target_method_handle, "method")->value.data.ref);

    execute_dynamic_method:

        // Now we can *finally* execute the dynamic method/field.
        av_alist dynamic_list;
        Variant dynamic_ret;
        av_start_struct(dynamic_list, call_native_method, Variant, 3, &dynamic_ret);

        av_ptr(dynamic_list, Method *, dynamic_method);

        Variant args[dynamic_method->descriptors->arguments_count];
        for (int j = dynamic_method->descriptors->arguments_count - 1; j >= 0; j--) {
            args[j] = stack_pop(frame->stack);
        }

        for (int j = 0; j < dynamic_method->descriptors->arguments_count; j++) {
            av_struct(dynamic_list, Variant, args[j]);
        }

        av_call(dynamic_list);

        if (dynamic_method->descriptors->return_descriptor.type != DESCRIPTOR_VOID)
            stack_push(frame->stack, dynamic_ret);

        DISPATCH();
    }

    new: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(pool, index);
        Object *object = object_new(class);
        stack_push_object(frame->stack, object);
        DISPATCH();
    }

    newarray: {
        uint8_t type = data[++frame->pc];
        uint32_t count = stack_pop(frame->stack).data.int_val;

        printf("Creating new array of type 0x%x and count %u\n", type, count);

        void *arr = NULL;
        switch (type) {
            case 4: /* boolean */
            case 8: /* byte */
                arr = arr_init_with_capacity(sizeof(char), count);
                break;
            case 6: /* float */
                arr = arr_init_with_capacity(sizeof(float), count);
                break;
            case 7: /* double */
                arr = arr_init_with_capacity(sizeof(double), count);
                break;
            case 5: /* char */
                arr = arr_init_with_capacity(sizeof(short), count);
                break;
            case 10: /* int */
                arr = arr_init_with_capacity(sizeof(int), count);
                break;
            case 11: /* long */
                arr = arr_init_with_capacity(sizeof(long), count);
                break;
            default:
                fprintf(stderr, "Unsupported array type 0x%x in newarray instruction\n", type);
                exit(1);
        }

        stack_push_ref(frame->stack, arr);

        DISPATCH();
    }

    anewarray: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Class *class = classes_get_class_from_index(pool, index);
        //printf("Creating new array of class %s at pc %d in method %s in class %s\n", class->name, frame->pc, method->name, method->class->name);

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

    athrow: {
        printf("Executing an athrow instruction at pc %d in method %s in class %s\n", frame->pc, method->name, method->class->name);
        Object *exception = stack_pop(frame->stack).data.object;
        if (!exception || !class_is_subclass(exception->class, classes_get_class("java/lang/Throwable"))) {
            fprintf(stderr, "Attempted to throw a non-Throwable object of class %s\n", exception->class->name);
            exit(1);
        }

        throw_exception(exception);

        // Now it's possible that the frame we think is ours was freed in pursuit of an exception handler, so we'll use the global current
        // frame to execute now
        frame = s_current_frame;
        method = frame->method;
        data = method->data;
        DISPATCH();
    }

    checkcast: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Object *object = frame->stack->items[frame->stack->top - 1].data.object; 

        if (object && object->initialized)
        {
            Class *cast_class = classes_get_class_from_index(pool, index);
            if (object->class != cast_class && !class_is_subclass(object->class, cast_class))
            {
                fprintf(stderr, "checkcast failed: object of class %s cannot be cast to %s\n", object->class->name, cast_class->name);
                exit(1);
            }
        }

        // Object can be cast, continue
        DISPATCH();
    }

    instanceof: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];
        Object *object = stack_pop(frame->stack).data.object;

        if (object && object->initialized)
        {
            Class *instanceof_class = classes_get_class_from_index(pool, index);
            int result = !!(object->class == instanceof_class || class_is_subclass(object->class, instanceof_class));
            stack_push_int(frame->stack, result);
        } else {
            stack_push_int(frame->stack, 0);
        }

        DISPATCH();
    }

    // TODO
    monitorenter:
        stack_pop(frame->stack);
        DISPATCH();

    monitorexit:
        stack_pop(frame->stack);
        DISPATCH();

    multianewarray: {
        uint16_t index = (data[++frame->pc] << 8) | data[++frame->pc];        
    }

    ifnull: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value = stack_pop(frame->stack);

        // HACK: Uninitialized fields do not have any type, fix them eventually
        //if (value.type == VARIANT_TYPE_REF || value.type == VARIANT_TYPE_OBJECT) {
            if (value.data.ref == NULL) {
                frame->pc += branch_offset - 3;
            }
        //}

        DISPATCH();
    }

    ifnonnull: {
        int16_t branch_offset = (data[++frame->pc] << 8) | data[++frame->pc];
        Variant value = stack_pop(frame->stack);

        // TODO: Fix our assumption of what a Java reference is
        if (value.type == VARIANT_TYPE_REF || value.type == VARIANT_TYPE_OBJECT || value.type == 0) {
            if (value.data.ref != NULL) {
                frame->pc += branch_offset - 3;
            }
        }

        DISPATCH();
    }
}

/* Class/methods code */
Class *class_parse_file(char *filename)
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
    class->parent = classes_get_class(constant_pool_resolve_string(class->pool, reader_read_uint16_be(reader)));

    uint16_t interfaces_count = reader_read_uint16_be(reader);
    if (interfaces_count)
    {
        class->interfaces = malloc(sizeof(Interface) * interfaces_count);

        for (int i = 0; i < interfaces_count; i++) {
            uint16_t interface_index = reader_read_uint16_be(reader);
            class->interfaces[i].index = interface_index;
            class->interfaces[i].interface = constant_pool_resolve_string(class->pool, interface_index);
        }
    }

    class->interfaces_count = interfaces_count;

    /* TODO: Eventually drop these somehow */
    class->fields = fields_new(class, reader, class->pool);
    class->methods = methods_new(class, reader, class->pool, &class->methods_count);
    class->attributes = attributes_new(reader, class->pool);
    class->static_initialized = false;

    return class;
}

void class_initialize_static(Class *class)
{
    Method *static_init = NULL;

    if ((static_init = class_get_method(class, "<clinit>", "()V"))) {
        class->static_initialized = true;

        if (class->built_in)
            call_native_method(static_init);
        else
            method_execute(static_init);
    }
}

void class_free(Class *class)
{
    if (!class->built_in) {
        attributes_free(class->attributes);
        constant_pool_free(class->pool);
        free(class->interfaces);
        free(class->reader);
        free(class->data);
    }

    fields_free(class->fields);
    methods_free(class->methods, class->methods_count);
    free(class);
}

/* Built-in classes will have no constant pools or any other associated
 * properties set. They will only contains a name and methods.
*/
Class *class_create_builtin(char *name, builtins *class_builtins)
{
    Class *class = malloc(sizeof(Class));
    memset(class, 0, sizeof(Class));
    class->built_in = true;
    class->name = name;

    if (class_builtins->parent)
        class->parent = classes_get_class(class_builtins->parent);

    class->methods_count = class_builtins->methods_length;
    class->methods = methods_new_builtin(class, class_builtins);
    class->fields = fields_new_builtin(class, class_builtins);

    class->static_initialized = false;

    return class;
}

Method *class_get_method(Class *class, const char *name, const char *descriptor)
{
    while (class) {
        for (int i = 0; i < class->methods_count; i++) {
            Method *method = &class->methods[i];
            if (!strcmp(method->name, name) && !strcmp(method->descriptors->descriptor, descriptor))
            {
                if ((method->info.access_flags & ACC_NATIVE) && !method->native_method)
                {
                    method->native_method = jni_resolve_method(get_jni(), class->name, method->name, method->descriptors);
                    method->max_stack = !!(method->descriptors->return_descriptor.type != DESCRIPTOR_VOID);
                    method->max_local = method->descriptors->arguments_count + !!(method->info.access_flags & ACC_STATIC);
                }
                return method;
            }
        }
        class = class->parent;
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

bool class_is_subclass(Class *child, Class *parent)
{
    Class *current = child;
    while (current) {
        if (current == parent)
            return true;
        current = current->parent;
    }
    return false;
}

bool classes_add_class(Class *class)
{
    if (!s_classes || !class)
        return false;

    class->classes = s_classes;
    s_classes->classes = realloc(s_classes->classes, (sizeof(Class*) * (s_classes->count + 1)));
    s_classes->classes[s_classes->count++] = class;

    return true;
}

static Class *resolve_unknown_class(const char *name)
{
    char class_path[2048];
    snprintf(class_path, sizeof(class_path), "%s.class", name);
    printf("Trying to load class from path %s\n", class_path);
    Class *class = class_parse_file(class_path);
    if (class) {
        if (!classes_add_class(class)) {
            fprintf(stderr, "Failed to resolve class %s\n", name);
            return NULL;
        }
    } else {
        fprintf(stderr, "Failed to resolve class %s\n", name);
        return NULL;
    }

    return class;
}

Class *classes_get_class(const char *name)
{
    if (!s_classes || !name)
        return NULL;

    for (int i = 0; i < s_classes->count; i++) {
        Class *class = s_classes->classes[i];
        if (!strcmp(class->name, name))
            return class;
    }

    // Try resolving too
    Class *class = resolve_unknown_class(name);

    return class;
}

/* TODO: Fix this method */
Class *classes_get_class_from_index(ConstantPool *pool, uint16_t index)
{
    const char *name = constant_pool_resolve_class_name(pool, index);
    Class *class = classes_get_class(name);

    if (!class)
    {
        class = resolve_unknown_class(name);
    }

    return class;
}

Method *classes_get_main_method()
{
    for (int i = 0; i < s_classes->count; i++) {
        Class *class = s_classes->classes[i];
        for (int j = 0; j < class->methods_count; j++) {
            Method *method = &class->methods[j];
            if (!strcmp(method->name, "main")) {
                // We found the main method. Great. Mark this as our main class.
                printf("Found method main in class %s\n", class->name);
                s_classes->main_class = class;
                return &class->methods[j];
            }
        }
    }

    return NULL;
}

void classes_new()
{
    s_classes = malloc(sizeof(Classes));
    s_classes->count = 0;
    s_classes->classes = NULL;
}

void classes_free()
{
    for (int i = 0; i < s_classes->count; i++) {
        Class *class = s_classes->classes[i];
        class_free(class);
    }
    free(s_classes->classes);
    free(s_classes);
}
