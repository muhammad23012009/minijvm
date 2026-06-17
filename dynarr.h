#ifndef DYNARR_H
#define DYNARR_H

#include <stddef.h>
#include <stdlib.h>

struct ArrayMetadata {
    size_t capacity;
    size_t length;
};

#define arr_init(type)                                                           \
    (type*)((struct ArrayMetadata*)calloc(1, sizeof(struct ArrayMetadata)) + 1)  \

static void *arr_init_with_capacity(size_t type_size, size_t capacity)
{
    struct ArrayMetadata *metadata = malloc(sizeof(struct ArrayMetadata) + type_size * capacity);
    metadata->capacity = capacity;
    metadata->length = 0;
    return (void*)(metadata + 1);
}

#define arr_push(arr, value) \
    do { \
        struct ArrayMetadata *metadata = (struct ArrayMetadata*)(arr) - 1;  \
        if (metadata->length >= metadata->capacity) {                       \
            if (metadata->capacity == 0)                                    \
                metadata->capacity = 4;                                     \
            else                                                            \
                metadata->capacity *= 2;                                    \
            metadata = (struct ArrayMetadata*)realloc(metadata, sizeof(struct ArrayMetadata) + sizeof(value) * metadata->capacity); \
            (arr) = (typeof(arr))(metadata + 1);                                 \
        }                                                                   \
        (arr)[metadata->length++] = value;                                  \
    } while (0);                                                            \

#define arr_length(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->length : 0)

#define arr_capacity(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->capacity : 0)

#define arr_free(arr) \
    do { \
        if (arr) { \
            struct ArrayMetadata *metadata = (struct ArrayMetadata*)(arr) - 1;  \
            free(metadata); \
            (arr) = NULL; \
        } \
    } while (0);

#endif