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

/* Arguments: None
 * Returns: Void
*/
void java_lang_Object_init(Method *method, Frame *frame)
{
    Object *object = frame->locals[0].data.object;
    object->initialized = true;
}

static builtin_methods methods[] = {
    { "<init>", "()V", 0, &java_lang_Object_init },
};

builtins java_lang_Object_builtins = {
    .parent = NULL, /* We are the ultimate parent. */
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};