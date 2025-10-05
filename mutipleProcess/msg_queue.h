#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <pthread.h>
#include "ipc_common.h"

#define QUEUE_LEN 16

typedef struct {
    message_t buffer[QUEUE_LEN];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} msg_queue_t;

void init_queue(msg_queue_t *q) {
    q->head = q->tail = q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void push_queue(msg_queue_t *q, message_t *msg) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == QUEUE_LEN)
        pthread_cond_wait(&q->cond, &q->mutex);
    q->buffer[q->tail] = *msg;
    q->tail = (q->tail + 1) % QUEUE_LEN;
    q->count++;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

int pop_queue(msg_queue_t *q, message_t *msg) {
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0)
        pthread_cond_wait(&q->cond, &q->mutex);
    *msg = q->buffer[q->head];
    q->head = (q->head + 1) % QUEUE_LEN;
    q->count--;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
    return 1;
}

#endif

