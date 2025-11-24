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

static char *help_text = "miniJVM: a stupidly simple JVM. \n\
Usage: ./miniJVM <class name>\n";

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "miniJVM: invalid arguments!\n");
        fprintf(stderr, help_text);
        return 1;
    }

    if (argc > 2) {
        fprintf(stderr, "miniJVM: too many arguments!\n");
        return 1;
    }

    char filename[2048];
    snprintf(filename, 2048, "%s%s", argv[1], ".class");

    Classes *classes = classes_new();

    /* Setup built-in classes and methods */
    classes_add_class(classes, class_create_builtin("java/lang/Object", &java_lang_Object_builtins, classes));
    classes_add_class(classes, class_create_builtin("java/util/Objects", &java_util_Objects_builtins, classes));
    classes_add_class(classes, class_create_builtin("java/lang/System", &java_lang_System_builtins, classes));
    classes_add_class(classes, class_create_builtin("java/io/PrintStream", &java_io_PrintStream_builtins, classes));
    classes_add_class(classes, class_create_builtin("java/lang/String", &java_lang_String_builtins, classes));
    classes_add_class(classes, class_create_builtin("java/lang/invoke/StringConcatFactory", &java_lang_invoke_StringConcatFactory_builtins, classes));

    if (!classes_add_class(classes, class_parse_file(classes, filename))) {
        classes_free(classes);
        return 1;
    }

    printf("miniJVM: all classes processed!\n");

    Method *main_method = classes_get_main_method(classes);
    if (!main_method) {
        fprintf(stderr, "Failed to find main method. Exiting!\n");
        classes_free(classes);
        return 1;
    }

    Frame *main_frame = frame_new(main_method->max_stack, main_method->max_local);

    method_execute(main_method, main_frame);

    classes_free(classes);
    frame_free(main_frame);
    return 0;
}
