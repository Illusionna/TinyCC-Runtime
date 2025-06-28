# TinyCC-Runtime

<div align=center>
    <img src="./demo.png" width="75%" height="75%">
</div>

## 指令

```bash
tcc main.c -o main.exe      // 直接生成二进制可执行文件
tcc -run main.c             // 以脚本形式运行
tcc -c main.c               // 生成 main.o 目标链接文件

tcc -E main.c -o main.i     // 预处理
gcc -S main.i -o main.s     // 编译（tcc 不支持，使用 gcc）
gcc -c main.s -o main.o     // 汇编（虽然 tcc 支持，但没有 gcc 汇编的 .def 命令，继续使用 gcc）
gcc main.o -o main.exe      // 链接
```

## 修复

- 2025-04-28: `win32/include/stdlib.h` 和 `win32/include/errno.h` 头文件
```c
(x) _CRTIMP extern int *__cdecl _errno(void);   // tcc 预处理生成 main.i 语法错误.
(√) _CRTIMP int *__cdecl _errno(void);          // 不需要 extern 关键词修饰.
```

- 2025-04-29: `win32/include/math.h` 头文件
```c
/** (x)
 * tcc 不支持汇编 t 约束.
float res;
__asm__ ("fabs;" : "=t" (res) : "0" (x));
return res;
**/
(√) return x < 0 ? -x : x;    // 直接用更加兼容的表达式.
```


## 新特性

- 2025-04-28: 多线程 `thread.h` 头文件.
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

- 2025-06-28: 哈希表字典 `hashmap.h` 头文件.
```c
#include "hashmap.h"

int main(int argc, char *argv[], char *env[]) {
    HashMap *dict = hashmap_create();
    hashmap_put(dict, "1", "com");
    hashmap_put(dict, "11", "org");
    hashmap_put(dict, "111", "xyz");
    hashmap_put(dict, "2", "net");
    hashmap_put(dict, "33", "cn");
    hashmap_put(dict, "3", "edu");
    hashmap_print(dict);
    printf("%s\n", hashmap_get(dict, "111") ? hashmap_get(dict, "111") : "(null)");
    printf("%s\n", hashmap_get(dict, "1314") ? hashmap_get(dict, "111") : "(null)");
    printf("delete %s\n", hashmap_remove(dict, "11") == 0 ? "success" : "failed");
    hashmap_view(dict);
    hashmap_destroy(dict);
    return 0;
}
```
