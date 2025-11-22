#ifndef _OS_H_
#define _OS_H_


#if !defined(__OS_WINDOWS__) && !defined(__OS_UNIX__)
    #if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
        #define __OS_WINDOWS__
    #elif defined(__linux__) || defined(__APPLE__)
        #define __OS_UNIX__
        #define _GNU_SOURCE
    #else
        #error "Unsupported platforms."
    #endif
#endif


#if defined(__OS_WINDOWS__)
    #include <windows.h>
#elif defined(__OS_UNIX__)
    #include <unistd.h>
#endif


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


/**
 * @brief Get process ID.
 * @return PID.
**/
int os_getpid();


/**
 * @brief Judge whether the file exists.
 * @param path The pointer of file path string.
 * @return `1` for existence, `0` for nonentity, `-1` for other errors (such as permission denied, etc.)
**/
int os_access(char *path);


/**
 * @brief Read file and convert it to string (note: the memory allocated must call free).
 * @param path The pointer of file path string.
 * @param range_start The start of range.
 * @param range_end The end of range.
 * @return The string content of file.
**/
char *os_readfile(char *path, int range_start, int range_end);


#endif