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

#include "stack.h"
#include <stdio.h>

void stack_push(Stack *stack, StackType type, StackData data)
{
    StackItem item;
    item.type = type;
    item.data = data;
    stack->items[stack->top++] = item;
}

/* Takes the top item, and duplicates it */
void stack_dup(Stack *stack)
{
    stack_push(stack, stack->items[stack->top].type, stack->items[stack->top].data);
}

StackItem *stack_pop(Stack *stack)
{
    if (stack->top <= 0)
        return NULL;

    return &stack->items[--stack->top];
}

Stack *stack_new(int max_size)
{
    Stack *stack = malloc(sizeof(Stack));
    stack->size = max_size;
    /* The stack will hold at most `max_size` items */
    stack->items = malloc(sizeof(StackItem) * max_size);
}

void stack_free(Stack *stack)
{
    free(stack->items);
    free(stack);
}