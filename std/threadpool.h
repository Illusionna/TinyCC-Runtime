#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_


#include <string.h>


#include "thread.h"


typedef struct ThreadTask {
	void (*func)(void *args);
	void *args;
    void (*cleanup)(void *args);
    struct ThreadTask *next;
} ThreadTask;


typedef struct {
    int shutdown;   // `0` for running, `1` for shutdown.
    int n_workers;
    int queue_length;
    int max_queue_length;
    Thread *threads;
    Mutex queue_lock;
    ThreadCondition notify;     // Used to wake up the sleeping threads.
    ThreadCondition not_full;
    ThreadTask *queue_head;
    ThreadTask *queue_tail;
} ThreadPool;


/**
 * @brief Create a thread pool.
 * @param n_workers The number of threads.
 * @param max_queue_length The maximum length of thread queue.
 * @return `NULL` for failure.
**/
ThreadPool *threadpool_create(int n_workers, int max_queue_length);


/**
 * @brief Add a task to the thread pool.
 * @param func The pointer of task function like `void func(void *args)`.
 * @param args The arguments of task function.
 * @param block `1` for blocking and waiting when the queue is full, `0` for returning an error immediately.
 * @param cleanup The thread is responsible for releasing the memory like `void cleanup(void *args)` and `NULL` for no cleaning.
 * @return `0` for success, `1` for failure, `2` for full queue.
**/
int threadpool_add(ThreadPool *pool, void (*func)(void *args), void *args, int block, void (*cleanup)(void *args));


/**
 * @brief Destroy the thread pool and release the memory.
 * @param pool The pointer of thread pool.
 * @param safe_exit `1` for awaiting all tasks to be completed, `0` for exiting immediately.
 * @return `0` for success, `1` for failure.
**/
int threadpool_destroy(ThreadPool *pool, int safe_exit);


/**
 * @brief The worker function of thread pool.
 * @param args The arguments of task function.
 * @return `0` for success.
**/
int __threadpool_worker__(void *args);


#endif
