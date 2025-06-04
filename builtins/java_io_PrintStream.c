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

/* Arguments: Reference to Java String
 * Returns: Void
*/
void java_io_PrintStream_println(Method *method, Frame *frame)
{
    char *str = stack_pop(frame->stack)->data.ref;
    Class *printstream = stack_pop(frame->stack)->data.ref;
    printf("%s\n", str);
}

builtins java_io_PrintStream_methods[] = {
    { "println", &java_io_PrintStream_println },
};
int java_io_PrintStream_methods_length = ARRAY_SIZE(java_io_PrintStream_methods);