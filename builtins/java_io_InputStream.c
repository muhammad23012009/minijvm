#include "builtins.h"

int java_io_InputStream_read_bytearray_extended(Object *this, void *bytearray, int offset, int length)
{
    if (!length)
        return 0;

    Method *read_method = class_get_method(this->class, "read", "()I");
    Variant byte = method_execute(read_method);
    if (byte.data.int_val == -1)
        return -1;

    uint8_t *bytearray_data = (uint8_t*)bytearray;
    bytearray_data[offset] = byte.data.int_val;
    int i = 1;

    for (; i < length; i++) {
        byte = method_execute(read_method);
        if (byte.data.int_val == -1)
            break;

        bytearray_data[offset + i] = byte.data.int_val;
    }

    return i;
}

int java_io_InputStream_read_bytearray(Object *this, void *bytearray)
{
    return java_io_InputStream_read_bytearray_extended(this, bytearray, 0, arr_capacity(bytearray));
}

static builtin_methods methods[] = {
    { "read", "()I", ACC_ABSTRACT, NULL },
    { "read", "([B)I", 0, &java_io_InputStream_read_bytearray },
    { "read", "([BII)I", 0, &java_io_InputStream_read_bytearray_extended },
    { "close", "()V", ACC_ABSTRACT, NULL },
};

builtins java_io_InputStream_builtins = {
    .parent = "java/lang/Object",
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};