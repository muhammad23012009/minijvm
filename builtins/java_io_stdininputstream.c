#include "builtins.h"

int java_io_StdinInputStream_read(Object *this)
{
    return getchar();
}

static builtin_methods methods[] = {
    { "read", "()I", 0, &java_io_StdinInputStream_read },
};

builtins java_io_StdinInputStream_builtins = {
    .parent = "java/io/InputStream",
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};