#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <pthread.h>

typedef struct Frame Frame;

typedef enum {
    THREAD_STATE_RUNNING,
    THREAD_STATE_WAITING,
    THREAD_STATE_TERMINATED
} ThreadState;

typedef struct ThreadInfo {
    // Our ID. Only used for internal tracking and for matching threads in java/lang/Thread
    uint64_t id;
    // We still need to interact with pthread, so store the pthread_t too
    pthread_t native_thread;

    ThreadState state;

    // The current frame executing in the thread
    Frame *current_frame;

    // The next thread
    struct ThreadInfo *next;
} ThreadInfo;

typedef struct ThreadList {
    ThreadInfo *head;
    ThreadInfo *removal_head;
    pthread_mutex_t mutex;
    uint64_t next_id;

    // For main to wait on until all threads are dead.
    pthread_cond_t cond;
} ThreadList;

// Returns an owned pointer to the ThreadInfo. Caller is not responsible for freeing it.
extern ThreadInfo *thread_list_add(pthread_t native_thread);
extern ThreadInfo *thread_list_get(uint64_t id);
extern ThreadInfo *thread_list_lookup(pthread_t native_thread);

extern void thread_list_remove(uint64_t id);

extern void thread_list_wait();
extern void thread_list_init();

#endif // THREAD_H