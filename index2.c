#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

void print_ethhdr(const struct ethhdr *eth) {
    printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
           eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           eth->h_source[0], eth->h_source[1], eth->h_source[2],
           eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("Protocol: 0x%04x\n", ntohs(eth->h_proto));
}

void print_iphdr(const struct iphdr *ip) {
    struct in_addr src_ip, dest_ip;
    src_ip.s_addr = ip->saddr;
    dest_ip.s_addr = ip->daddr;

    printf("Version: %u\n", ip->version);
    printf("IHL: %u\n", ip->ihl);
    printf("Type of Service: %u\n", ip->tos);
    printf("Total Length: %u\n", ntohs(ip->tot_len));
    printf("Identification: %u\n", ntohs(ip->id));
    printf("Flags: %u\n", ip->frag_off);
    printf("TTL: %u\n", ip->ttl);
    printf("Protocol: %u\n", ip->protocol);
    printf("Header Checksum: 0x%04x\n", ntohs(ip->check));
    printf("Source IP: %s\n", inet_ntoa(src_ip));
    printf("Destination IP: %s\n", inet_ntoa(dest_ip));
}

void print_tcphdr(const struct tcphdr *tcp) {
    printf("Source Port: %u\n", ntohs(tcp->source));
    printf("Destination Port: %u\n", ntohs(tcp->dest));
    printf("Sequence Number: %u\n", ntohl(tcp->seq));
    printf("Acknowledgment Number: %u\n", ntohl(tcp->ack_seq));
    printf("Data Offset: %u\n", tcp->doff);
    printf("Flags: 0x%02x\n", tcp->fin << 0 | tcp->syn << 1 | tcp->rst << 2 | tcp->psh << 3 | tcp->ack << 4 | tcp->urg << 5);
    printf("Window Size: %u\n", ntohs(tcp->window));
    printf("Checksum: 0x%04x\n", ntohs(tcp->check));
    printf("Urgent Pointer: %u\n", ntohs(tcp->urg_ptr));
}

void print_udphdr(const struct udphdr *udp) {
    printf("Source Port: %u\n", ntohs(udp->source));
    printf("Destination Port: %u\n", ntohs(udp->dest));
    printf("Length: %u\n", ntohs(udp->len));
    printf("Checksum: 0x%04x\n", ntohs(udp->check));
}

void process_packet(const uint8_t *packet, size_t len) {
    if (len < sizeof(struct ethhdr)) {
        printf("Packet too short\n");
        return;
    }

    // 解析以太網標頭
    const struct ethhdr *eth = (struct ethhdr *)packet;
    print_ethhdr(eth);

    // 檢查協議類型是否為 IP
    if (ntohs(eth->h_proto) != ETH_P_IP) {
        printf("Not an IP packet\n");
        return;
    }

    // 解析 IP 標頭
    const struct iphdr *ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
    print_iphdr(ip);

    // 檢查協議類型以獲取 TCP 或 UDP
    if (ip->protocol == IPPROTO_TCP) {
        const struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(struct ethhdr) + ip->ihl * 4);
        print_tcphdr(tcp);
    } else if (ip->protocol == IPPROTO_UDP) {
        const struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + ip->ihl * 4);
        print_udphdr(udp);
    } else {
        printf("Other IP protocol: %u\n", ip->protocol);
    }

    // 打印原始數據
    printf("Raw packet data:\n");
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", packet[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

int main() {
    // 假設 packet 包含了一個完整的以太網幀數據
    uint8_t packet[1518]; // Ethernet frame max size
    size_t len = sizeof(packet);

    // 這裡的 packet 數據需要根據實際情況填充

    process_packet(packet, len);
    
    return 0;
}

