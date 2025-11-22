#include "os.h"


int os_getpid() {
    #if defined(__OS_WINDOWS__)
        return (int)GetCurrentProcessId();
    #elif defined(__OS_UNIX__)
        return (int)getpid();
    #endif
}


int os_access(char *path) {
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        return 1;
    } else {
        if (errno == ENOENT) return 0;
        return -1;
    }
}


char *os_readfile(char *path, int range_start, int range_end) {
    if (range_start == 0 && range_end == -1) {
        FILE *f = fopen(path, "r");
        if (!f) return NULL;

        fseek(f, 0, SEEK_END);
        long length = ftell(f);
        rewind(f);

        char *buffer = (char *)malloc(sizeof(char) * (length + 1));
        if (!buffer) {
            fclose(f);
            return NULL;
        }

        fread(buffer, 1, length, f);
        buffer[length] = '\0';
        fclose(f);
        return buffer;
    } else {
        FILE *f = fopen(path, "r");
        if (!f) return NULL;

        int length = range_end - range_start + 1;
        fseek(f, range_start, SEEK_SET);
        char *buffer = (char *)malloc(sizeof(char) * (length + 1));
        if (!buffer) {
            fclose(f);
            return NULL;
        }
        buffer[fread(buffer, 1, length, f)] = '\0';
        fclose(f);
        return buffer;
    }
}