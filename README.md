# TinyCC-Runtime

<div align=center>
    <img src="./demo.png" width="75%" height="75%">
</div>

## 新特性

- 2025-04-28: 多线程 thread.h 头文件.
```c
// >>> tcc test-thread.c

# include <stdio.h>
# include <thread.h>
# include <windows.h>

int func(void *arg) {
    printf("thread is running...\n");
    Sleep(3000);
    printf("thread is done!\n");
    return 0;
}

int main(int argc, char *argv[], char *env[]) {
    _thread thread;
    if (create_thread(&thread, func, NULL) != thread_success) {
        printf("\x1b[31m[Error] failed to create thread.\x1b[0m\n");
        return 1;
    }
    Sleep(1000);
    printf("main is done!\n");
    return 0;
}
```