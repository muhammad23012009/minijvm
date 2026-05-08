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
#include "gc.h"

/* TODO:
 * Add fields for objects 
 * Properly handle `initialized` field
 * Also maybe we want to add something else here?
 */

Field *object_get_field(Object *object, const char *field_name)
{
    for (int i = 0; i < object->fields_count; i++) {
        Field *f = &object->fields[i];
        if (!strcmp(f->name, field_name))
            return f;
    }

    return NULL;
}

void object_set_field(Object *object, const char *field_name, Variant value)
{
    Field *field = object_get_field(object, field_name);
    if (!field) {
        /* Uhhhh? */
        return;
    }

    field->value = value;
}

Object *object_new(Class *class)
{
    Object *object = malloc(sizeof(Object));
    object->class = class;
    object->initialized = false;
    object->parent_field = NULL;

    /* Parse all fields in the class */
    if (class->fields && class->fields->fields_count > 0) {
        object->fields_count = class->fields->fields_count;
        object->fields = malloc(sizeof(Field) * object->fields_count);
        memcpy(object->fields, class->fields->fields, sizeof(Field) * object->fields_count);
    } else {
        object->fields_count = 0;
        object->fields = NULL;
    }

    gc_track_object(object);
    return object;
}

void object_free(Object *object)
{
    // The class will actually free the relevant fields, we just need to free the copy we took.
    free(object->fields);
    free(object);
}
