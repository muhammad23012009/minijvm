#include "descriptor.h"
#include <stdio.h>

char *get_argument_end(char *argument_start)
{
    char *str = argument_start;
    while (*str != ')' && *str != '\0') {
        str++;
    }

    return str;
}

int get_descriptor_count(char *descriptor)
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

Descriptor parse_descriptor(char **string, char *end)
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
        case 'V': {
            descriptor.type = DESCRIPTOR_VOID;
            break;
        }
        case 'I': {
            descriptor.type = DESCRIPTOR_INT;
            break;
        }
        case 'L': {
            /* Object type */
            descriptor.type = DESCRIPTOR_OBJECT;
            descriptor.object_name = strndup(++*string, end - *string - 1);
            /* Skip the entire string */
            if (!(*string = strchr(*string, ';'))) {
                *string = NULL;
            }
            *string++;
            break;
        }
        default: {
            /* No arguments. */
            descriptor.type = DESCRIPTOR_VOID;
            descriptor.object_name = NULL;
            break;
        }
    }

    *string++;
    return descriptor;
}

Descriptors *descriptors_new(char *descriptor_str)
{
    Descriptors *descriptors = malloc(sizeof(Descriptors));
    char *argument_start = descriptor_str;
    char *argument_end = get_argument_end(argument_start);
    char *returns_start = *argument_end != '\0' ? argument_end + 1 : NULL;

    descriptors->arguments_count = get_descriptor_count(argument_start);

    descriptors->arguments = calloc(descriptors->arguments_count, sizeof(Descriptor));

    for (int i = 0; i < descriptors->arguments_count; i++) {
        descriptors->arguments[i] = parse_descriptor(&argument_start, argument_end);
    }

    if (returns_start)
        descriptors->return_descriptor = parse_descriptor(&returns_start, NULL);

    return descriptors;
}

void descriptors_free(Descriptors *descriptors)
{
    if (!descriptors)
        return;

    free(descriptors->arguments);
    free(descriptors);
}