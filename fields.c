#include "fields.h"

Fields *fields_new(Class *class, Reader *reader, ConstantPool *pool)
{
    int total_count = reader_read_uint16_be(reader);
    if (total_count <= 0)
        return NULL;

    Fields *fields = malloc(sizeof(Fields));
    memset(fields, 0, sizeof(Fields));

    for (int i = 0; i < total_count; i++) {
        Field *field;
        uint16_t access_flags = reader_read_uint16_be(reader);

        /* I should probably use some vector implementation for this. */
        if (access_flags & 0x0008) { // ACC_STATIC
            fields->static_fields_count++;
            fields->static_fields = realloc(fields->static_fields, sizeof(Field) * fields->static_fields_count);
            field = &fields->static_fields[fields->static_fields_count - 1];
        } else {
            fields->fields_count++;
            fields->fields = realloc(fields->fields, sizeof(Field) * fields->fields_count);
            field = &fields->fields[fields->fields_count - 1];
        }

        /* Fill in FieldInfo */
        field->info.access_flags = access_flags;
        field->info.name = constant_pool_resolve_string(pool, reader_read_uint16_be(reader));
        field->info.descriptor = constant_pool_resolve_string(pool, reader_read_uint16_be(reader));
        field->info.attributes = attributes_new(reader, pool);

        field->name = field->info.name;
    }

    return fields;
}

Fields *fields_new_builtin(Class *class, builtins *class_builtins)
{
    int total_count = class_builtins->fields_length;
    if (total_count <= 0)
        return NULL;

    Fields *fields = malloc(sizeof(Fields));
    memset(fields, 0, sizeof(Fields));

    for (int i = 0; i < total_count; i++) {
        builtin_fields *bf = &class_builtins->fields[i];
        Field *field;
        if (bf->flags & 0x0008) { // ACC_STATIC
            fields->static_fields_count++;
            fields->static_fields = realloc(fields->static_fields, sizeof(Field) * fields->static_fields_count);
            field = &fields->static_fields[fields->static_fields_count - 1];
        } else {
            fields->fields_count++;
            fields->fields = realloc(fields->fields, sizeof(Field) * fields->fields_count);
            field = &fields->fields[fields->fields_count - 1];
        }

        /* Fill in FieldInfo */
        field->info.access_flags = bf->flags;
        field->info.name = bf->name;
        field->info.descriptor = bf->descriptor;
        field->info.attributes = NULL;

        field->name = bf->name;
    }

    return fields;
}

void fields_free(Fields *fields)
{
    if (!fields)
        return;

    for (int i = 0; i < fields->fields_count; i++) {
        Field *field = &fields->fields[i];
        attributes_free(field->info.attributes);
    }
    for (int i = 0; i < fields->static_fields_count; i++) {
        Field *field = &fields->static_fields[i];
        attributes_free(field->info.attributes);
    }
    free(fields->static_fields);
    free(fields->fields);
    free(fields);
}

Method *methods_new(Class *class, Reader *reader, ConstantPool *pool, int *count)
{
    *count = reader_read_uint16_be(reader);
    if (*count <= 0)
        return NULL;

    Method *methods = calloc(*count, sizeof(Method));

    for (int i = 0; i < *count; i++) {
        Method *method = &methods[i];
        method->class = class;

        /* Fill in MethodInfo */
        method->info.access_flags = reader_read_uint16_be(reader);
        method->info.name = constant_pool_resolve_string(pool, reader_read_uint16_be(reader));
        method->info.descriptor = constant_pool_resolve_string(pool, reader_read_uint16_be(reader));
        method->info.attributes = attributes_new(reader, pool);

        method->name = method->info.name;

        /* Parse Code attribute (if it exists) */
        AttributeInfo *code_attr = attributes_get_attribute(method->info.attributes, "Code");
        if (code_attr) {
            method->data_length = code_attr->CodeAttribute.code_length;
            method->data = code_attr->CodeAttribute.code;
            method->max_stack = code_attr->CodeAttribute.max_stack;
            method->max_local = code_attr->CodeAttribute.max_locals;
        }

        if (method->info.access_flags & ACC_NATIVE)
        {
            // We'll link this method when loading the native library.
        }

        method->descriptors = descriptors_new(method->info.descriptor);
    }

    return methods;
}

Method *methods_new_builtin(Class *class, builtins *class_builtins)
{
    int count = class_builtins->methods_length;
    if (count <= 0)
        return NULL;

    Method *methods = calloc(count, sizeof(Method));

    for (int i = 0; i < count; i++) {
        builtin_methods *bm = &class_builtins->methods[i];
        Method *method = &methods[i];

        method->class = class;
        method->name = bm->name;
        method->native_method = bm->method;
        method->descriptors = descriptors_new(bm->descriptor);

        /* We only allocate enough stack to push the return value */
        method->max_stack = DESCRIPTORS_GET_RETURN_TYPE(method->descriptors) != DESCRIPTOR_VOID;
        method->max_local = method->descriptors->arguments_count + 1; // +1 for `this`

        method->info.access_flags = bm->flags;
        method->info.name = bm->name;
        method->info.descriptor = bm->descriptor;
        method->info.attributes = NULL;
    }

    return methods;
}

void methods_free(Method *methods, int count)
{
    if (!methods)
        return;

    for (int i = 0; i < count; i++) {
        Method *method = &methods[i];
        attributes_free(method->info.attributes);
        descriptors_free(method->descriptors);
    }
    free(methods);
}