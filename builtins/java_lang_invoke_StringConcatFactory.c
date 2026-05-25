#include "builtins.h"
#include <stdarg.h>
#include "../dynarr.h"

#define _GNU_SOURCE
#include <stdio.h>

static int count_placeholders(const char *recipe)
{
    int count = 0;
    while (*recipe) {
        if (*recipe == 1 || *recipe == 2)
            count++;

        recipe++;
    }

    return count;
}

// I am too lazy.
static void store_ref_in_variant(Field* field, void *ref)
{
    field->value = variant_make_owned_ref(ref);
}

static char* get_str_from_obj(Object* obj)
{
    return (char*)object_get_field(obj, "value")->value.data.ref;
}

// TODO: Figure out a way to create the format string while creating the JIT function and stuffing it in there.
static char* create_format_string(const char* recipe, const char* descriptor)
{
    // Since we're already counting placeholders, we just need to add the number of placeholders to the length to get the format string length
    // i.e. Hello \1 becomes Hello %s.
    char* format_str = malloc(strlen(recipe) + count_placeholders(recipe) + 1);
    memset(format_str, 0, strlen(recipe) + count_placeholders(recipe) + 1);
    char* start;
    Descriptors *descriptors = descriptors_new(descriptor);
    int descriptors_index = 0;

    while ((start = strchr(recipe, 1)) || (start = strchr(recipe, 2)))
    {
        strncat(format_str, recipe, start - recipe);

        if (*start == 1 || *start == 2)
        {
            if (descriptors->arguments[descriptors_index].type == DESCRIPTOR_OBJECT)
                strcat(format_str, "%s");
            else if (descriptors->arguments[descriptors_index].type == DESCRIPTOR_INT)
                strcat(format_str, "%d");
            else if (descriptors->arguments[descriptors_index].type == DESCRIPTOR_CHAR)
                strcat(format_str, "%c");

            descriptors_index++;
        }

        recipe = start + 1;
    }

    descriptors_free(descriptors);

    strcat(format_str, recipe);

    return format_str;
}

Object* java_lang_invoke_StringConcatFactory_makeConcatWithConstants(Object *lookup, Object *name, Object *type, Object* recipe, Array *args)
{
    // Generate a format string for using with JIT later.
    char* start;
    char* recipe_str = (char*)object_get_field(recipe, "value")->value.data.ref;
    Descriptors *descriptors = descriptors_new(object_get_field(type, "descriptor")->value.data.ref);
    int arguments = descriptors->arguments_count;

    jit_type_t params[arguments];
    for (int i = 0; i < arguments; i++) {
        // All operands will be String objects
        params[i] = jit_type_void_ptr;
    }

    // For some reason, libJIT docs do not tell you that signatures are allocated on the heap.
    jit_type_t *signatures = arr_init(jit_type_t);
    jit_type_t signature;
    // Now generate the JIT method for concatenating the strings.
    jit_context_build_start(get_jit_context());
    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, params, arguments, 0);
    arr_push(signatures, signature);

    jit_function_t func = jit_function_create(get_jit_context(), signature);

    jit_value_t descriptor_jit = jit_value_create_long_constant(func, jit_type_void_ptr, (uintptr_t)object_get_field(type, "descriptor")->value.data.ref);

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, (jit_type_t[]){ jit_type_void_ptr, jit_type_void_ptr }, 2, 0);
    arr_push(signatures, signature);

    jit_value_t format_str = jit_insn_call_native(func, "create_format_string", create_format_string,
                                                  signature,
                                                  (jit_value_t[]){ jit_value_create_long_constant(func, jit_type_void_ptr, (uintptr_t)recipe_str), descriptor_jit }, 2, JIT_CALL_NOTHROW);
    jit_type_t asprintf_params[arguments + 2];
    for (int i = 0; i < arguments + 2; i++) {
        asprintf_params[i] = jit_type_void_ptr; // all arguments are strings
    }

    jit_value_t jit_args[arguments + 2];
    jit_value_t buffer = jit_value_create(func, jit_type_void_ptr);
    jit_value_t buffer_ptr = jit_insn_address_of(func, buffer);
    jit_args[0] = buffer_ptr;
    jit_args[1] = format_str;
    for (int i = 2; i < arguments + 2; i++) {
        if (descriptors->arguments[i - 2].type == DESCRIPTOR_OBJECT)
        {
            signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, (jit_type_t[]){ jit_type_void_ptr }, 1, 0);
            arr_push(signatures, signature);

            jit_args[i] = jit_insn_call_native(func, "get_str_from_obj", get_str_from_obj,
                                              signature,
                                              (jit_value_t[]){ jit_value_get_param(func, i - 2) }, 1, JIT_CALL_NOTHROW);
        } else {
            // For non-object types, we can just pass the argument directly to sprintf without any conversion.
            jit_args[i] = jit_value_get_param(func, i - 2);
        }
    }

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, asprintf_params, arguments + 2, 0);
    arr_push(signatures, signature);

    jit_value_t str_result = jit_insn_call_native(func, "asprintf", asprintf,
                                                  signature,
                                                  jit_args, arguments + 2, JIT_CALL_NOTHROW);

    const char* str_class_name = "java/lang/String";
    const char* value_field_name = "value";
    jit_value_t hhh = jit_value_create_long_constant(func, jit_type_void_ptr, (uintptr_t)str_class_name);
    jit_value_t hhh2 = jit_value_create_long_constant(func, jit_type_void_ptr, (uintptr_t)value_field_name);

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, (jit_type_t[]){ jit_type_void_ptr }, 1, 0);
    arr_push(signatures, signature);

    jit_value_t str_class = jit_insn_call_native(func, "classes_get_class", classes_get_class,
                                               signature,
                                               (jit_value_t[]){ hhh }, 1, JIT_CALL_NOTHROW);

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, (jit_type_t[]){ jit_type_void_ptr }, 1, 0);
    arr_push(signatures, signature);

    jit_value_t str_obj = jit_insn_call_native(func, "object_new_string", object_new,
                                               signature,
                                               (jit_value_t[]){ str_class }, 1, JIT_CALL_NOTHROW);

    // Now store the result of sprintf into the "value" field of the String object we just created.
    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void_ptr, (jit_type_t[]){ jit_type_void_ptr, jit_type_void_ptr }, 2, 0);
    arr_push(signatures, signature);

    jit_value_t value_field_ptr = jit_insn_call_native(func, "object_get_field_value_ptr", object_get_field,
                                               signature,
                                               (jit_value_t[]){ str_obj, hhh2 }, 2, JIT_CALL_NOTHROW);

    jit_value_t none = jit_insn_call_native(func, "store_string_in_field", store_ref_in_variant,
                                               signature,
                                               (jit_value_t[]){ value_field_ptr, buffer }, 2, JIT_CALL_NOTHROW);

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, (jit_type_t[]){ jit_type_void_ptr }, 1, 0);
    arr_push(signatures, signature);

    jit_insn_call_native(func, "free", free,
                          signature,
                          (jit_value_t[]){ format_str }, 1, JIT_CALL_NOTHROW);

    jit_insn_return(func, str_obj);
    jit_function_compile(func);
    jit_context_build_end(get_jit_context());

    for (int i = 0; i < arr_length(signatures); i++) {
        jit_type_free(signatures[i]);
    }
    arr_free(signatures);
    descriptors_free(descriptors);

    return java_lang_invoke_CallSite_create(func);
}

static builtin_methods methods[] = {
    {
      "makeConcatWithConstants",
      "(Ljava/lang/invoke/MethodHandles$Lookup;Ljava/lang/String;Ljava/lang/invoke/MethodType;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/invoke/CallSite;",
      0x0008, /* static */
      &java_lang_invoke_StringConcatFactory_makeConcatWithConstants
    },
};

builtins java_lang_invoke_StringConcatFactory_builtins = {
    .parent = "java/lang/Object",
    .fields = NULL,
    .methods = methods,
    .methods_length = 1,
};