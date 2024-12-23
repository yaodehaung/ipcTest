#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>

// parser eth header  -> Data Link layer , Physical Layer
void parse_ethernet(const uint8_t *buffer) {
    const struct ethhdr *eth = (struct ethhdr *)buffer;

    printf("Ethernet Header:\n");
    printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
           eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

    printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_source[0], eth->h_source[1], eth->h_source[2],
           eth->h_source[3], eth->h_source[4], eth->h_source[5]);

    printf("Protocol: 0x%04x\n", ntohs(eth->h_proto));
}

// paser IP header -> IP , ICMP , ARP
void parse_ip(const uint8_t *buffer) {
    const struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));

    printf("IP Header:\n");
    printf("  Version: %u\n", ip->version);
    printf("  Header Length: %u bytes\n", ip->ihl * 4);
    printf("  Type of Service: %u\n", ip->tos);
    printf("  Total Length: %u\n", ntohs(ip->tot_len));
    printf("  Identification: %u\n", ntohs(ip->id));
    printf("  TTL: %u\n", ip->ttl);
    printf("  Protocol: %u\n", ip->protocol);

    struct in_addr src_ip, dest_ip;
    src_ip.s_addr = ip->saddr;
    dest_ip.s_addr = ip->daddr;

    printf("  Source IP: %s\n", inet_ntoa(src_ip));
    printf("  Destination IP: %s\n", inet_ntoa(dest_ip));
}

// 解析 TCP 標頭 
void parse_tcp(const uint8_t *buffer) {
    const struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    const struct tcphdr *tcp = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + ip->ihl * 4);

    printf("TCP Header:\n");
    printf("  Source Port: %u\n", ntohs(tcp->source));
    printf("  Destination Port: %u\n", ntohs(tcp->dest));
    printf("  Sequence Number: %u\n", ntohl(tcp->seq));
    printf("  Acknowledgment Number: %u\n", ntohl(tcp->ack_seq));
    printf("  Flags: 0x%02x\n", tcp->fin | (tcp->syn << 1) | (tcp->rst << 2) |
                                    (tcp->psh << 3) | (tcp->ack << 4) | (tcp->urg << 5));
    printf("  Window Size: %u\n", ntohs(tcp->window));
    printf("  Checksum: 0x%04x\n", ntohs(tcp->check));
}

// 解析 UDP 標頭
void parse_udp(const uint8_t *buffer) {
    const struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    const struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + ip->ihl * 4);

    printf("UDP Header:\n");
    printf("  Source Port: %u\n", ntohs(udp->source));
    printf("  Destination Port: %u\n", ntohs(udp->dest));
    printf("  Length: %u\n", ntohs(udp->len));
    printf("  Checksum: 0x%04x\n", ntohs(udp->check));
}

// 打印數據部分 (Payload)
void print_payload(const uint8_t *buffer, size_t len) {
    setlocale(LC_CTYPE, "");  // Set locale for Unicode support
    printf("Payload (%zu bytes):\n", len);
    
    for (size_t i = 0; i < len; i += 16) {
        // Print the offset
        printf("%04zx  ", i);

        // Print the hex representation
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02x ", buffer[i + j]);
            } else {
                printf("   ");  // Padding for rows less than 16 bytes
            }
        }

        // Print ASCII representation
        printf(" | ");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                if (buffer[i + j] >= 32 && buffer[i + j] <= 126) {
                    printf("%c", buffer[i + j]);
                } else {
                    printf(".");  // Replace non-printable characters with '.'
                }
            }
        }

        printf("\n");  // Move to the next line
    }
    printf("\n");
}

// 綜合處理流程
void process_packet(const uint8_t *buffer, size_t len) {
    parse_ethernet(buffer);

    const struct ethhdr *eth = (struct ethhdr *)buffer;
    if (ntohs(eth->h_proto) == ETH_P_IP) {
        parse_ip(buffer);

        const struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol == IPPROTO_TCP) {
            parse_tcp(buffer);
        } else if (ip->protocol == IPPROTO_UDP) {
            parse_udp(buffer);
        } else {
            printf("Unsupported IP Protocol: %u\n", ip->protocol);
        }

        // 打印數據部分
        size_t ip_header_len = ip->ihl * 4;
        size_t payload_offset = sizeof(struct ethhdr) + ip_header_len;
        size_t payload_len = len - payload_offset;
        print_payload(buffer + payload_offset, payload_len);
    } else {
        printf("Not an IP packet (Protocol: 0x%04x)\n", ntohs(eth->h_proto));
    }
}

// 主函式，使用 RAW socket 接收資料
int main() {
    int sockfd;
    struct sockaddr saddr;
    uint8_t buffer[65536];
    socklen_t saddr_len = sizeof(saddr);

    // 創建 RAW socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Listening for packets...\n");

    while (1) {
        ssize_t packet_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, &saddr, &saddr_len);
        if (packet_len < 0) {
            perror("recvfrom failed");
            break;
        }

        printf("\nReceived packet (%zd bytes):\n", packet_len);
        process_packet(buffer, packet_len);
    }

    close(sockfd);
    return 0;
}

