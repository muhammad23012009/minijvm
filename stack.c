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
    if (stack->head == NULL) {
        stack->head = malloc(sizeof(StackItem));
        stack->head->item = value;
        stack->head->next = NULL;
    } else {
        StackItem *new_item = malloc(sizeof(StackItem));
        new_item->item = value;
        new_item->next = stack->head;
        stack->head = new_item;
    }
}

void stack_push_int(Stack *stack, int value)
{
    Variant variant;
    variant.type = VARIANT_TYPE_INT;
    variant.data.int_val = value;
    stack_push(stack, variant);
}

void stack_push_ref(Stack *stack, void *value)
{
    Variant variant;
    variant.type = VARIANT_TYPE_REF;
    variant.data.ref = value;
    stack_push(stack, variant);
}

void stack_push_object(Stack *stack, Object *value)
{
    Variant variant;
    variant.type = VARIANT_TYPE_OBJECT;
    variant.data.object = value;
    stack_push(stack, variant);
}

/* Takes the top item, and duplicates it */
void stack_dup(Stack *stack)
{
    stack_push(stack, stack->head->item);
}

Variant stack_pop(Stack *stack)
{
    Variant value;
    if (!stack->head)
        return value;

    StackItem *old_head = stack->head;
    stack->head = old_head->next;
    value = old_head->item;
    free(old_head);

    return value;
}

Stack *stack_new(int max_size)
{
    Stack *stack = malloc(sizeof(Stack));
    stack->max_size = max_size;
    /* The stack will hold at most `max_size` items */
    stack->head = NULL;

    return stack;
}

void stack_free(Stack *stack)
{
    for (StackItem *item = stack->head; item != NULL;) {
        StackItem *next = item->next;

        if (item->item.type == VARIANT_TYPE_OBJECT)
            object_free(item->item.data.object);

        free(item);
        item = next;
    }

    free(stack);
}