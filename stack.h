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

#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include "variant.h"

typedef struct Variant Variant;
typedef struct Object Object;

typedef struct Stack {
    int max_size;
    int count;
    int top;
    Variant *items;
} Stack;

extern void stack_push(Stack *stack, Variant value);

extern void stack_push_int(Stack *stack, int value);
extern void stack_push_ref(Stack *stack, void *value);
extern void stack_push_object(Stack *stack, Object *value);

extern void stack_dup(Stack *stack);
extern Variant stack_pop(Stack *stack);

extern Stack *stack_new(int max_size);
extern void stack_free(Stack *stack);

#endif