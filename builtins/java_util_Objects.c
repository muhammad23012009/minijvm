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

/* Arguments: Reference to java/lang/Object
 * Returns: Reference to java/lang/Object
*/
void java_util_Objects_requireNonNull(Method *method, Frame *frame)
{
    Object *object = frame->locals[0].data.object;
    if (!object->initialized)
        printf("Uninitialized object!\n");

    stack_push_object(frame->stack, object);
}

static builtin_methods methods[] = {
    { "requireNonNull", "(Ljava/lang/Object;)Ljava/lang/Object", 1, &java_util_Objects_requireNonNull },
};

builtins java_util_Objects_builtins = {
    .parent = "java/lang/Object",
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};