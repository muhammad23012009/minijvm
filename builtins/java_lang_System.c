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

void java_lang_System_clinit(Method *method, Frame *frame)
{
    Class *printstream = classes_get_class(method->class->classes, "java/io/PrintStream");
    Field *field = class_get_static_field(method->class, "out");
    field->value.data.object = object_new(printstream);
}

static builtin_fields fields[] = {
    { "out", 0x0008 },
};

static builtin_methods methods[] = {
    { "<clinit>", "()V", 0, &java_lang_System_clinit },
};

builtins java_lang_System_builtins = {
    .parent = "java/lang/Object",
    .fields = fields,
    .fields_length = ARRAY_SIZE(fields),
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};