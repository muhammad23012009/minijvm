#include "array.h"

Array *array_new(Class *c, int count)
{
    Array *array = malloc(sizeof(Array));
    memset(array, 0, sizeof(*array));

    array->parent_class = c;
    array->count = count;
    array->value = calloc(count, sizeof(Variant));

    return array;
}

void array_set_value(Array *array, int index, Variant value)
{
    variant_release(&array->value[index]);
    array->value[index] = value;
}

void array_free(Array *array)
{
    if (!array)
        return;

    for (int i = 0; i < array->count; i++) {
        variant_release(&array->value[i]);
    }

    free(array->value);
    free(array);
}