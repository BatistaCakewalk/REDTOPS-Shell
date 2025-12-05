#include "../headers/netinfo.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <cstdlib>
#include <iomanip>

void NetInfoCommand::Execute(const std::vector<std::string>& args) {
    TerminalRenderer& term = TerminalRenderer::Instance();

    bool show_interfaces = false;
    bool show_routes = false;
    bool check_connectivity = false;
    bool verbose = false;
    std::vector<std::string> hosts_to_check;

    // Parse flags
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-i" || arg == "--interfaces") show_interfaces = true;
        else if (arg == "-r" || arg == "--routes") show_routes = true;
        else if (arg == "-v" || arg == "--verbose") verbose = true;
        else if ((arg == "-c" || arg == "--check") && i + 1 < args.size()) {
            check_connectivity = true;
            hosts_to_check.push_back(args[++i]);
        }
    }

    // Default behavior
    if (!show_interfaces && !show_routes && !check_connectivity) {
        show_interfaces = true;
        show_routes = true;
    }

    // ===== Interfaces =====
    if (show_interfaces) {
        term.PrintLine("\033[1;34m=== Network Interfaces ===\033[0m");

        struct ifaddrs* ifaddr;
        if (getifaddrs(&ifaddr) == -1) {
            term.PrintLine("Failed to retrieve network interfaces.");
        } else {
            std::set<std::string> seen;
            for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
                if (!ifa->ifa_addr) continue;
                std::string name = ifa->ifa_name;
                if (seen.count(name)) continue;
                seen.insert(name);

                std::vector<std::string> ipv4_addrs, ipv6_addrs;

                for (struct ifaddrs* cur = ifaddr; cur != nullptr; cur = cur->ifa_next) {
                    if (!cur->ifa_addr) continue;
                    if (name != cur->ifa_name) continue;

                    char buf[INET6_ADDRSTRLEN];
                    if (cur->ifa_addr->sa_family == AF_INET) {
                        sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(cur->ifa_addr);
                        inet_ntop(AF_INET, &(sa->sin_addr), buf, INET_ADDRSTRLEN);
                        ipv4_addrs.push_back(buf);
                    } else if (cur->ifa_addr->sa_family == AF_INET6) {
                        sockaddr_in6* sa6 = reinterpret_cast<sockaddr_in6*>(cur->ifa_addr);
                        inet_ntop(AF_INET6, &(sa6->sin6_addr), buf, INET6_ADDRSTRLEN);
                        ipv6_addrs.push_back(buf);
                    }
                }

                std::ifstream macfile("/sys/class/net/" + name + "/address");
                std::string mac = "?";
                if (macfile) std::getline(macfile, mac);

                term.PrintLine("Interface: " + name);
                term.PrintLine("  IPv4: " + (ipv4_addrs.empty() ? "N/A" : ipv4_addrs[0]));
                term.PrintLine("  IPv6: " + (ipv6_addrs.empty() ? "N/A" : ipv6_addrs[0]));
                term.PrintLine("  MAC : " + (mac.empty() ? "?" : mac));

                if (verbose) {
                    std::ifstream mtufile("/sys/class/net/" + name + "/mtu");
                    std::string mtu;
                    if (mtufile) std::getline(mtufile, mtu);

                    term.PrintLine("  MTU : " + (mtu.empty() ? "?" : mtu));
                    term.PrintLine("  Flags: " + std::to_string(ifa->ifa_flags));

                    // Optional: RX/TX stats
                    std::ifstream rxfile("/sys/class/net/" + name + "/statistics/rx_bytes");
                    std::ifstream txfile("/sys/class/net/" + name + "/statistics/tx_bytes");
                    std::string rx, tx;
                    if (rxfile) std::getline(rxfile, rx);
                    if (txfile) std::getline(txfile, tx);
                    term.PrintLine("  RX  : " + (rx.empty() ? "?" : rx) + " bytes");
                    term.PrintLine("  TX  : " + (tx.empty() ? "?" : tx) + " bytes");
                }

                term.PrintLine(""); // spacing
            }
            freeifaddrs(ifaddr);
        }
    }

    // ===== Routes =====
    if (show_routes) {
        term.PrintLine("\033[1;34m=== Routing Table ===\033[0m");
        std::ifstream route_file("/proc/net/route");
        std::string line;
        int lineno = 0;
        while (std::getline(route_file, line)) {
            lineno++;
            if (lineno == 1) continue; // skip header
            std::istringstream iss(line);
            std::string iface, dest_hex, gateway_hex;
            int flags, refcnt, use, metric, mask_hex;
            if (!(iss >> iface >> dest_hex >> gateway_hex >> std::hex >> flags >> refcnt >> use >> metric >> mask_hex))
                continue;

            if (dest_hex == "00000000") { // default gateway
                unsigned long gw = std::stoul(gateway_hex, nullptr, 16);
                std::ostringstream oss;
                oss << ((gw & 0xFF)) << "." << ((gw >> 8) & 0xFF) << "."
                    << ((gw >> 16) & 0xFF) << "." << ((gw >> 24) & 0xFF);
                term.PrintLine("Default Gateway: " + oss.str() + " via " + iface);
            }
        }
    }

    // ===== Connectivity Check =====
    if (check_connectivity) {
        for (auto& host : hosts_to_check) {
            term.PrintLine("\033[1;34m=== Checking Connectivity ===\033[0m");
            term.PrintLine("Pinging " + host + " ...");

            FILE* pipe = popen(("ping -c 1 " + host).c_str(), "r");
            if (!pipe) {
                term.PrintLine("Failed to execute ping.");
                continue;
            }

            char buffer[128];
            bool reachable = false;
            while (fgets(buffer, sizeof(buffer), pipe)) {
                std::string line(buffer);
                if (line.find("1 received") != std::string::npos ||
                    line.find("1 packets received") != std::string::npos)
                    reachable = true;
            }
            pclose(pipe);
            term.PrintLine(host + (reachable ? " is reachable." : " is not reachable."));
        }
    }
}
