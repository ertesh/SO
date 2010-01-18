#include <stdlib.h>
#include "queue.h"
#include "err.h"

struct QueueElement
{
    element_t el;
    struct QueueElement* next;
};

struct QueueRecord
{
    struct QueueElement* first;
    struct QueueElement* last;
};

void queue_init(Queue* q)
{
    *q = (Queue) malloc(sizeof(struct QueueRecord));
    if (*q == NULL) {
        syserr("malloc\n");
    }
    (*q)->first = NULL;
    (*q)->last = NULL;
}

void queue_finalize(Queue q)
{
    free(q);
}

void queue_push(const Queue q, element_t el)
{
    if (q == NULL) fatal("Uninitialized queue.\n");
    struct QueueElement* qe;
    qe = (struct QueueElement*) malloc(sizeof(struct QueueElement));
    if (qe == NULL) syserr("malloc\n");
    qe->el = el;
    qe->next = NULL;
    if (q->first == NULL) {
        q->first = qe;
    }
    if (q->last != NULL) {
        q->last->next = qe;
    }
    q->last = qe;
}

void queue_pop(const Queue q)
{
    if (q == NULL) fatal("Uninitialized queue.\n");
    struct QueueElement* qe = q->first;
    if (qe == NULL) fatal("Empty queue.");
    q->first = qe->next;
    free(qe);
}

element_t queue_front(const Queue q) {
    return q->first->el;
}

int queue_is_empty(const Queue q) {
    return (q->first == NULL);
}

