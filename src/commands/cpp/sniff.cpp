#include "../headers/sniff.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include "../../core/header/Exceptions.hpp"
#include "../../core/header/Shell.hpp" // Include Shell.hpp for handle management
#include <pcap.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <iomanip>
#include <sstream>

// Callback function for libpcap
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    TerminalRenderer& renderer = TerminalRenderer::Instance();

    // Ethernet header
    struct ethhdr *eth_header = (struct ethhdr *) packet;
    
    std::stringstream ss;
    ss << "----------------------------------------------------" << std::endl;
    ss << "Packet captured! Length: " << pkthdr->len << std::endl;
    ss << "Source MAC: " << std::hex << std::setfill('0');
    for (int i = 0; i < ETH_ALEN; ++i) ss << std::setw(2) << (int)eth_header->h_source[i] << (i == ETH_ALEN - 1 ? "" : ":");
    ss << std::endl;
    ss << "Dest MAC:   " << std::hex << std::setfill('0');
    for (int i = 0; i < ETH_ALEN; ++i) ss << std::setw(2) << (int)eth_header->h_dest[i] << (i == ETH_ALEN - 1 ? "" : ":");
    ss << std::endl;
    
    // IP header
    if (ntohs(eth_header->h_proto) == ETHERTYPE_IP) {
        struct iphdr *ip_header = (struct iphdr *)(packet + ETH_HLEN);
        ss << "Source IP: " << inet_ntoa(*(in_addr *)&ip_header->saddr) << std::endl;
        ss << "Dest IP:   " << inet_ntoa(*(in_addr *)&ip_header->daddr) << std::endl;
        ss << "Protocol:  " << (unsigned int)ip_header->protocol << std::endl;

        unsigned int ip_header_len = ip_header->ihl * 4;
        
        // TCP header
        if (ip_header->protocol == IPPROTO_TCP) {
            struct tcphdr *tcp_header = (struct tcphdr *)(packet + ETH_HLEN + ip_header_len);
            ss << "Source Port: " << ntohs(tcp_header->source) << std::endl;
            ss << "Dest Port:   " << ntohs(tcp_header->dest) << std::endl;
        }
        // UDP header
        else if (ip_header->protocol == IPPROTO_UDP) {
            struct udphdr *udp_header = (struct udphdr *)(packet + ETH_HLEN + ip_header_len);
            ss << "Source Port: " << ntohs(udp_header->source) << std::endl;
            ss << "Dest Port:   " << ntohs(udp_header->dest) << std::endl;
        }
    }
    ss << "----------------------------------------------------";

    renderer.PrintLine(ss.str(), Color::CYAN);
}

SniffCommand::SniffCommand() {}

void SniffCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        throw RedTops::CommandError("sniff: usage: sniff <interface> [count]");
    }

    std::string interface = args[0];
    int count = 0; // 0 means sniff indefinitely
    if (args.size() > 1) {
        try {
            count = std::stoi(args[1]);
        } catch (const std::invalid_argument& e) {
            throw RedTops::CommandError("sniff: invalid count: " + args[1]);
        } catch (const std::out_of_range& e) {
            throw RedTops::CommandError("sniff: count out of range: " + args[1]);
        }
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    // Open the device for sniffing
    handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        throw RedTops::NetworkError("sniff: Could not open interface " + interface + ": " + errbuf);
    }

    // Check if DLT_EN10MB (Ethernet) is supported
    if (pcap_datalink(handle) != DLT_EN10MB) {
        pcap_close(handle);
        throw RedTops::NetworkError("sniff: Interface " + interface + " is not Ethernet");
    }
    
    TerminalRenderer::Instance().PrintLine("Starting packet capture on interface " + interface + "...");
    if (count == 0) {
        TerminalRenderer::Instance().PrintLine("Sniffing indefinitely. Press Ctrl+C to stop.");
    } else {
        TerminalRenderer::Instance().PrintLine("Capturing " + std::to_string(count) + " packets.");
    }

    // Set the pcap handle in the Shell so the signal handler can access it
    Shell::Instance().SetCurrentPcapHandle(handle);

    // Loop forever (or until 'count' packets are captured)
    // The packet_handler function will be called for each packet
    int result = pcap_loop(handle, count, packet_handler, NULL);
    
    // Clear the pcap handle from the Shell once done or on error
    Shell::Instance().ClearCurrentPcapHandle();

    if (result == -1) { // Error
        pcap_close(handle);
        throw RedTops::NetworkError("sniff: Error during capture: " + std::string(pcap_geterr(handle)));
    } else if (result == -2) { // Loop terminated by pcap_breakloop
        // This is fine, usually means Ctrl+C
        TerminalRenderer::Instance().PrintLine("Packet capture stopped.", Color::AMBER);
    } else if (result == 0 && count != 0) { // Number of packets captured
        TerminalRenderer::Instance().PrintLine("Finished capturing " + std::to_string(count) + " packets.", Color::AMBER);
    }


    // Close the handle
    pcap_close(handle);
}