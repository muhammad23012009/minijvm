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

    //printf("stack_push: pushing value of type %d at top %d\n", value.type, stack->top);
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
    if (stack->top - 1 < 0) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1);
    }

    Variant value = stack->items[--stack->top];
    variant_release(&value);
    //printf("Popping value of type %d from stack, new top is %d\n", value.type, stack->top);
    return value;
}

void stack_clear(Stack *stack)
{
    while (stack->top > 0) {
        variant_release(&stack->items[--stack->top]);
    }
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
    stack_clear(stack);

    free(stack->items);
    free(stack);
}
