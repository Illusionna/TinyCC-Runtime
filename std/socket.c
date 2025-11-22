#include "socket.h"


int socket_init() {
    #ifdef __OS_WINDOWS__
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;
    #endif
    return 0;
}


Socket socket_create(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}


void socket_close(Socket s) {
    #if defined(__OS_WINDOWS__)
        if (s == SOCKET_INVALID) return;
        else closesocket(s);
    #elif defined(__OS_UNIX__)
        if (s == SOCKET_INVALID) return;
        else close(s);
    #endif
}


void socket_destroy() {
    #ifdef __OS_WINDOWS__
        WSACleanup();
    #endif
}


void socket_config(struct sockaddr_in *server, int domain, char *ip, int port) {
    #if defined(__OS_UNIX__)
        server->sin_family = domain;
        server->sin_addr.s_addr = inet_addr(ip);
        server->sin_port = htons(port);
    #elif defined(__OS_WINDOWS__)
        server->sin_family = domain;
        server->sin_addr.S_un.S_addr = inet_addr(ip);
        server->sin_port = htons(port);
    #endif
}


int socket_connect(Socket s, struct sockaddr_in *server, int size) {
    return connect(s, (struct sockaddr *)server, size);
}


int socket_send(Socket s, char *buffer, int size, int flag) {
    return send(s, buffer, size, flag);
}


int socket_recv(Socket s, char *buffer, int size, int flag) {
    return recv(s, buffer, size, flag);
}


int socket_bind(Socket s, struct sockaddr_in *address_name, int size) {
    return bind(s, (struct sockaddr *)address_name, size);
}


int socket_listen(Socket s, int backlog) {
    return listen(s, backlog);
}


Socket socket_accept(Socket s, struct sockaddr_in *address, int *size_pointer) {
    #if defined(__OS_UNIX__)
        return accept(s, (struct sockaddr *)address, (socklen_t *)size_pointer);
    #elif defined(__OS_WINDOWS__)
        return accept(s, (struct sockaddr *)address, size_pointer);
    #endif
}


int socket_setopt(Socket s, int level, int optname) {
    int opt = 1;
    #if defined(__OS_UNIX__)
        return setsockopt(s, level, optname, (void *)&opt, sizeof(opt));
    #elif defined(__OS_WINDOWS__)
        return setsockopt(s, level, optname, (char *)&opt, sizeof(opt));
    #endif
}