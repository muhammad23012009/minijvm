#ifndef GC_H
#define GC_H

#include "method.h"

typedef struct Object Object;

/* A rather crude implementation of a garbage collector */
struct list_head {
    void *value;
    struct list_head *next;
    struct list_head *prev;
};

typedef struct GarbageCollector {
    struct list_head *tracked_frames;
    struct list_head *tracked_objects;
    struct list_head *marked_objects;
} GarbageCollector;

extern void gc_create();
extern void gc_track_frame(Frame *frame);
extern void gc_untrack_frame(Frame *frame);
extern void gc_track_object(Object *object);
extern void gc_collect();
extern void gc_free();

#endif