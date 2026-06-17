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

/* Maintain a list of Class's with their java/lang/Class objects */
Object *java_lang_Object_getClass(Object *this)
{
    Object *cobj = object_new(classes_get_class("java/lang/Class"));
    object_get_field(cobj, "class")->value.data.class = this->class;
    return cobj;
}

int java_lang_Object_hashCode(Object *this)
{
    return (int)(uintptr_t)this;
}

void java_lang_Object_init(Object *this)
{
    this->initialized = true;
}

bool java_lang_Object_equals(Object *this, Object *other)
{
    return this == other;
}

Object *java_lang_Object_toString(Object *this)
{
    char *str;
    asprintf(&str, "%s@%p", this->class->name, (void*) this);
    Object *str_obj = object_new(classes_get_class("java/lang/String"));
    object_set_field(str_obj, "value", variant_make_owned_ref(str));
    return str_obj;
}

static builtin_methods methods[] = {
    { "getClass", "()Ljava/lang/Class;", 0, &java_lang_Object_getClass },
    { "hashCode", "()I", 0, &java_lang_Object_hashCode },
    { "equals", "(Ljava/lang/Object;)Z", 0, &java_lang_Object_equals },
    { "toString", "()Ljava/lang/String;", 0, &java_lang_Object_toString },
    { "<init>", "()V", 0, &java_lang_Object_init },
};

builtins java_lang_Object_builtins = {
    .parent = NULL, /* We are the ultimate parent. */
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};