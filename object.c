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

#define _GNU_SOURCE

#include <stdarg.h>
#include "object.h"
#include "gc.h"
#include "dynarr.h"
#include <pthread.h>

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
    Field *fields = arr_init(sizeof(Field));
    Class *object_class = class;
    Object *ret;
    int fields_count = 0;

    while (class) {
        if (class->fields && class->fields->fields_count) {
            fields_count += class->fields->fields_count;

            for (int i = 0; i < class->fields->fields_count; ++i) {
                Field f = class->fields->fields[i];
                arr_push(fields, f);
            }
        }

        class = class->parent;
    }

    ret = malloc(sizeof(Object) + sizeof(Field) * fields_count);
    memset(ret, 0, sizeof(Object) + sizeof(Field) * fields_count);
    ret->class = object_class;
    ret->fields_count = fields_count;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&ret->lock, &attr);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_init(&ret->cond, NULL);

    if (ret->fields_count) {
        memcpy(ret->fields, fields, sizeof(Field) * fields_count);
        arr_free(fields);
    }

    //gc_track_object(object);
    return ret;
}

void object_free(Object *object)
{
    for (int i = 0; i < object->fields_count; i++) {
        variant_release(&object->fields[i].value);
    }

    pthread_cond_destroy(&object->cond);
    pthread_mutex_destroy(&object->lock);
    // The class will actually free the relevant fields, we just need to free the copy we took.
    free(object);
}
