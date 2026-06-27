#ifndef DYNARR_H
#define DYNARR_H

#include <stddef.h>
#include <stdlib.h>

typedef struct Class Class;

typedef enum {
    // Could be a native type
    ARRAY_TYPE_NONE = 0,
    ARRAY_TYPE_OBJECT,
    ARRAY_TYPE_INT,
    ARRAY_TYPE_CHAR,
    ARRAY_TYPE_BYTE,
    ARRAY_TYPE_FLOAT,
    ARRAY_TYPE_DOUBLE,
    ARRAY_TYPE_LONG
} ArrayType;

struct ArrayMetadata {
    size_t capacity;
    size_t length;
    size_t element_size;
    ArrayType type;
    Class *component_type;
};

static void *arr_init_with_capacity_and_type(size_t type_size, size_t capacity, ArrayType type, Class *component_type)
{
    struct ArrayMetadata *metadata = malloc(sizeof(struct ArrayMetadata) + type_size * capacity);
    memset(metadata, 0, sizeof(struct ArrayMetadata) + type_size * capacity);
    metadata->capacity = capacity;
    metadata->length = 0;
    metadata->element_size = type_size;
    metadata->type = type;
    metadata->component_type = component_type;
    return (void*)(metadata + 1);
}

static void* arr_init_with_capacity(size_t type_size, size_t capacity)
{
    return arr_init_with_capacity_and_type(type_size, capacity, ARRAY_TYPE_NONE, NULL);
}

static void* arr_init_with_type(size_t type_size, ArrayType type, Class *component_type)
{
    return arr_init_with_capacity_and_type(type_size, 4, type, component_type);
}

static void *arr_init(size_t type_size)
{
    return arr_init_with_capacity_and_type(type_size, 4, ARRAY_TYPE_NONE, NULL);
}

#define arr_resize(arr, size) \
    do { \
        struct ArrayMetadata *metadata = (struct ArrayMetadata*)realloc(metadata, sizeof(struct ArrayMetadata) + metadata->element_size * size); \
        (arr) = (typeof(arr))(metadata + 1);                                 \
    } while (0);                                                            \

#define arr_push(arr, value) \
    do { \
        struct ArrayMetadata *metadata = (struct ArrayMetadata*)(arr) - 1;  \
        if (metadata->length >= metadata->capacity) {                       \
            if (metadata->capacity == 0)                                    \
                metadata->capacity = 4;                                     \
            else                                                            \
                metadata->capacity *= 2;                                    \
            metadata = (struct ArrayMetadata*)realloc(metadata, sizeof(struct ArrayMetadata) + metadata->element_size * metadata->capacity); \
            (arr) = (typeof(arr))(metadata + 1);                                 \
        }                                                                   \
        (arr)[metadata->length++] = value;                                  \
    } while (0);                                                            \

// TODO: Make this move stuff around as well
#define arr_insert(arr, index, value) \
    do { \
        struct ArrayMetadata *metadata = (struct ArrayMetadata*)(arr) - 1;  \
        if (metadata->length >= metadata->capacity) {                       \
            if (metadata->capacity == 0)                                    \
                metadata->capacity = 4;                                     \
            else                                                            \
                metadata->capacity *= 2;                                    \
            metadata = (struct ArrayMetadata*)realloc(metadata, sizeof(struct ArrayMetadata) + metadata->element_size * metadata->capacity); \
            (arr) = (typeof(arr))(metadata + 1);                                 \
        }                                                                   \
        (arr)[index] = value;                                  \
        metadata->length++; \
    } while (0);                                                            \

#define arr_length(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->length : 0)

#define arr_capacity(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->capacity : 0)

#define arr_type(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->type : ARRAY_TYPE_NONE)

#define arr_component_type(arr) \
     ((arr) ? ((struct ArrayMetadata*)(arr) - 1)->component_type : NULL)

#define arr_free(arr) \
    do { \
        if (arr) { \
            struct ArrayMetadata *metadata = (struct ArrayMetadata*)(arr) - 1;  \
            free(metadata); \
            (arr) = NULL; \
        } \
    } while (0);

#endif /* DYNARR_H */