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

#ifndef OBJECT_H
#define OBJECT_H

/* Each object is created whenever bytecode invokes the `new` method. 
 * The object holds fields and their associated values.
*/

#include <stdlib.h>
#include <stdbool.h>
#include "method.h"

typedef struct Class Class;
typedef struct Field Field;

typedef struct Object {
    Class *class;
    bool initialized;

    uint16_t fields_count;
    Field **fields;
} Object;

extern Field *object_get_field(Object *object, char *field_name);
extern Object *object_new(Class *class);
extern void object_free(Object *object);

#endif