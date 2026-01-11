#include "threadpool.h"


ThreadPool *threadpool_create(int n_workers, int max_queue_length) {
    if (n_workers <= 0 || max_queue_length <= 0) return NULL;
    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (pool == NULL) return NULL;
    memset(pool, 0, sizeof(ThreadPool));

    pool->n_workers = n_workers;
    pool->max_queue_length = max_queue_length;
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

    if (condition_init(&pool->not_full) != 0) {
        condition_destroy(&pool->notify);
        mutex_destroy(&pool->queue_lock);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < n_workers; i++) {
        if (thread_create(&(pool->threads[i]), __threadpool_worker__, (void *)pool) != 0) {
            pool->n_workers = i;
            pool->shutdown = 1;
            condition_broadcast(&pool->notify);

            for (int j = 0; j < i; j++) thread_join(&(pool->threads[j]), NULL);

            condition_destroy(&pool->not_full);
            condition_destroy(&pool->notify);
            mutex_destroy(&pool->queue_lock);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }
    return pool;
}


int threadpool_add(ThreadPool *pool, void (*func)(void *args), void *args, int block, void (*cleanup)(void *args)) {
    if (pool == NULL || func == NULL) return 1;
    ThreadTask *task = (ThreadTask *)malloc(sizeof(ThreadTask));
    if (task == NULL) return 1;

    task->func = func;
    task->args = args;
    task->cleanup = cleanup;
    task->next = NULL;

    mutex_lock(&pool->queue_lock);

    if (pool->shutdown) {
        mutex_unlock(&pool->queue_lock);
        free(task);
        return 1;
    }

    if (pool->max_queue_length > 0 && pool->queue_length >= pool->max_queue_length) {
        if (block) {
            while (pool->queue_length >= pool->max_queue_length && !pool->shutdown) condition_wait(&pool->not_full, &pool->queue_lock);

            if (pool->shutdown) {
                mutex_unlock(&pool->queue_lock);
                free(task);
                return 1;
            }
        } else {
            mutex_unlock(&pool->queue_lock);
            free(task);
            return 2;
        }
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


int threadpool_destroy(ThreadPool *pool, int safe_exit) {
    if (pool == NULL) return 1;

    mutex_lock(&pool->queue_lock);
    if (pool->shutdown) {
        mutex_unlock(&pool->queue_lock);
        return 1;
    }
    pool->shutdown = 1;

    if (!safe_exit) {
        ThreadTask *task = pool->queue_head;
        while (task != NULL) {
            ThreadTask *next = task->next;
            if (task->cleanup) task->cleanup(task->args);
            free(task);
            task = next;
        }
        pool->queue_head = NULL;
        pool->queue_tail = NULL;
        pool->queue_length = 0;
    }

    condition_broadcast(&pool->notify);
    condition_broadcast(&pool->not_full);
    mutex_unlock(&pool->queue_lock);

    for (int i = 0; i < pool->n_workers; i++) thread_join(&(pool->threads[i]), NULL);
    free(pool->threads);

    // Conservative style, attempting again to release memory.
    ThreadTask *task = pool->queue_head;
    while (task != NULL) {
        ThreadTask *next = task->next;
        if (task->cleanup) task->cleanup(task->args);
        free(task);
        task = next;
    }

    mutex_destroy(&pool->queue_lock);
    condition_destroy(&pool->notify);
    condition_destroy(&pool->not_full);
    free(pool);
    return 0;
}


int __threadpool_worker__(void *args) {
    ThreadPool *pool = (ThreadPool *)args;

    while (1) {
        mutex_lock(&pool->queue_lock);

        while (pool->queue_length == 0 && !pool->shutdown) condition_wait(&pool->notify, &pool->queue_lock);

        if (pool->shutdown && pool->queue_length == 0) {
            mutex_unlock(&pool->queue_lock);
            thread_exit();
        }

        ThreadTask *task = pool->queue_head;
        if (task) {
            pool->queue_head = task->next;
            pool->queue_length--;
            if (pool->queue_length == 0) pool->queue_tail = NULL;
            condition_signal(&pool->not_full);
        }

        mutex_unlock(&pool->queue_lock);

        if (task) {
            if (task->func) (*(task->func))(task->args);
            if (task->cleanup) (*(task->cleanup))(task->args);
            free(task);
        }
    }
    return 0;
}
