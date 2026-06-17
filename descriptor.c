#include "descriptor.h"
#include <stdio.h>

const char *get_argument_end(const char *argument_start)
{
    const char *str = argument_start;
    while (*str != ')' && *str != '\0') {
        str++;
    }

    return str;
}

int get_descriptor_count(const char *descriptor)
{
    int count = 0;

    /* Skip '(' */
    if (*descriptor == '(')
        descriptor++;

    while (*descriptor != ')' && *descriptor != '\0') {
        while (*descriptor == '[')
            descriptor++;

        if (*descriptor == 'L') {
            descriptor = strchr(descriptor, ';');
        }

        descriptor++;
        count++;
    }

    return count;
}

Descriptor parse_descriptor(const char **string, const char *end)
{
    Descriptor descriptor;
    descriptor.array_dimesions_count = 0;

    if (**string == '(')
        *string = *string + 1;

    if (*string == end) {
        // No arguments.
        descriptor.type = DESCRIPTOR_VOID;
        return descriptor;
    }

    while (**string == '[') {
        descriptor.array_dimesions_count++;
        *string = *string + 1;
    }

    switch (**string) {
        /* Figure out the basic types first */
        case 'V':
            descriptor.type = DESCRIPTOR_VOID;
            break;
        case 'I': 
            descriptor.type = DESCRIPTOR_INT;
            break;
        case 'C':
            descriptor.type = DESCRIPTOR_CHAR;
            break;
        case 'Z':
            descriptor.type = DESCRIPTOR_BOOL;
            break;
        case 'J':
            descriptor.type = DESCRIPTOR_LONG;
            break;
        case 'F':
            descriptor.type = DESCRIPTOR_FLOAT;
            break;
        case 'D':
            descriptor.type = DESCRIPTOR_DOUBLE;
            break;
        case 'L': {
            /* Object type */
            char* obj_end = strchr(*string, ';');
            descriptor.type = DESCRIPTOR_OBJECT;
            descriptor.object_name = strndup(++*string, obj_end - *string);
            /* Skip the entire string */
            *string = obj_end + 1;
            break;
        }
        default: {
            /* No arguments. */
            descriptor.type = DESCRIPTOR_VOID;
            descriptor.object_name = NULL;
            break;
        }
    }

    (void)*string++;
    return descriptor;
}

Descriptors *descriptors_new(const char *descriptor_str)
{
    Descriptors *descriptors = malloc(sizeof(Descriptors));
    const char *argument_start = descriptor_str;
    const char *argument_end = get_argument_end(argument_start);
    const char *returns_start = *argument_end != '\0' ? argument_end + 1 : NULL;
    descriptors->arguments = NULL;
    descriptors->descriptor = descriptor_str;
    descriptors->arguments_count = get_descriptor_count(argument_start);

    if (descriptors->arguments_count)
    {
        descriptors->arguments = calloc(descriptors->arguments_count, sizeof(Descriptor));

        for (int i = 0; i < descriptors->arguments_count; i++) {
            descriptors->arguments[i] = parse_descriptor(&argument_start, argument_end);
        }
    }

    if (returns_start)
        descriptors->return_descriptor = parse_descriptor(&returns_start, NULL);

    return descriptors;
}

void descriptors_free(Descriptors *descriptors)
{
    if (!descriptors)
        return;

    if (descriptors->return_descriptor.type == DESCRIPTOR_OBJECT)
        free(descriptors->return_descriptor.object_name);

    for (int i = 0; i < descriptors->arguments_count; i++) {
        if (descriptors->arguments[i].type == DESCRIPTOR_OBJECT)
            free(descriptors->arguments[i].object_name);
    }

    free(descriptors->arguments);
    free(descriptors);
}