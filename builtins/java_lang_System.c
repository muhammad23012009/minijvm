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

/* TODO: 
 * Implement proper fields 
 * Implement a way to derive arguments from method descriptor
*/

/* Arguments: None
 * Returns: Reference to PrintStream class
*/
void java_lang_System_out(Method *method, Frame *frame)
{
    Class *printstream = classes_get_class(method->class->classes, "java/io/PrintStream");
    StackData data;
    data.ref = printstream;
    stack_push(frame->stack, STACK_ITEM_TYPE_REF, data);
}

builtins java_lang_System_methods[] = {
    { "out", &java_lang_System_out },
};
int java_lang_System_methods_length = ARRAY_SIZE(java_lang_System_methods);