#include "threadpool.h"


int threadpool_worker(void *args) {
    ThreadPool *pool = (ThreadPool *)args;
    while (1) {
        mutex_lock(&pool->queue_lock);
        while (pool->queue_length == 0 && !pool->shutdown) condition_wait(&pool->notify, &pool->queue_lock);
        if (pool->shutdown && pool->queue_length == 0) {
            mutex_unlock(&pool->queue_lock);
            thread_exit();
        }
        ThreadTask *task = pool->queue_head;
        pool->queue_head = task->next;
        pool->queue_length--;
        if (pool->queue_length == 0) pool->queue_tail = NULL;
        mutex_unlock(&pool->queue_lock);
        if (task) {
            (*(task->func))(task->args);
            free(task);
        }
    }
    return 0;
}


ThreadPool *threadpool_create(int n_workers) {
    if (n_workers <= 0) return NULL;
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (pool == NULL) return NULL;
    memset(pool, 0, sizeof(ThreadPool));

    pool->n_workers = n_workers;
    pool->queue_length = 0;
    pool->shutdown = 0;
    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->threads = (Thread *)malloc(n_workers * sizeof(Thread));
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    if (mutex_create(&pool->queue_lock, 1) != 0) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (condition_init(&pool->notify) != 0) {
        mutex_destroy(&pool->queue_lock);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < n_workers; i++) {
        if (thread_create(&(pool->threads[i]), threadpool_worker, (void *)pool) != 0) {
            threadpool_destroy(pool); 
            return NULL;
        }
    }
    return pool;
}


int threadpool_add(ThreadPool *pool, void (*func)(void *args), void *args) {
    if (pool == NULL || func == NULL) return 1;
    ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
    if (task == NULL) return 1;

    task->func = func;
    task->args = args;
    task->next = NULL;

    mutex_lock(&pool->queue_lock);
    if (pool->shutdown) {
        mutex_unlock(&pool->queue_lock);
        free(task);
        return 1; 
    }

    if (pool->queue_length == 0) {
        pool->queue_head = task;
        pool->queue_tail = task;
    } else {
        pool->queue_tail->next = task;
        pool->queue_tail = task;
    }
    pool->queue_length++;

    condition_signal(&pool->notify);
    mutex_unlock(&pool->queue_lock);
    return 0;
}


int threadpool_destroy(ThreadPool *pool) {
    if (pool == NULL) return 1;

    mutex_lock(&pool->queue_lock);
    if (pool->shutdown) {
        mutex_unlock(&pool->queue_lock);
        return 1;
    }
    pool->shutdown = 1;

    condition_broadcast(&pool->notify);
    mutex_unlock(&pool->queue_lock);

    for (int i = 0; i < pool->n_workers; i++) thread_join(&(pool->threads[i]), NULL);
    free(pool->threads);
    
    ThreadTask *head = NULL;
    while (pool->queue_head != NULL) {
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free(head);
    }

    mutex_destroy(&pool->queue_lock);
    condition_destroy(&pool->notify);
    free(pool);
    return 0;
}