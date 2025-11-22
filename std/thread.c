#include "thread.h"


#if defined(__OS_UNIX__)
    void *__thread_wrapper__(void *args) {
        _ThreadInformation *info = (_ThreadInformation *)args;
        _ThreadFunction f = info->ptr;
        void *x = info->args;
        free(info);
        void *result = malloc(sizeof(int));
        if (result != NULL) *(int *)result = f(x);
        return result;
    }
#elif defined(__OS_WINDOWS__)
    unsigned WINAPI __thread_wrapper__(void *args) {
        _ThreadInformation *info = (_ThreadInformation *)args;
        _ThreadFunction f = info->ptr;
        void *x = info->args;
        free(info);
        return f(x);
    }
#endif


int thread_create(Thread *thread, _ThreadFunction func, void *args) {
    _ThreadInformation *info = malloc(sizeof(_ThreadInformation));
    if (info == NULL) return 1;

    info->ptr = func;
    info->args = args;

    #if defined(__OS_UNIX__)
        if (pthread_create(thread, NULL, __thread_wrapper__, (void *)info) != 0) *thread = 0;
    #elif defined(__OS_WINDOWS__)
        *thread = (HANDLE)_beginthreadex(NULL, 0, __thread_wrapper__, (void *)info, 0, NULL);
    #endif

    if (!*thread) {
        free(info);
        return 1;
    }
    return 0;
}


int thread_join(Thread *thread, int *result) {
    #if defined(__OS_UNIX__)
        void *u;
        int ans = 0;
        if (pthread_join(*thread, &u) != 0) return 1;
        if (u != NULL) {
            ans = *(int *)u;
            free(u);
        }
        if (result != NULL) *result = ans;
    #elif defined(__OS_WINDOWS__)
        if (WaitForSingleObject(*thread, 0xffffffff) == (DWORD)0xffffffff) return 1;
        if (result != NULL) {
            DWORD d;
            GetExitCodeThread(*thread, &d);
            *result = d;
        }
    #endif
    return 0;
}


int thread_detach(Thread *thread) {
    #if defined(__OS_UNIX__)
        return (pthread_detach(*thread) == 0) ? 0 : 1;
    #elif defined(__OS_WINDOWS__)
        return (CloseHandle(*thread) != 0) ? 0 : 1;
    #endif
}


void thread_exit() {
    #if defined(__OS_UNIX__)
        pthread_exit(NULL);
    #elif defined(__OS_WINDOWS__)
        _endthreadex(0);
    #endif
}


int mutex_create(Mutex *mutex, int type) {
    #if defined(__OS_UNIX__)
        pthread_mutexattr_t t;
        pthread_mutexattr_init(&t);
        if (type & 8) pthread_mutexattr_settype(&t, PTHREAD_MUTEX_RECURSIVE);
        int res = pthread_mutex_init(mutex, &t);
        pthread_mutexattr_destroy(&t);
        return (res == 0) ? 0 : 1;
    #elif defined(__OS_WINDOWS__)
        mutex->status = 0;
        mutex->recursive = type & 8;
        InitializeCriticalSection(&mutex->cs);
        return 0;
    #endif
}


void mutex_destroy(Mutex *mutex) {
    #if defined(__OS_UNIX__)
        pthread_mutex_destroy(mutex);
    #elif defined(__OS_WINDOWS__)
        DeleteCriticalSection(&mutex->cs);
    #endif
}


int mutex_lock(Mutex *mutex) {
    #if defined(__OS_UNIX__)
        return (pthread_mutex_lock(mutex) == 0) ? 0 : 1;
    #elif defined(__OS_WINDOWS__)
        EnterCriticalSection(&mutex->cs);
        if (!mutex->recursive) {
            while (mutex->status) Sleep(1000);  // Simulate deadlock.
            mutex->status = 1;
        }
        return 0;
    #endif
}


int mutex_trylock(Mutex *mutex) {
    #if defined(__OS_UNIX__)
        return (pthread_mutex_trylock(mutex) == 0) ? 0 : 1;
    #elif defined(__OS_WINDOWS__)
        int res = TryEnterCriticalSection(&mutex->cs) ? 0 : 1;
        if ((!mutex->recursive) && (res == 1) && mutex->status) {
            LeaveCriticalSection(&mutex->cs);
            res = 1;
        }
        return res;
    #endif
}


int mutex_unlock(Mutex *mutex) {
    #if defined(__OS_UNIX__)
        return (pthread_mutex_unlock(mutex) == 0) ? 0 : 1;
    #elif defined(__OS_WINDOWS__)
        mutex->status = 0;
        LeaveCriticalSection(&mutex->cs);
        return 0;
    #endif
}