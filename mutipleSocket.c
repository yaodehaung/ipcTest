
/*
gcc mutipleSocket.c -o mutipleSocket

*/
#define _GNU_SOURCE  // 必須放最前面，啟用 GNU 擴展
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define TCP_PORT 12345
#define UDP_PORT 12346
#define BUF_SIZE 256

typedef struct {
    int fd;
    struct sockaddr_in addr;
    int is_udp; // 1 = UDP, 0 = TCP
} client_info_t;

// -------------------- server thread --------------------
void* server_thread(void* arg) {
    client_info_t *info = (client_info_t*)arg;
    char buf[BUF_SIZE];

    // 設定 thread 名稱為 fd
    char thread_name[16];
    snprintf(thread_name, sizeof(thread_name), "fd_%d", info->fd);
    pthread_setname_np(pthread_self(), thread_name);

    if (info->is_udp) {
        socklen_t addrlen = sizeof(info->addr);
        int n = recvfrom(info->fd, buf, BUF_SIZE-1, 0,
                         (struct sockaddr*)&info->addr, &addrlen);
        if (n > 0) {
            buf[n] = 0;
            printf("[UDP Thread %s] from %s:%d -> %s\n",
                   thread_name,
                   inet_ntoa(info->addr.sin_addr), ntohs(info->addr.sin_port), buf);
            sendto(info->fd, buf, n, 0, (struct sockaddr*)&info->addr, addrlen);
        }
        free(info);
        return NULL;
    } else {
        printf("[Thread %s] TCP Client connected\n", thread_name);
        while (1) {
            int n = recv(info->fd, buf, BUF_SIZE-1, 0);
            if (n <= 0) {
                printf("[Thread %s] TCP Client disconnected\n", thread_name);
                close(info->fd);
                break;
            }
            buf[n] = 0;
            printf("[Thread %s] received: %s\n", thread_name, buf);
            send(info->fd, buf, n, 0); // echo back
        }
        free(info);
        return NULL;
    }
}

int main() {
    int tcp_fd, udp_fd;

    // -------------------- TCP --------------------
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) { perror("TCP socket"); exit(1); }

    struct sockaddr_in tcp_addr = {0};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(TCP_PORT);
    tcp_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(tcp_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) < 0) {
        perror("TCP bind"); exit(1);
    }
    if (listen(tcp_fd, 10) < 0) { perror("TCP listen"); exit(1); }

    // -------------------- UDP --------------------
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) { perror("UDP socket"); exit(1); }

    struct sockaddr_in udp_addr = {0};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_fd, (struct sockaddr*)&udp_addr, sizeof(udp_addr)) < 0) {
        perror("UDP bind"); exit(1);
    }

    fd_set readfds;
    int max_fd = udp_fd > tcp_fd ? udp_fd : tcp_fd;

    printf("Monitoring TCP:%d UDP:%d ...\n", TCP_PORT, UDP_PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcp_fd, &readfds);
        FD_SET(udp_fd, &readfds);

        int ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) { perror("select"); break; }

        // ---- TCP 新連線 ----
        if (FD_ISSET(tcp_fd, &readfds)) {
            client_info_t *info = malloc(sizeof(client_info_t));
            info->fd = accept(tcp_fd, NULL, NULL);
            info->is_udp = 0;
            if (info->fd >= 0) {
                pthread_t tid;
                pthread_create(&tid, NULL, server_thread, info);
                pthread_detach(tid);
            } else {
                free(info);
            }
        }

        // ---- UDP 收封包 ----
        if (FD_ISSET(udp_fd, &readfds)) {
            client_info_t *info = malloc(sizeof(client_info_t));
            info->fd = udp_fd;
            info->is_udp = 1;
            struct sockaddr_in src;
            socklen_t addrlen = sizeof(src);
            int n = recvfrom(udp_fd, NULL, 0, MSG_PEEK, (struct sockaddr*)&src, &addrlen);
            if (n >= 0) {
                info->addr = src;
                pthread_t tid;
                pthread_create(&tid, NULL, server_thread, info);
                pthread_detach(tid);
            } else {
                free(info);
            }
        }
    }

    close(tcp_fd);
    close(udp_fd);
    return 0;
}

