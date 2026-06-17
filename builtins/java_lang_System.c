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

#include "builtins.h"
#include "../method.h"

void java_lang_System_clinit(Class *this)
{
    Class *printstream = classes_get_class("java/io/PrintStream");
    Field *field = class_get_static_field(this, "out");
    field->value.type = VARIANT_TYPE_OBJECT;
    field->value.data.object = object_new(printstream);
    field->value.data.object->parent_field = field;

    field = class_get_static_field(this, "in");
    field->value.type = VARIANT_TYPE_OBJECT;
    field->value.data.object = object_new(classes_get_class("java/io/StdinInputStream"));
    field->value.data.object->parent_field = field;
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

Object *java_lang_System_getProperty(Object *name)
{
    printf("Fetching property '%s'\n", (char*)object_get_field(name, "value")->value.data.ref);
    Object *str = object_new(classes_get_class("java/lang/String"));
    char *pp = object_get_field(name, "value")->value.data.ref;
    char *p = pp;
    // Replace decimals with underscore
    while (p && *p) {
        if (*p == '.') {
            *p = '_';
        }
        p++;
    }

    char *env = getenv(pp);
    printf("Converted env value is '%s'\n", env ? env : "");
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
    { "loadLibrary", "(Ljava/lang/String;)V", ACC_STATIC, &java_lang_System_loadLibrary },
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