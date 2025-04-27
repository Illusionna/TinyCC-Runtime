/**
 * C 语言线程库, 用于 Windows、Linux、macOS 操作系统简单的多线程编程.
 * 借鉴 TinyCThread 项目.
**/


# ifndef _THREAD_H_
# define _THREAD_H_


# if !defined(__OS_PLATFORM__)
    # if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
        # define __OS_WINDOWS__
    # else
        # define __OS_UNIX__
    # endif
    #define __OS_PLATFORM__
# endif


# if defined(__OS_UNIX__)
    # include <pthread.h>
# elif defined(__OS_WINDOWS__)
    # include <process.h>
    # include <windows.h>
# endif


# define thread_success 1
# define thread_error 0


# define mutex_try 1
# define mutex_plain 2
# define mutex_recursive 3


# if defined(__OS_UNIX__)
    typedef pthread_t _thread;
# elif defined(__OS_WINDOWS__)
    typedef HANDLE _thread;
# endif


# if defined(__OS_UNIX__)
    typedef pthread_t _thread;
# elif defined(__OS_WINDOWS__)
    typedef HANDLE _thread;
# endif


typedef int (*_thread_function)(void *);


typedef struct {
    _thread_function ptr;       // 执行线程的函数的指针.
    void *args;                 // 执行线程的函数的参数.
} __thread_information;


# if defined(__OS_UNIX__)
    typedef pthread_mutex_t _mutex;
# elif defined(__OS_WINDOWS__)
    typedef struct{
        CRITICAL_SECTION cs;    // 临界区.
        int status;             // 互斥锁的状态, 用 1 代表已锁, 0 代表未上锁.
        int recursive;          // 互斥锁特例递归锁, 1 代表是, 即同一线程多次加锁 (会死锁), 0 代表否.
    } _mutex;
# endif


// 创建线程.
int create_thread(_thread *thread, _thread_function func, void *args);

// 等待线程.
int join_thread(_thread *thread, int *res);

// 退出线程.
void exit_thread();

// 创建互斥锁.
int create_mutex(_mutex *mutex, int type);

// 销毁互斥锁.
void destroy_mutex(_mutex *mutex);

// 上锁.
int mutex_lock(_mutex *mutex);

// 解开.
int mutex_unlock(_mutex *mutex);

// 尝试.
int mutex_trylock(_mutex *mutex);


# endif