#include "array.h"

Array *array_new(Class *c, int count)
{
    Array *array = malloc(sizeof(Array));
    memset(array, 0, sizeof(*array));

    array->parent_class = c;
    array->count = count;
    array->value = calloc(count, sizeof(Variant));

    printf("Created new array of class %s with %d elements %p\n", c->name, count, array->value);

    return array;
}

void array_set_value(Array *array, int index, Variant value)
{
    array->value[index] = value;
}