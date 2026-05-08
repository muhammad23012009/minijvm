#include "builtins.h"
#include <string.h>

Object* java_lang_String_replace(Object *this, uint16_t oldChar, uint16_t newChar)
{
    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    char *value = strdup((char*)object_get_field(this, "value")->value.data.ref);
    char *ptr = value;

    while (ptr && *ptr) {
        if (*ptr == (oldChar & 0xFF)) {
            *ptr = newChar & 0xFF;
        }

        if (*(ptr + 1) == ((oldChar >> 8) & 0xFF)) {
            *(ptr + 1) = (newChar >> 8) & 0xFF;
        }
        ptr++;
    }

    object_set_field(str_obj, "value", variant_make_owned_ref(value));
    return str_obj;
}

static builtin_fields fields[] = {
    { "value", "", 0x0000 }, // handle ACC_PRIVATE later
};

static builtin_methods methods[] = {
    { "replace", "(CC)Ljava/lang/String;", 0, &java_lang_String_replace },
};

builtins java_lang_String_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = 1,
    .methods = methods,
    .methods_length = 1,
};