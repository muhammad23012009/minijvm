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

#include "minijvm.h"
#include <stdio.h>

void stack_push(Stack *stack, Variant value)
{
    if (stack->top + 1 > stack->max_size) {
        fprintf(stderr, "Stack overflow!\n");
        exit(1);
    }

    variant_acquire(&value);
    stack->items[stack->top++] = value;
}

void stack_push_int(Stack *stack, int value)
{
    Variant variant = {0};
    variant.type = VARIANT_TYPE_INT;
    variant.data.int_val = value;
    stack_push(stack, variant);
}

void stack_push_ref(Stack *stack, void *value)
{
    Variant variant = {0};
    variant.type = VARIANT_TYPE_REF;
    variant.data.ref = value;
    stack_push(stack, variant);
}

void stack_push_object(Stack *stack, Object *value)
{
    Variant variant = {0};
    variant.type = VARIANT_TYPE_OBJECT;
    variant.data.object = value;
    stack_push(stack, variant);
}

/* Takes the top item, and duplicates it */
void stack_dup(Stack *stack)
{
    stack_push(stack, stack->items[stack->top - 1]);
}

Variant stack_pop(Stack *stack)
{
    Variant value = stack->items[--stack->top];
    variant_release(&value);
    return value;
}

Stack *stack_new(int max_size)
{
    Stack *stack = malloc(sizeof(Stack));
    /* The stack will hold at most `max_size` items */
    stack->max_size = max_size;
    stack->items = calloc(max_size, sizeof(Variant));
    stack->top = 0;

    return stack;
}

void stack_free(Stack *stack)
{
    for (int i = 0; i < stack->top; i++) {
        variant_release(&stack->items[i]);
    }

    free(stack->items);
    free(stack);
}
