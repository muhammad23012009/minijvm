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

#include <unistd.h>
#include <stdio.h>

#include "builtins.h"
#include "../method.h"

void java_lang_System_clinit(Class *this)
{
    Class *printstream = classes_get_class("java/io/PrintStream");
    Field *field = class_get_static_field(this, "out");
    field->value.type = VARIANT_TYPE_OBJECT;
    field->value.data.object = object_new(printstream);

    field = class_get_static_field(this, "in");
    field->value.type = VARIANT_TYPE_OBJECT;
    field->value.data.object = object_new(classes_get_class("java/io/StdinInputStream"));

    Method *meow = class_get_method(classes_get_class("sun/misc/VM"), "saveAndRemoveProperties", "()V");
    method_execute(meow);
}

void java_lang_System_arraycopy(Object *srcobj, int srcPos, Object *destobj, int destPos, int length)
{
    if (!srcobj || !destobj) {
        fprintf(stderr, "NullPointerException in System.arraycopy\n");
        exit(1);
    }

    void *src = (Object**)srcobj->array;
    void *dest = (Object**)destobj->array;

    if (arr_type(src) != arr_type(dest)) {
        fprintf(stderr, "ArrayStoreException in System.arraycopy: incompatible array types\n");
        exit(1);
    }

    printf("Performing arraycopy from src %p at position %d to dest %p at position %d with length %d\n", (void*)src, srcPos, (void*)dest, destPos, length);
    if (destPos + length > arr_capacity(dest) || srcPos + length > arr_capacity(src)) {
        fprintf(stderr, "IndexOutOfBoundsException in System.arraycopy: source or destination position out of bounds\n");
        exit(1);
    }

    // ew what is this
    for (int i = 0; i < length; ++i) {
        if (arr_type(src) == ARRAY_TYPE_OBJECT) {
            ((Object**)dest)[destPos + i] = ((Object**)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_INT) {
            ((int*)dest)[destPos + i] = ((int*)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_LONG) {
            ((int64_t*)dest)[destPos + i] = ((int64_t*)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_FLOAT) {
            ((float*)dest)[destPos + i] = ((float*)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_DOUBLE) {
            ((double*)dest)[destPos + i] = ((double*)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_CHAR) {
            ((uint16_t*)dest)[destPos + i] = ((uint16_t*)src)[srcPos + i];
        } else if (arr_type(src) == ARRAY_TYPE_BYTE) {
            ((uint8_t*)dest)[destPos + i] = ((uint8_t*)src)[srcPos + i];
        }
    }
}

void java_lang_System_loadLibrary(Object *name)
{
    // Make this call into Runtime.getRuntime().loadLibrary(name) eventually
    const char format[] = "lib%s.so";
    char *lib_name;

    asprintf(&lib_name, format, object_get_field(name, "value")->value.data.ref);
    jni_load(get_jni(), lib_name);
    free(lib_name);
}

long java_lang_System_currentTimeMillis()
{
    return get_current_time();
}

Object *java_lang_System_getProperty(Object *name)
{
    Object *str = object_new(classes_get_class("java/lang/String"));
    char *pp = object_get_field(name, "value")->value.data.ref;

    printf("Fetching property '%s'\n", pp);
    // Hardcode some properties
    if (!strcmp(pp, "file.separator")) {
        object_set_field(str, "value", variant_make_ref("/"));
        return str;
    } else if (!strcmp(pp, "line.separator")) {
        object_set_field(str, "value", variant_make_ref("\n"));
        return str;
    } else if (!strcmp(pp, "path.separator")) {
        object_set_field(str, "value", variant_make_ref(":"));
        return str;
    } else if (!strcmp(pp, "java.vm.name")) {
        object_set_field(str, "value", variant_make_ref("MiniJVM"));
        return str;
    } else if (!strcmp(pp, "java.vm.version")) {
        object_set_field(str, "value", variant_make_ref("1.0"));
        return str;
    } else if (!strcmp(pp, "java.vm.vendor")) {
        object_set_field(str, "value", variant_make_ref("MiniJVM Project"));
        return str;
    } else if (!strcmp(pp, "java.home")) {
        char path[2048];
        getcwd(path, sizeof(path));
        object_set_field(str, "value", variant_make_ref(path));
        return str;
    }

    char *p = pp;
    // Replace decimals with underscore
    while (p && *p) {
        if (*p == '.') {
            *p = '_';
        }
        p++;
    }

    char *env = getenv(pp);
    object_set_field(str, "value", variant_make_ref(env ? env : ""));
    return str;
}

Object *java_lang_System_getSecurityManager()
{
    return NULL;
}

static builtin_fields fields[] = {
    { "out", "Ljava/io/PrintStream;", ACC_STATIC },
    { "in", "Ljava/io/InputStream;", ACC_STATIC },
};

static builtin_methods methods[] = {
    { "<clinit>", "()V", 0, &java_lang_System_clinit },
    { "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", ACC_STATIC, &java_lang_System_arraycopy },
    { "loadLibrary", "(Ljava/lang/String;)V", ACC_STATIC, &java_lang_System_loadLibrary },
    { "currentTimeMillis", "()J", ACC_STATIC, &java_lang_System_currentTimeMillis },
    { "getProperty", "(Ljava/lang/String;)Ljava/lang/String;", ACC_STATIC, &java_lang_System_getProperty },
    { "getSecurityManager", "()Ljava/lang/SecurityManager;", ACC_STATIC, &java_lang_System_getSecurityManager },
};

builtins java_lang_System_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};