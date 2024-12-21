#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/if_ether.h> // 以太網標頭

int main() {

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    char buffer[65536];
    while (1) {
        ssize_t data_size = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (data_size < 0) {
            perror("Recvfrom failed");
            close(sock);
            return 1;
        }

        // 解讀以太網標頭
        struct ethhdr *eth = (struct ethhdr *)buffer;
        printf("Destination MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
               eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
        printf("Source MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               eth->h_source[0], eth->h_source[1], eth->h_source[2],
               eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    }

    close(sock);
    return 0;
}

