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

/* Arguments: Any (parsed automatically)
 * Returns: Void
*/

void java_io_PrintStream_println_str(Method *method, Frame *frame)
{
    Object *printstream = frame->locals[0].data.object;
    Object *str = frame->locals[1].data.object;
    printf("%s\n", object_get_field(str, "value")->value.data.ref);
}

void java_io_PrintStream_println_int(Method *method, Frame *frame)
{
    Object *printstream = frame->locals[0].data.object;
    int32_t value = frame->locals[1].data.int_val;
    printf("%d\n", value);
}

static builtin_methods methods[] = {
    { "println", "(Ljava/lang/String;)V", 0, &java_io_PrintStream_println_str },
    { "println", "(I)V", 0, &java_io_PrintStream_println_int },
};

builtins java_io_PrintStream_builtins = {
    .parent = NULL, /* TODO: implement all parents */
    .fields = NULL,
    .methods = methods,
    .methods_length = ARRAY_SIZE(methods),
};