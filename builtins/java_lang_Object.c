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
    Object *cobj = object_new(classes_get_class(this->class->classes, "java/lang/Class"));
    object_get_field(cobj, "class")->value.data.class = this->class;
    return cobj;
}

void java_lang_Object_init(Object *this)
{
    this->initialized = true;
    printf("Initialized object of class %s\n", this->class->name);
}

static builtin_methods methods[] = {
    { "getClass", "()Ljava/lang/Class;", 0, &java_lang_Object_getClass },
    { "<init>", "()V", 0, &java_lang_Object_init },
};

builtins java_lang_Object_builtins = {
    .parent = NULL, /* We are the ultimate parent. */
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};