/* Maciej Andrejczuk */

#ifndef _QUEUE_H
#define _QUEUE_H

/* Queue stores elements of any type (void*) */
typedef void* element_t;

struct QueueRecord;
typedef struct QueueRecord* Queue;

/* Initialization of a queue.
   Should be done before use of a queue.
   Returns 0 if everything is OK. */
void queue_init(Queue*);

/* Cleans up. Should be done after use of a queue */
void queue_finalize(Queue);

/* Inserts a value into queue. */
void queue_push(const Queue, element_t);

/* Removes last element of a queue */
void queue_pop(const Queue);

/* Returns first element of a queue. */
element_t queue_front(const Queue);

/* Returns non zero value if queue is empty, zero otherwise. */
int queue_is_empty(const Queue);

#endif
