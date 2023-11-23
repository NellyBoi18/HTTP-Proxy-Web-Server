#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <pthread.h>

// Type definition for queue items
typedef struct {
    int client_fd; // Client file descriptor
    char *path;    // Request path
    int priority;  // Priority based on path or other criteria
    int delay;     // Delay in seconds before processing the request
} queue_item_t;

// Type definition for the priority queue
typedef struct {
    queue_item_t *items;            // Dynamic array of queue items
    int size;                       // Current number of items in the queue
    int capacity;                   // Maximum capacity of the queue
    pthread_mutex_t lock;           // Mutex for thread synchronization
    pthread_cond_t not_empty_cond;  // Condition variable for blocking get
} priority_queue_t;

// Function declarations
priority_queue_t *create_queue(int capacity);
int add_work(priority_queue_t *queue, queue_item_t item);
queue_item_t get_work(priority_queue_t *queue, int blocking);
void handle_getjob_request(int client_fd, priority_queue_t *queue);

#endif // SAFEQUEUE_H
