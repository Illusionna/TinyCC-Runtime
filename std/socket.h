#ifndef _SOCKET_H_
#define _SOCKET_H_


#if !defined(__OS_WINDOWS__) && !defined(__OS_UNIX__)
    #if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
        #define __OS_WINDOWS__
    #elif defined(__linux__) || defined(__APPLE__)
        #define __OS_UNIX__
    #else
        #error "Unsupported platforms."
    #endif
#endif


#if defined(__OS_WINDOWS__)
    #include <winsock2.h>
#elif defined(__OS_UNIX__)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif


#if defined(__OS_WINDOWS__)
    typedef SOCKET Socket;
    #define SOCKET_INVALID INVALID_SOCKET
#elif defined(__OS_UNIX__)
    typedef int Socket;
    #define SOCKET_INVALID -1
#endif


/**
 * @brief Initialize a socket.
 * @return `1` for failure.
**/
int socket_init();


/**
 * @brief Create a socket.
 * @param domain The communication domain (e.g. `AF_INET` for IPv4).
 * @param type The socket type (e.g. `SOCK_STREAM` for TCP).
 * @param protocol The protocol to be used (e.g. `0` for default).
 * @return A socket descriptor (e.g. `SOCKET_INVALID` for failure).
**/
Socket socket_create(int domain, int type, int protocol);


/**
 * @brief Close a socket.
 * @param s The socket descriptor.
**/
void socket_close(Socket s);


/**
 * @brief Destroy the socket.
**/
void socket_destroy();


/**
 * @brief Configure a socket address structure.
 * @param server Pointer to the `sockaddr_in` structure.
 * @param domain The communication domain (e.g. `AF_INET` for IPv4).
 * @param ip The IP address (e.g. `"127.0.0.1"`).
 * @param port The host port (e.g. `8080`).
**/
void socket_config(struct sockaddr_in *server, int domain, char *ip, int port);


/**
 * @brief Connect to a server using socket.
 * @param s The socket descriptor.
 * @param server Pointer to the `sockaddr_in` structure.
 * @param size The `sizeof(server)` of the `sockaddr_in` structure.
 * @return `SOCKET_INVALID` for failure.
**/
int socket_connect(Socket s, struct sockaddr_in *server, int size);


/**
 * @brief Send data through socket TCP tunnel.
 * @param s The socket descriptor.
 * @param buffer The data buffer.
 * @param size The size of data buffer.
 * @param flag The flag for sending data (e.g. `0` for default).
 * @return `SOCKET_INVALID` for failure.
**/
int socket_send(Socket s, char *buffer, int size, int flag);


/**
 * @brief Receive the response from the server with blocking.
 * @param s The socket descriptor.
 * @param buffer The response data buffer.
 * @param size The size of response data buffer.
 * @param flag The flag for receiving data (e.g. `0` for default).
 * @return The index of the last character received.
**/
int socket_recv(Socket s, char *buffer, int size, int flag);


/**
 * @brief Bind socket to a certain address.
 * @param s The socket descriptor.
 * @param address_name Pointer to the `sockaddr_in` structure.
 * @param size The `sizeof(address_name)` of the `sockaddr_in` structure.
 * @return `SOCKET_INVALID` for failure.
**/
int socket_bind(Socket s, struct sockaddr_in *address_name, int size);


/**
 * @brief Listen the socket.
 * @param s The socket descriptor.
 * @param backlog The maximum queue length waiting for connection (e.g. `backlog = 5`, five connection requests will be queued).
 * @return `SOCKET_INVALID` for failure.
**/
int socket_listen(Socket s, int backlog);


/**
 * @brief Accept the socket.
 * @param s The socket descriptor.
 * @param address Pointer to the `sockaddr_in` structure.
 * @param size_pointer The `sizeof(address)` of the `sockaddr_in` structure pointer.
 * @return A new socket object (`SOCKET_INVALID` for failure).
**/
Socket socket_accept(Socket s, struct sockaddr_in *address, int *size_pointer);


/**
 * @brief Set the option of socket.
 * @param s The socket descriptor.
 * @param level `SOL_SOCKET` for default.
 * @param optname `SO_REUSEADDR` for default (port reuse).
 * @return `SOCKET_INVALID` for failure.
**/
int socket_setopt(Socket s, int level, int optname);


#endif