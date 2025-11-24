#ifndef ARRAY_H
#define ARRAY_H

#include "method.h"
#include "variant.h"

// TODO: Should I implement this as an Object directly or a Class?
typedef struct Array {
    Class *parent_class;
    int count;

    Variant *value;
} Array;

extern Array *array_new(Class *c, int count);
extern void array_set_value(Array *array, int index, Variant value);

#endif