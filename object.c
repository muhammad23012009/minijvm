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

#include "object.h"

/* TODO:
 * Add fields for objects 
 * Properly handle `initialized` field
 * Also maybe we want to add something else here?
 */

Field *object_get_field(Object *object, char *field_name)
{
    for (int i = 0; i < object->fields_count; i++) {
        Field *f = object->fields[i];
        if (!strcmp(f->name, field_name))
            return f;
    }

    return NULL;
}

Object *object_new(Class *class)
{
    Object *object = malloc(sizeof(Object));
    object->class = class;
    object->initialized = false;

    /* Parse all fields in the class */
    if (class->class_fields) {
        object->fields_count = class->class_fields->count;
        object->fields = malloc(sizeof(Field*) * object->fields_count);
        for (int i = 0; i < class->class_fields->count; i++) {
            FieldInfo *info = &class->class_fields->fields[i];
            Field *field = malloc(sizeof(Field));

            field->name = info->name.name;
            field->class = class;
            object->fields[i] = field;
        }
    } else {
        object->fields_count = 0;
        object->fields = NULL;
    }

    return object;
}

void object_free(Object *object)
{
    for (int i = 0; i < object->fields_count; i++)
        free(object->fields[i]);

    free(object->fields);
    free(object);
}