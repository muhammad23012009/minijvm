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
void java_io_PrintStream_println(Method *method, Frame *frame, char *descriptor_str)
{
    /* We do not have any descriptors. Parse them ourselves */
    Descriptors *descriptors = descriptors_new(descriptor_str);

    Class *printstream = frame->locals->items[0].data.ref;

    /* Figure out what values we need to print */
    for (int i = 0; i < descriptors->arguments_count; i++) {
        Variant item = frame->locals->items[i + 1];
        Descriptor desc = descriptors->arguments[i];
        switch (desc.type) {
            case DESCRIPTOR_INT:
                printf("%d\n", item.data.int_val);
                break;
            case DESCRIPTOR_OBJECT:
                /* Probably a string? */
                printf("%s\n", item.data.ref);
                break;
            default:
                printf("FUCK!\n");
        }
    }

    descriptors_free(descriptors);
}

builtins java_io_PrintStream_methods[] = {
    { "println", "", &java_io_PrintStream_println },
};
int java_io_PrintStream_methods_length = ARRAY_SIZE(java_io_PrintStream_methods);