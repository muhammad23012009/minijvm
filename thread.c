#include "thread.h"
#include <stdio.h>
#include <stdlib.h>

static ThreadList s_thread_list;

ThreadInfo *thread_list_add(pthread_t native_thread)
{
    ThreadInfo *info = malloc(sizeof(ThreadInfo));

    info->id = s_thread_list.next_id++;
    info->native_thread = native_thread;
    info->state = THREAD_STATE_RUNNING;
    info->current_frame = NULL;
    info->next = s_thread_list.head;

    pthread_mutex_lock(&s_thread_list.mutex);

    {
        s_thread_list.head = info;
    }

    pthread_mutex_unlock(&s_thread_list.mutex);

    return info;
}

ThreadInfo *thread_list_get(uint64_t id)
{
    pthread_mutex_lock(&s_thread_list.mutex);

    ThreadInfo *current = s_thread_list.head;
    while (current) {
        if (current->id == id) {
            pthread_mutex_unlock(&s_thread_list.mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&s_thread_list.mutex);
    return NULL;
}

ThreadInfo *thread_list_lookup(pthread_t native_thread)
{
    pthread_mutex_lock(&s_thread_list.mutex);

    ThreadInfo *current = s_thread_list.head;
    while (current) {
        if (pthread_equal(current->native_thread, native_thread)) {
            pthread_mutex_unlock(&s_thread_list.mutex);
            return current;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&s_thread_list.mutex);
    return NULL;
}

void thread_list_remove(uint64_t id)
{
    pthread_mutex_lock(&s_thread_list.mutex);

    ThreadInfo **current = &s_thread_list.head;
    while (*current) {
        if ((*current)->id == id) {
            (*current)->state = THREAD_STATE_TERMINATED;
            *current = (*current)->next;
            break;
        }
        current = &(*current)->next;
    }

    if (!s_thread_list.head) {
        // Only one thread, signal main to exit
        pthread_cond_signal(&s_thread_list.cond);
    }

    pthread_mutex_unlock(&s_thread_list.mutex);
}

void thread_list_wait()
{
    if (s_thread_list.head == NULL) {
        // No threads, return immediately
        return;
    }

    //printf("Head and tail are %p and %p\n", (void*)s_thread_list.head, (void*)s_thread_list.tail);

    pthread_cond_wait(&s_thread_list.cond, &s_thread_list.mutex);
}

void thread_list_init()
{
    s_thread_list.head = NULL;
    pthread_mutex_init(&s_thread_list.mutex, NULL);
    pthread_cond_init(&s_thread_list.cond, NULL);
    s_thread_list.next_id = 0;
}