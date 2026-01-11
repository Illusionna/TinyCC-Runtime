#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_


#include <string.h>
#include "thread.h"


typedef struct ThreadTask {
	void (*func)(void *args);
	void *args;
    struct ThreadTask *next;
} ThreadTask;


typedef struct {
    int shutdown;   // `0` for running, `1` for shutdown.
    int n_workers;
    int queue_length;
    Thread *threads;
    Mutex queue_lock;
    ThreadCondition notify;     // Used to wake up the sleeping threads.
    ThreadTask *queue_head;
    ThreadTask *queue_tail;
} ThreadPool;


/**
 * @brief Create a thread pool.
**/
ThreadPool *threadpool_create(int n_workers);


/**
 * @brief Destroy the thread pool and release the memory.
**/
int threadpool_destroy(ThreadPool *pool);


/**
 * @brief Add the a to the thread pool.
 * @param func The pointer of thread function. `void func(void *args);`
**/
int threadpool_add(ThreadPool *pool, void (*func)(void *args), void *args);


/**
 * @brief The worker function of thread pool.
**/
int threadpool_worker(void *args);


#endif