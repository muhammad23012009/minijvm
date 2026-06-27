#include "builtins.h"
#include <string.h>

Object* java_lang_String_replace(Object *this, uint16_t oldChar, uint16_t newChar)
{
    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
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

void java_lang_String_getChars(Object *this, int srcBegin, int srcEnd, Object *dstobj, int dstBegin)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    int len = srcEnd - srcBegin;

    uint16_t *dst = (uint16_t*)dstobj->array;

    if (srcBegin < 0 || srcEnd > strlen(value) || dstBegin < 0 || dstBegin + len > arr_capacity(dst)) {
        printf("Index out of bounds in String.getChars\n");
        exit(1);
    }

    for (int i = 0; i < len; ++i) {
        dst[dstBegin + i] = value[srcBegin + i];
    }
}

int java_lang_String_length(Object *this)
{
    // TODO: use the built-in length field
    const char *value = (char*)object_get_field(this, "value")->value.data.ref;
    int len = strlen(value);
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

Object *java_lang_String_valueOf_I(int value)
{
    char *str;
    asprintf(&str, "%d", value);
    Object *str_obj = java_lang_String_new(str);
    free(str);
    return str_obj;
}

bool java_lang_String_isEmpty(Object *this)
{
    const char *value = (char*)object_get_field(this, "value")->value.data.ref;
    return strlen(value) == 0;
}

int java_lang_String_indexOf(Object *this, int ch)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    char *p = strchr(value, ch);
    if (p) {
        return p - value;
    }
    return -1;
}

int java_lang_String_indexOf_string(Object *this, Object *str_obj)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    char *str_value = (char*)object_get_field(str_obj, "value")->value.data.ref;
    char *p = strstr(value, str_value);
    if (p) {
        return p - value;
    }
    return -1;
}

int java_lang_String_lastIndexOf_I(Object *this, int ch)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    char *p = &value[strlen(value) - 1];
    while (p >= value) {
        if (*p == ch)
            return p - value;
        p--;
    }
    return -1;
}

Object *java_lang_String_substring_I(Object *this, int beginIndex)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    char *substr = strdup(value + beginIndex);

    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
    object_set_field(str_obj, "value", variant_make_owned_ref(substr));
    return str_obj;
}

Object *java_lang_String_substring_II(Object *this, int beginIndex, int endIndex)
{
    char *value = (char*)object_get_field(this, "value")->value.data.ref;
    int len = endIndex - beginIndex;
    char *substr = malloc(len + 1);
    strncpy(substr, value + beginIndex, len);
    substr[len] = '\0';

    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    str_obj->initialized = true;
    object_set_field(str_obj, "value", variant_make_owned_ref(substr));
    return str_obj;
}

int java_lang_String_hashcode(Object *this)
{
    const char *value = (char*)object_get_field(this, "value")->value.data.ref;
    int hash = 0;
    for (int i = 0; value[i] != '\0'; i++) {
        hash = 31 * hash + value[i];
    }
    return hash;
}

bool java_lang_String_equals(Object *this, Object *other)
{
    if (this == other) {
        return true;
    }

    if (!other || other->class != this->class) {
        return false;
    }

    const char *this_value = (char*)object_get_field(this, "value")->value.data.ref;
    const char *other_value = (char*)object_get_field(other, "value")->value.data.ref;

    return strcmp(this_value, other_value) == 0;
}

void java_lang_String_init_chararray(Object *this, Object *chararrayobject)
{
    uint16_t* chararray = (uint16_t*)chararrayobject->array;
    char *value = malloc(arr_capacity(chararray) + 1);
    for (int i = 0; i < arr_capacity(chararray); ++i)
        value[i] = chararray[i] & 0xFF;

    value[arr_capacity(chararray)] = '\0';
    object_set_field(this, "value", variant_make_owned_ref(value));
}

void java_lang_String_init_chararray_off_len(Object *this, Object *chararrayobj, int offset, int length)
{
    printf("Creating a new string from char array with offset %d and length %d\n", offset, length);
    char *value = malloc(length + 1);
    uint16_t* chararray = (uint16_t*)chararrayobj->array;

    for (int i = 0; i < length; ++i)
        value[i] = chararray[offset + i] & 0xFF;

    value[length] = '\0';
    object_set_field(this, "value", variant_make_owned_ref(value));
    printf("Created string with value '%s'\n", value);
}

static builtin_fields fields[] = {
    { "value", "", 0x0000 }, // handle ACC_PRIVATE later
    { "length", "", 0x0000 }
};

static builtin_methods methods[] = {
    { "replace",     "(CC)Ljava/lang/String;", 0, &java_lang_String_replace },
    { "getChars",    "(II[CI)V", 0, &java_lang_String_getChars },
    { "length",      "()I", 0, &java_lang_String_length },
    { "charAt",      "(I)C", 0, &java_lang_String_charAt },
    { "valueOf",     "(Ljava/lang/Object;)Ljava/lang/String;", ACC_STATIC, &java_lang_String_valueOf_object },
    { "valueOf",     "(I)Ljava/lang/String;", ACC_STATIC, &java_lang_String_valueOf_I },
    { "isEmpty",     "()Z", 0, &java_lang_String_isEmpty },
    { "indexOf",     "(I)I", 0, &java_lang_String_indexOf },
    { "indexOf",     "(Ljava/lang/String;)I", 0, &java_lang_String_indexOf_string },
    { "lastIndexOf", "(I)I", 0, &java_lang_String_lastIndexOf_I },
    { "substring",   "(I)Ljava/lang/String;", 0, &java_lang_String_substring_I },
    { "substring",   "(II)Ljava/lang/String;", 0, &java_lang_String_substring_II },
    { "hashCode",    "()I", 0, &java_lang_String_hashcode },
    { "equals",      "(Ljava/lang/Object;)Z", 0, &java_lang_String_equals },
    { "<init>",      "([C)V", 0, &java_lang_String_init_chararray },
    { "<init>",      "([CII)V", 0, &java_lang_String_init_chararray_off_len },
};

builtins java_lang_String_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};