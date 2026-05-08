#include "gc.h"
#include "variant.h"

static GarbageCollector *gc = NULL;

/* TODO:
 * Add a custom allocator so we can detect allocated objects more efficiently
 * Match static objects with their classes, so we can free them when the class is unloaded
 * Add support for custom heap size and object thresholds
 * Implement proper mark-and-sweep algorithm
 * Implement a hashmap to track objects and frames for faster lookups
 * Implement generations in objects for garbage collection (older generations collected less frequently)
 * Implement an efficient algorithm to mark objects reachable from frames (like setting bits in a bitmap or pointer of the object)
 * Add support for running the GC at specific intervals or memory thresholds
 * Add support for running the GC when objects are created/destroyed
 * Optimize the tracking and searches of frames and objects to reduce overhead
 */

static void prepend_list_node(struct list_head **head, void *value)
{
    struct list_head *new_node = malloc(sizeof(struct list_head));
    new_node->value = value;
    new_node->next = *head;
    new_node->prev = NULL;

    if (*head) {
        (*head)->prev = new_node;
    }
    *head = new_node;
}

static struct list_head *find_list_node(struct list_head *head, void *value)
{
    struct list_head *current = head;
    while (current) {
        if (current->value == value) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Caller is responsible for freeing the node later */
static void steal_list_node(struct list_head **head, struct list_head *node)
{
    if (!node) return;

    /* The only item in the list */
    if (*head == node) {
        *head = node->next;
        return;
    }

    if (node->prev) {
        node->prev->next = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
}

static void foreach_list_node(struct list_head **head, void *user, void (*func)(struct list_head *node, struct list_head **head, void *))
{
    struct list_head *current = *head;

    while (current) {
        struct list_head *next = current->next;
        func(current, head, user);
        current = next;
    }
}

static bool find_object_in_frame(Frame *frame, Object *object)
{
    /* Search locals first */
    for (int i = 0; i < frame->max_locals; ++i) {
        Variant *local = &frame->locals[i];
        if (local->type == VARIANT_TYPE_OBJECT && local->data.object == object) {
            return true;
        }
    }

    /* Search stack next */
    for (int i = 0; i < frame->max_stack; ++i) {
        Variant *stack_item = &frame->stack->items[i];
        if (stack_item->type == VARIANT_TYPE_OBJECT && stack_item->data.object == object) {
            return true;
        }
    }

    return false;
}

static void free_object_in_frame(Frame *frame, Object *object)
{
    if (find_object_in_frame(frame, object)) {
        object_free(object);
    }
}

static void mark_object_in_frame(struct list_head *obj_node, struct list_head **head, void *gc_ptr)
{
    GarbageCollector *gc = (GarbageCollector *)gc_ptr;
    /* Iterate over all tracked frames and see if any of them reference this object */
    struct list_head *current_frame_node = gc->tracked_frames;

    while (current_frame_node) {
        Frame *frame = (Frame *)current_frame_node->value;

        if (find_object_in_frame(frame, obj_node->value)) {
            //printf("Found object of class %s with pointer %p in frame %p, leaving it be\n", ((Object *)obj_node->value)->class->name, (void*)obj_node->value, (void*)frame);
            /* Mark this object */
            steal_list_node(head, obj_node);
            prepend_list_node(&gc->marked_objects, obj_node->value);
            free(obj_node);
            return;
        }
        current_frame_node = current_frame_node->next;
    }
}

static void free_unmarked_object(struct list_head *obj_node, struct list_head **head, void *user)
{
    Object *object = (Object *)obj_node->value;
    // TODO: get rid of this parent_field bullshit
    if (object->parent_field && object->parent_field->info.access_flags & 0x0008) {
        return;
    }

    //printf("Freeing unmarked object of class %s with pointer %p\n", object->class->name, (void*)object);
    steal_list_node(head, obj_node);
    object_free(object);
    free(obj_node);
}

static void free_all_unmarked_object(struct list_head *obj_node, struct list_head **head, void *user)
{
    Object *object = (Object *)obj_node->value;
    //printf("Freeing unmarked object of class %s with pointer %p\n", object->class->name, (void*)object);
    steal_list_node(head, obj_node);
    object_free(object);
    free(obj_node);
}

static void find_object_in_other_frames(struct list_head *obj_node, struct list_head **head, void *frame_ptr)
{
    Frame *excluded_frame = (Frame *)frame_ptr;
    Object *object = (Object *)obj_node->value;
    struct list_head *current_frame_node = gc->tracked_frames;
    bool referenced_elsewhere = false;

    while (current_frame_node) {
        Frame *frame = (Frame *)current_frame_node->value;
        if (frame != excluded_frame) {
            if (find_object_in_frame(frame, object)) {
                /* Found in another frame, do not free */
                return;
            }
        }
        current_frame_node = current_frame_node->next;
    }

    /* Not found in any other frame, safe to free */
    //printf("Did not find object of class %s with pointer %p in other frames, freeing it\n", object->class->name, (void*)object);
    steal_list_node(head, obj_node);
    object_free(object);
    free(obj_node);
}

void gc_track_frame(Frame *frame)
{
    prepend_list_node(&gc->tracked_frames, frame);
}

/* Ideally to make this more efficient, we would be the ones allocating these objects in our heap */
void gc_untrack_frame(Frame *frame)
{
    struct list_head *frame_node = find_list_node(gc->tracked_frames, frame);

    if (!frame_node)
        /* Uhhhh? */
        return;

    /* Steal this node from the list */
    steal_list_node(&gc->tracked_frames, frame_node);

    /* Iterate over locals and the stack, and check if other frames have a reference 
     * to the objects in this frame. If so, they get migrated over to the marked list. In
     * case the objects are marked, but the only frame with references to them is this one, they get freed.
     * At the end, if any objects are still unmarked, we free them as well.
     */

    foreach_list_node(&gc->tracked_objects, gc, mark_object_in_frame);

    /* Now check if any marked objects exist that are only referenced by this frame (maybe do this first?) */
    foreach_list_node(&gc->marked_objects, frame, find_object_in_other_frames);

    /* Now free unmarked objects */
    foreach_list_node(&gc->tracked_objects, NULL, free_unmarked_object);

    free(frame_node);
}

void gc_track_object(Object *object)
{
    //printf("Tracking object of class %s with pointer %p\n", object->class->name, (void*)object);
    prepend_list_node(&gc->tracked_objects, object);
}

void gc_collect()
{
    /* Ok, time to run our magic here! */
}

void gc_create()
{
    gc = malloc(sizeof(GarbageCollector));
    gc->tracked_frames = NULL;
    gc->tracked_objects = NULL;
    gc->marked_objects = NULL;
}

void gc_free()
{
    /* Free everything */
    printf("Freeing garbage collector\n");
    foreach_list_node(&gc->tracked_objects, NULL, free_all_unmarked_object);
    foreach_list_node(&gc->marked_objects, NULL, free_all_unmarked_object);

    free(gc);
}