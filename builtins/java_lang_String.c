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

int java_lang_String_length(Object *this)
{
    // TODO: use the built-in length field
    const char *value = (char*)object_get_field(this, "value")->value.data.ref;
    int len = strlen(value);
    printf("Length of string '%s' is %d\n", value, len);
    return len;
}

uint16_t java_lang_String_charAt(Object *this, int index)
{
    // TODO: UTF-16!!!
    const char* value = (char*)object_get_field(this, "value")->value.data.ref;
    return value[index];
}

Object *java_lang_String_new(const char *value)
{
    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
    object_set_field(str_obj, "value", variant_make_owned_ref(strdup(value)));
    return str_obj;
}

Object *java_lang_String_valueOf_object(Object *obj)
{
    if (!obj)
        return java_lang_String_new("null");

    // TODO: Make method_execute call native methods too
    char *str;
    asprintf(&str, "%s@%p", obj->class->name, (void*) obj);
    Object *str_obj = java_lang_String_new(str);
    free(str);
    return str_obj;
}

void java_lang_String_init_chararray(Object *this, uint16_t* chararray)
{
    char *value = malloc(arr_length(chararray) + 1);
    for (int i = 0; i < arr_length(chararray); ++i)
        value[i] = chararray[i] & 0xFF;
    value[arr_length(chararray)] = '\0';
    object_set_field(this, "value", variant_make_owned_ref(value));
}

static builtin_fields fields[] = {
    { "value", "", 0x0000 }, // handle ACC_PRIVATE later
    { "length", "", 0x0000 }
};

static builtin_methods methods[] = {
    { "replace", "(CC)Ljava/lang/String;", 0, &java_lang_String_replace },
    { "length", "()I", 0, &java_lang_String_length },
    { "charAt", "(I)C", 0, &java_lang_String_charAt },
    { "valueOf", "(Ljava/lang/Object;)Ljava/lang/String;", ACC_STATIC, &java_lang_String_valueOf_object },
    { "<init>", "([C)V", 0, &java_lang_String_init_chararray },
};

builtins java_lang_String_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};