# include "../include/thread.h"


# if defined(__OS_UNIX__)
    void *__create_thread_wrapper__(void *__arg__) {
        __thread_information *t = (__thread_information *) __arg__;

        _thread_function func = t->ptr;
        void *arg = t->args;

        free(t);                                    // 线程负责释放开辟的内存信息.

        void *res = malloc(sizeof(int));
        if (res != NULL) *(int *)res = func(arg);   // 转化实际执行线程的函数结果.
        return res;
    }
# elif defined(__OS_WINDOWS__)
    unsigned WINAPI __create_thread_wrapper__(void *__arg__) {
        __thread_information *t = (__thread_information *) __arg__;

        _thread_function func = t->ptr;
        void *arg = t->args;

        free(t);            // 线程负责释放开辟的内存信息.
        return func(arg);   // 返回实际执行线程的函数结果.
    }
# endif



int create_thread(_thread *thread, _thread_function func, void *args) {
    __thread_information *t = (__thread_information *)malloc(sizeof(__thread_information));
    if (t == NULL) {
        return thread_error;
    }
    t->ptr = func;
    t->args = args;

    # if defined(__OS_UNIX__)
        if (pthread_create(thread, NULL, __create_thread_wrapper__, (void *)t) != 0) *thread = 0;
    # elif defined(__OS_WINDOWS__)
        *thread = (HANDLE)_beginthreadex(NULL, 0, __create_thread_wrapper__, (void *)t, 0, NULL);
    # endif

    // 若操作系统创建线程失败, 闭包子函数则无法负责释放内存, 需要在此处释放.
    if (!*thread) {
        free(t);
        return thread_error;
    }
    return thread_success;
}



int join_thread(_thread *thread, int *res) {
    # if defined(__OS_UNIX__)
        void *u;
        int ans = 0;
        if (pthread_join(*thread, &u) != 0) return thread_error;
        if (u != NULL) {
            ans = *(int *)u;
            free(u);
        }
        if (res != NULL) *res = ans;
    # elif defined(__OS_WINDOWS__)
        if (WaitForSingleObject(*thread, 0xffffffff) == (DWORD)0xffffffff) return thread_error;
        if (res != NULL) {
            DWORD d;
            GetExitCodeThread(*thread, &d);
            *res = d;
        }
    # endif
    return thread_success;
}



void exit_thread() {
    # if defined(__OS_UNIX__)
        pthread_exit(NULL);
    # elif defined(__OS_WINDOWS__)
        _endthreadex(0);
    # endif
}


int create_mutex(_mutex *mutex, int type) {
    # if defined(__OS_UNIX__)
        pthread_mutexattr_t t;
        pthread_mutexattr_init(&t);
        if (type & mutex_recursive) pthread_mutexattr_settype(&t, PTHREAD_MUTEX_RECURSIVE);
        int res = pthread_mutex_init(mutex, &t);
        pthread_mutexattr_destroy(&t);
        return res == 0 ? thread_success : thread_error;
    # elif defined(__OS_WINDOWS__)
        mutex->status = 0;
        mutex->recursive = type & mutex_recursive;
        InitializeCriticalSection(&mutex->cs);
        return thread_success;
    # endif
}



void destroy_mutex(_mutex *mutex) {
    # if defined(__OS_UNIX__)
        pthread_mutex_destroy(mutex);
    # elif defined(__OS_WINDOWS__)
        DeleteCriticalSection(&mutex->cs);
    # endif
}



int mutex_lock(_mutex *mutex) {
    # if defined(__OS_UNIX__)
        return pthread_mutex_lock(mutex) == 0 ? thread_success : thread_error;
    # elif defined(__OS_WINDOWS__)
        EnterCriticalSection(&mutex->cs);
        if (!mutex->recursive) {
            while (mutex->status) Sleep(1000);  // 模拟死锁.
            mutex->status = 1;
        } else {
            // if (mutex->thread_id == GetCurrentThreadId()) {
            //     // 当前线程已经拥有锁, 增加递归计数器.
            //     mutex->recursive_count++;
            // } else {
            //     // 其他线程拥有锁, 等待. 临界区本身会处理等待.
            // }
        }
        return thread_success;
    # endif
}



int mutex_unlock(_mutex *mutex) {
    # if defined(__OS_UNIX__)
        return pthread_mutex_unlock(mutex) == 0 ? thread_success : thread_error;
    # elif defined(__OS_WINDOWS__)
        mutex->status = 0;
        LeaveCriticalSection(&mutex->cs);
        return thread_success;
    # endif
}



int mutex_trylock(_mutex *mutex) {
    # if defined(__OS_UNIX__)
        return (pthread_mutex_trylock(mutex) == 0) ? thread_success : thread_error;
    # elif defined(__OS_WINDOWS__)
        int res = TryEnterCriticalSection(&mutex->cs) ? thread_success : thread_error;
        if ((!mutex->recursive) && (res == thread_success) && mutex->status) {
            LeaveCriticalSection(&mutex->cs);
            res = thread_error;
        }
        return res;
    # endif
}