#include "../headers/trace.hpp"
#include "../../core/header/TerminalRenderer.hpp"

#include <iostream>
#include <string>
#include <chrono>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>

// Compute checksum for ICMP packet
static uint16_t icmp_checksum(uint16_t* data, int length) {
    uint32_t sum = 0;
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }
    if (length == 1) {
        sum += *(uint8_t*)data;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

void TraceCommand::Execute(const std::vector<std::string>& args) {
    auto& renderer = TerminalRenderer::Instance();

    if (args.empty()) {
        renderer.PrintLine("Usage: trace <host>");
        return;
    }

    std::string host = args[0];
    renderer.PrintLine("Tracing route to " + host + "...\n");

    // Resolve hostname → IP
    sockaddr_in dest{};
    dest.sin_family = AF_INET;

    if (inet_pton(AF_INET, host.c_str(), &dest.sin_addr) <= 0) {
        renderer.PrintLine("Error: invalid host or IP.");
        return;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0) {
        renderer.PrintLine("Error: need root privileges to run traceroute.");
        return;
    }

    // Storage for replies
    sockaddr_in reply_addr{};
    socklen_t reply_len = sizeof(reply_addr);

    const int MAX_HOPS = 30;

    for (int ttl = 1; ttl <= MAX_HOPS; ++ttl) {
        // Set TTL
        if (setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            renderer.PrintLine("Error: failed to set TTL");
            close(sock);
            return;
        }

        // Build ICMP Echo Request
        char packet[64];
        memset(packet, 0, sizeof(packet));

        struct icmphdr* icmp = (struct icmphdr*)packet;
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = getpid() & 0xFFFF;
        icmp->un.echo.sequence = ttl;
        icmp->checksum = icmp_checksum((uint16_t*)packet, sizeof(packet));

        // Send packet
        auto start = std::chrono::high_resolution_clock::now();
        sendto(sock, packet, sizeof(packet), 0, (sockaddr*)&dest, sizeof(dest));

        // Wait for reply
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        timeval tv{2, 0};  // 2-second timeout
        int ready = select(sock + 1, &readfds, nullptr, nullptr, &tv);

        if (ready <= 0) {
            // Timeout — or unreachable hop
            renderer.PrintLine(std::to_string(ttl) + "   *  (timeout)");
            continue;
        }

        char buffer[512];
        int bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&reply_addr, &reply_len);

        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        char hopIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &reply_addr.sin_addr, hopIP, sizeof(hopIP));

        // Print hop result
        renderer.PrintLine(
            std::to_string(ttl) + "   " +
            std::string(hopIP) + "   " +
            std::to_string(ms) + " ms"
        );

        // If we reached the destination, stop
        if (reply_addr.sin_addr.s_addr == dest.sin_addr.s_addr) {
            renderer.PrintLine("\nTrace complete.");
            break;
        }
    }

    close(sock);
}

