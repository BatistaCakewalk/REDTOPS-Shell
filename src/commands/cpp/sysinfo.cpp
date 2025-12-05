#include "../headers/sysinfo.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>
#include <cmath>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <cstring>
#include <iomanip>

void SysInfoCommand::Execute(const std::vector<std::string>& args) {
    bool show_all = true;
    bool show_cpu = false;
    bool show_mem = false;
    bool show_os = false;
    bool show_uptime = false;
    bool show_net = false;
    bool cpu_adv = false;

    // Parse flags
    for (auto& arg : args) {
        if (arg == "-cpu") { show_cpu = true; show_all = false; }
        else if (arg == "-mem") { show_mem = true; show_all = false; }
        else if (arg == "-os") { show_os = true; show_all = false; }
        else if (arg == "-uptime") { show_uptime = true; show_all = false; }
        else if (arg == "-net") { show_net = true; show_all = false; }
        else if (arg == "-adv") { cpu_adv = true; show_all = false; show_cpu = true; }
    }

    TerminalRenderer& term = TerminalRenderer::Instance();

    // ===== OS Info =====
    if (show_os || show_all) {
        struct utsname uname_data;
        uname(&uname_data);
        term.PrintLine("\033[1;34m=== OS Info ===\033[0m");
        term.PrintLine("System: " + std::string(uname_data.sysname));
        term.PrintLine("Node: " + std::string(uname_data.nodename));
        term.PrintLine("Release: " + std::string(uname_data.release));
        term.PrintLine("Version: " + std::string(uname_data.version));
        term.PrintLine("Machine: " + std::string(uname_data.machine));
    }

    // ===== CPU Info =====
    if (show_cpu || show_all) {
        term.PrintLine("\n\033[1;34m=== CPU Info ===\033[0m");
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        std::vector<std::map<std::string,std::string>> cpus;
        std::map<std::string,std::string> cpu_data;

        // Helper to trim whitespace
        auto trim = [](std::string s) -> std::string {
            while (!s.empty() && isspace(s.front())) s.erase(0, 1);
            while (!s.empty() && isspace(s.back())) s.pop_back();
            return s;
        };

        while (std::getline(cpuinfo, line)) {
            if (line.empty()) {
                if (!cpu_data.empty()) {
                    cpus.push_back(cpu_data);
                    cpu_data.clear();
                }
                continue;
            }
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            cpu_data[trim(key)] = trim(value);  // Trim both key and value
        }
        if (!cpu_data.empty()) cpus.push_back(cpu_data);

        if (!cpus.empty()) {
            term.PrintLine("Model: " + cpus[0]["model name"]);
            term.PrintLine("Vendor: " + cpus[0]["vendor_id"]);
            term.PrintLine("Cores: " + std::to_string(cpus.size()));
        }

        if (cpu_adv && !cpus.empty()) {
            term.PrintLine("\n\033[1;33m--- Advanced CPU Info ---\033[0m");
            for (size_t i = 0; i < cpus.size(); ++i) {
                term.PrintLine("Core " + std::to_string(i) + ":");
                if (cpus[i].count("cache size")) term.PrintLine("  Cache: " + cpus[i]["cache size"]);
                if (cpus[i].count("flags")) term.PrintLine("  Flags: " + cpus[i]["flags"]);
                if (cpus[i].count("bogomips")) term.PrintLine("  Bogomips: " + cpus[i]["bogomips"]);
            }
        }
    }

    // ===== Memory Info =====
    if (show_mem || show_all) {
        term.PrintLine("\n\033[1;34m=== Memory Info ===\033[0m");
        struct sysinfo memInfo;
        sysinfo(&memInfo);
        auto to_mb = [&memInfo](long long val){ return val * memInfo.mem_unit / 1024 / 1024; };
        long long totalPhysMem = to_mb(memInfo.totalram);
        long long freePhysMem = to_mb(memInfo.freeram);
        long long usedPhysMem = totalPhysMem - freePhysMem;
        term.PrintLine("Total RAM: " + std::to_string(totalPhysMem) + " MB");
        term.PrintLine("Used RAM : " + std::to_string(usedPhysMem) + " MB");
        term.PrintLine("Free RAM : " + std::to_string(freePhysMem) + " MB");
    }

    // ===== Uptime =====
    if (show_uptime || show_all) {
        term.PrintLine("\n\033[1;34m=== Uptime ===\033[0m");
        struct sysinfo s_info;
        sysinfo(&s_info);
        long uptime_sec = s_info.uptime;
        long hours = uptime_sec / 3600;
        long minutes = (uptime_sec % 3600) / 60;
        long seconds = uptime_sec % 60;
        term.PrintLine("Uptime: " + std::to_string(hours) + "h " +
                       std::to_string(minutes) + "m " +
                       std::to_string(seconds) + "s");
    }

    // ===== Network Info =====
    if (show_net || show_all) {
        term.PrintLine("\n\033[1;34m=== Network Interfaces ===\033[0m");
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1) {
            term.PrintLine("Failed to retrieve network info.");
        } else {
            std::set<std::string> seen; // avoid duplicate interfaces
            for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
                if (!ifa->ifa_addr) continue;
                std::string name = ifa->ifa_name;
                if (seen.count(name)) continue;
                seen.insert(name);

                int family = ifa->ifa_addr->sa_family;
                std::string addr_str = "?";
                if (family == AF_INET) {
                    char buf[INET_ADDRSTRLEN];
                    sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
                    inet_ntop(AF_INET, &(sa->sin_addr), buf, INET_ADDRSTRLEN);
                    addr_str = buf;
                } else if (family == AF_INET6) {
                    char buf[INET6_ADDRSTRLEN];
                    sockaddr_in6* sa6 = reinterpret_cast<sockaddr_in6*>(ifa->ifa_addr);
                    inet_ntop(AF_INET6, &(sa6->sin6_addr), buf, INET6_ADDRSTRLEN);
                    addr_str = buf;
                }

                std::ifstream macfile("/sys/class/net/" + name + "/address");
                std::string mac;
                if (macfile) std::getline(macfile, mac);

                term.PrintLine("Interface: " + name);
                term.PrintLine("  IP   : " + addr_str);
                term.PrintLine("  MAC  : " + (mac.empty() ? "?" : mac));
            }
            freeifaddrs(ifaddr);
        }
    }
}
