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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "minijvm.h"
#include "reader.h"
#include "builtins/builtins.h"

int main(void)
{
    /* TODO: Parse multiple class files and create maps */
    Classes *classes = classes_new();

    /* Setup built-in classes and methods */
    Class *java_lang_System = class_create_builtin("java/lang/System");
    classes_add_class(classes, java_lang_System);
    for (int i = 0; i < java_lang_System_methods_length; i++) {
        builtins s = java_lang_System_methods[i];
        class_add_builtin_method(java_lang_System, s.name, s.method);
    }

    Class *java_io_PrintStream = class_create_builtin("java/io/PrintStream");
    classes_add_class(classes, java_io_PrintStream);
    for (int i = 0; i < java_io_PrintStream_methods_length; i++) {
        builtins s = java_io_PrintStream_methods[i];
        class_add_builtin_method(java_io_PrintStream, s.name, s.method);
    }

    classes_add_class(classes, class_parse_file("Test.class"));

    Method *main_method = classes_get_main_method(classes);
    Frame *main_frame = frame_new(main_method->max_stack, main_method->max_local);

    method_execute(main_method, main_frame);

    classes_free(classes);
    frame_free(main_frame);
    /* TODO: Implement proper stack */
}