#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

/* The `Arguments` object is parsed from a Java method descriptor,
 * and is passed to Java methods (native or interpreted) to figure out
 * what arguments we need to push to locals of method and then pop from that
 * method's stack.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    DESCRIPTOR_VOID,
    DESCRIPTOR_INT,
    DESCRIPTOR_OBJECT,
    DESCRIPTOR_CHAR,
    DESCRIPTOR_BOOL,
} DescriptorType;

typedef struct Descriptor {
    DescriptorType type;
    char *object_name;
    /* The number of dimensions of the array (if descriptor is an array) */
    int array_dimesions_count;
} Descriptor;

typedef struct Descriptors {
    int arguments_count;

    /* Java-style descriptor (e.g. ()V)
     * () means no arguments
     * V means void return
    */
    const char *descriptor;

    Descriptor *arguments;
    Descriptor return_descriptor;
} Descriptors;

extern int get_descriptor_count(const char *descriptor);
extern Descriptor parse_descriptor(const char **string, const char *end);

extern Descriptors *descriptors_new(const char *descriptor);
extern void descriptors_free(Descriptors *descriptor);

#define FOREACH_DESCRIPTOR(descriptors) \
    int i = 0; \
    for (Descriptor descriptor = descriptors->arguments[i]; i < descriptors->arguments_count; ++i) \

#define DESCRIPTORS_GET_RETURN_TYPE(descriptors) \
    descriptors->return_descriptor.type

#endif