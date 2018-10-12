#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "socklib.h"

#define HOSTLEN 256
#define BACKLOG 1

int make_server_socket_q(int, int);

int make_server_socket(int portnum) {
    return make_server_socket_q(portnum, BACKLOG);
}
int make_server_socket_q(int portnum, int backlog) {
    struct sockaddr_in saddr;
    struct hostent *hp;
    char hostname[HOSTLEN];
    int sock_id;

    /* 创建socket */
    sock_id = socket(PF_INET, SOCK_STREAM, 0);
    if(sock_id == -1) {
        return -1;
    }

    /* 找主机地址 */
    memset(&saddr, 0, sizeof(saddr));
    gethostname(hostname, HOSTLEN);
    hp = gethostbyname(hostname);

    memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
    saddr.sin_port = htons(portnum);
    saddr.sin_family = AF_INET;

    /* 将地址和socket绑定 */
    if(bind(sock_id, (struct sockaddr*) &saddr, sizeof(saddr)) != 0) {
        return -1;
    }

    /* 监听 */
    if(listen(sock_id, backlog) != 0) {
        return -1;
    }
    return sock_id;
}

int connect_to_server(char *host, int portnum) {
    int sock;
    struct sockaddr_in servadd;
    struct hostent *hp;

    /* 创建一个socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        return -1;
    }

    /* 获取主机地址 */
    memset(&servadd, 0, sizeof(servadd));
    hp = gethostbyname(host);
    if(hp == NULL) {
        return -1;
    }

    memcpy(&servadd.sin_addr, hp->h_addr, hp->h_length);
    servadd.sin_port = htons(portnum);
    servadd.sin_family = AF_INET;

    if(connect(sock, (struct sockaddr*) &servadd, sizeof(servadd)) != 0) {
        return -1;
    }
    return sock;
}
