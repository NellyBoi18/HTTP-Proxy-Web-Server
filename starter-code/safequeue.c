#include "safequeue.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    // Structure to hold request data
    int client_fd; // Client file descriptor
    char *path;    // Request path
    int priority;  // Priority based on path or other criteria
} queue_item_t;

typedef struct {
    queue_item_t *items;            // Dynamic array of queue items
    int size;                       // Current number of items in the queue
    int capacity;                   // Maximum capacity of the queue
    pthread_mutex_t lock;           // Mutex for thread synchronization
    pthread_cond_t not_empty_cond;  // Condition variable for blocking get
} priority_queue_t;

priority_queue_t *create_queue(int capacity) {
    priority_queue_t *queue = malloc(sizeof(priority_queue_t));
    if (!queue) return NULL;

    queue->items = malloc(sizeof(queue_item_t) * capacity);
    if (!queue->items) {
        free(queue);
        return NULL;
    }

    queue->size = 0;
    queue->capacity = capacity;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->not_empty_cond, NULL);
    return queue;
}

// Helper function to swap two items in the queue
void swap(queue_item_t *a, queue_item_t *b) {
    queue_item_t temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up for maintaining the heap property after insertion
void heapify_up(priority_queue_t *queue, int index) {
    int parent_index = (index - 1) / 2;
    if (index && queue->items[parent_index].priority < queue->items[index].priority) {
        swap(&queue->items[index], &queue->items[parent_index]);
        heapify_up(queue, parent_index);
    }
}

// Heapify down for maintaining the heap property after removal
void heapify_down(priority_queue_t *queue, int index) {
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int largest = index;

    if (left < queue->size && queue->items[left].priority > queue->items[largest].priority) {
        largest = left;
    }
    if (right < queue->size && queue->items[right].priority > queue->items[largest].priority) {
        largest = right;
    }

    if (largest != index) {
        swap(&queue->items[index], &queue->items[largest]);
        heapify_down(queue, largest);
    }
}

int add_work(priority_queue_t *queue, queue_item_t item) {
    pthread_mutex_lock(&queue->lock);

    if (queue->size >= queue->capacity) {
        pthread_mutex_unlock(&queue->lock);
        return -1; // Queue is full
    }

    // Add item to the end of the queue and heapify up
    queue->items[queue->size] = item;
    heapify_up(queue, queue->size);
    queue->size++;

    pthread_cond_signal(&queue->not_empty_cond); // Signal any waiting threads
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

queue_item_t get_work(priority_queue_t *queue, int blocking) {
    pthread_mutex_lock(&queue->lock);

    while (blocking && queue->size == 0) {
        pthread_cond_wait(&queue->not_empty_cond, &queue->lock);
    }

    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->lock);
        return (queue_item_t){0}; // Return an empty item if the queue is empty
    }

    // Remove the highest priority item (root of the heap)
    queue_item_t item = queue->items[0];
    queue->items[0] = queue->items[queue->size - 1];
    queue->size--;
    heapify_down(queue, 0);

    pthread_mutex_unlock(&queue->lock);
    return item;
}