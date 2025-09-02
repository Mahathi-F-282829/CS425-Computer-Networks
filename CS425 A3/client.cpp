#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#define DEST_PORT 12345
#define SOURCE_PORT 43210
#define SERVER_IP "127.0.0.1"
#define SYN_SEQ 200
#define SYN_ACK_SEQ 400
#define ACK_SEQ 600

// Pseudo header for TCP checksum calculation
struct pseudo_header {
    u_int32_t src_addr;
    u_int32_t dst_addr;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t tcp_len;
};

// Calculate TCP checksum
unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
    unsigned long sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1)
        sum += *(u_int8_t*)ptr;
    
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

// Create raw socket
int create_raw_socket() {
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

// Configure socket options
void configure_socket(int sock) {
    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

// Send SYN packet
void send_syn(int sock, sockaddr_in *dest_addr) {
    char packet[sizeof(iphdr) + sizeof(tcphdr)] = {0};
    
    // IP header
    struct iphdr *ip = (struct iphdr *)packet;
    ip->version = 4;
    ip->ihl = 5;
    ip->tot_len = htons(sizeof(packet));
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr("127.0.0.1");
    ip->daddr = dest_addr->sin_addr.s_addr;
    
    // TCP header
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(iphdr));
    tcp->source = htons(SOURCE_PORT);
    tcp->dest = htons(DEST_PORT);
    tcp->seq = htonl(SYN_SEQ);
    tcp->doff = 5;
    tcp->syn = 1;
    tcp->window = htons(5840);
    
    // Calculate checksum
    pseudo_header psh = {
        .src_addr = ip->saddr,
        .dst_addr = ip->daddr,
        .placeholder = 0,
        .protocol = IPPROTO_TCP,
        .tcp_len = htons(sizeof(tcphdr))
    };
    
    char pseudogram[sizeof(psh) + sizeof(tcphdr)];
    memcpy(pseudogram, &psh, sizeof(psh));
    memcpy(pseudogram + sizeof(psh), tcp, sizeof(tcphdr));
    tcp->check = calculate_checksum((unsigned short*)pseudogram, sizeof(pseudogram));
    
    if (sendto(sock, packet, sizeof(packet), 0, 
              (struct sockaddr*)dest_addr, sizeof(*dest_addr)) < 0) {
        perror("sendto() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

// Receive SYN-ACK
bool receive_syn_ack(int sock, uint32_t *seq, uint32_t *ack) {
    char buffer[1024];
    sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    
    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    while (true) {
        int received = recvfrom(sock, buffer, sizeof(buffer), 0, 
                               (struct sockaddr*)&src_addr, &addr_len);
        if (received <= 0) return false;
        
        struct iphdr *ip = (struct iphdr*)buffer;
        if (ip->protocol != IPPROTO_TCP) continue;
        
        struct tcphdr *tcp = (struct tcphdr*)(buffer + (ip->ihl * 4));
        if (tcp->syn && tcp->ack && ntohl(tcp->seq) == SYN_ACK_SEQ) {
            *seq = ntohl(tcp->seq);
            *ack = ntohl(tcp->ack_seq);
            return true;
        }
    }
}

// Send ACK packet
void send_ack(int sock, sockaddr_in *dest_addr, uint32_t ack_num) {
    char packet[sizeof(iphdr) + sizeof(tcphdr)] = {0};
    
    // IP header
    struct iphdr *ip = (struct iphdr *)packet;
    ip->version = 4;
    ip->ihl = 5;
    ip->tot_len = htons(sizeof(packet));
    ip->ttl = 64;
    ip->protocol = IPPROTO_TCP;
    ip->saddr = inet_addr("127.0.0.1");
    ip->daddr = dest_addr->sin_addr.s_addr;
    
    // TCP header
    struct tcphdr *tcp = (struct tcphdr *)(packet + sizeof(iphdr));
    tcp->source = htons(SOURCE_PORT);
    tcp->dest = htons(DEST_PORT);
    tcp->seq = htonl(ACK_SEQ);
    tcp->ack_seq = htonl(ack_num + 1);
    tcp->doff = 5;
    tcp->ack = 1;
    tcp->window = htons(5840);
    
    // Calculate checksum
    pseudo_header psh = {
        .src_addr = ip->saddr,
        .dst_addr = ip->daddr,
        .placeholder = 0,
        .protocol = IPPROTO_TCP,
        .tcp_len = htons(sizeof(tcphdr))
    };
    
    char pseudogram[sizeof(psh) + sizeof(tcphdr)];
    memcpy(pseudogram, &psh, sizeof(psh));
    memcpy(pseudogram + sizeof(psh), tcp, sizeof(tcphdr));
    tcp->check = calculate_checksum((unsigned short*)pseudogram, sizeof(pseudogram));
    
    if (sendto(sock, packet, sizeof(packet), 0, 
              (struct sockaddr*)dest_addr, sizeof(*dest_addr)) < 0) {
        perror("sendto() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int sock = create_raw_socket();
    configure_socket(sock);
    
    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    inet_pton(AF_INET, SERVER_IP, &dest_addr.sin_addr);
    
    std::cout << "Sending SYN (seq=" << SYN_SEQ << ")\n";
    send_syn(sock, &dest_addr);
    
    uint32_t seq, ack;
    if (!receive_syn_ack(sock, &seq, &ack)) {
        std::cerr << "SYN-ACK not received\n";
        close(sock);
        return EXIT_FAILURE;
    }
    std::cout << "Received SYN-ACK (seq=" << seq << " ack=" << ack << ")\n";
    
    std::cout << "Sending ACK (seq=" << ACK_SEQ << ")\n";
    send_ack(sock, &dest_addr, seq);
    
    close(sock);
    std::cout << "Handshake complete\n";
    return EXIT_SUCCESS;
}
