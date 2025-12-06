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
#include <cstring>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
    #include <iphlpapi.h>
    #pragma comment(lib, "iphlpapi.lib")
#else
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <sys/sysinfo.h>
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <net/if.h>
#endif



void SysInfoCommand::Execute(const std::vector<std::string>& args) {
    bool show_all = true;
    bool show_cpu = false;
    bool show_mem = false;
    bool show_os = false;
    bool show_uptime = false;
    bool show_net = false;
    bool cpu_adv = false;

    for (auto& arg : args) {
        if (arg == "-cpu") { show_cpu = true; show_all = false; }
        else if (arg == "-mem") { show_mem = true; show_all = false; }
        else if (arg == "-os") { show_os = true; show_all = false; }
        else if (arg == "-uptime") { show_uptime = true; show_all = false; }
        else if (arg == "-net") { show_net = true; show_all = false; }
        else if (arg == "-adv") { cpu_adv = true; show_all = false; show_cpu = true; }
    }

    TerminalRenderer& term = TerminalRenderer::Instance();

    // ===================== OS INFO =====================
    if (show_os || show_all) {
        term.PrintLine("\033[1;34m=== OS Info ===\033[0m");

    #ifdef _WIN32
        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        GetVersionExW((LPOSVERSIONINFOW)&osvi);

        term.PrintLine("System: Windows");
        term.PrintLine("Version: " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion));

        TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD size = sizeof(computerName) / sizeof(computerName[0]);
        GetComputerName(computerName, &size);
        term.PrintLine("Node: " + std::string(computerName));

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        term.PrintLine("Machine: x" + std::to_string(sysInfo.wProcessorArchitecture));

    #else
        struct utsname uname_data;
        uname(&uname_data);

        term.PrintLine("System: " + std::string(uname_data.sysname));
        term.PrintLine("Node: " + std::string(uname_data.nodename));
        term.PrintLine("Release: " + std::string(uname_data.release));
        term.PrintLine("Version: " + std::string(uname_data.version));
        term.PrintLine("Machine: " + std::string(uname_data.machine));
    #endif
    }

    // ===================== CPU INFO =====================
    if (show_cpu || show_all) {
        term.PrintLine("\n\033[1;34m=== CPU Info ===\033[0m");

    #ifdef _WIN32
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            char buf[256];
            DWORD bufSize = sizeof(buf);

            if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
                term.PrintLine("Model: " + std::string(buf));

            bufSize = sizeof(buf);
            if (RegQueryValueExA(hKey, "VendorIdentifier", nullptr, nullptr, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
                term.PrintLine("Vendor: " + std::string(buf));

            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            term.PrintLine("Cores: " + std::to_string(sysInfo.dwNumberOfProcessors));

            RegCloseKey(hKey);
        }

        if (cpu_adv)
            term.PrintLine("\033[1;33m(Advanced flags not yet supported on Windows)\033[0m");

    #else
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        std::vector<std::map<std::string,std::string>> cpus;
        std::map<std::string,std::string> cpu_data;

        auto trim = [](std::string s)->std::string{
            while (!s.empty() && isspace(s.front())) s.erase(0,1);
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

            std::string key = trim(line.substr(0,pos));
            std::string value = trim(line.substr(pos+1));
            cpu_data[key] = value;
        }
        if (!cpu_data.empty()) cpus.push_back(cpu_data);

        if (!cpus.empty()) {
            term.PrintLine("Model: " + cpus[0]["model name"]);
            term.PrintLine("Vendor: " + cpus[0]["vendor_id"]);
            term.PrintLine("Cores: " + std::to_string(cpus.size()));
        }

        if (cpu_adv && !cpus.empty()) {
            term.PrintLine("\n\033[1;33m--- Advanced CPU Info ---\033[0m");
            for (size_t i = 0; i < cpus.size(); i++) {
                term.PrintLine("Core " + std::to_string(i) + ":");
                if (cpus[i].count("cache size")) term.PrintLine("  Cache: " + cpus[i]["cache size"]);
                if (cpus[i].count("flags")) term.PrintLine("  Flags: " + cpus[i]["flags"]);
                if (cpus[i].count("bogomips")) term.PrintLine("  Bogomips: " + cpus[i]["bogomips"]);
            }
        }
    #endif
    }

    // ===================== MEMORY INFO =====================
    if (show_mem || show_all) {
        term.PrintLine("\n\033[1;34m=== Memory Info ===\033[0m");

    #ifdef _WIN32
        MEMORYSTATUSEX mem = { sizeof(mem) };
        GlobalMemoryStatusEx(&mem);

        term.PrintLine("Total RAM: " + std::to_string(mem.ullTotalPhys / 1024 / 1024) + " MB");
        term.PrintLine("Used RAM : " + std::to_string((mem.ullTotalPhys - mem.ullAvailPhys) / 1024 / 1024) + " MB");
        term.PrintLine("Free RAM : " + std::to_string(mem.ullAvailPhys / 1024 / 1024) + " MB");

    #else
        struct sysinfo memInfo;
        sysinfo(&memInfo);

        auto tom = [&](long long v){ return v * memInfo.mem_unit / 1024 / 1024; };
        long long total = tom(memInfo.totalram);
        long long free = tom(memInfo.freeram);

        term.PrintLine("Total RAM: " + std::to_string(total) + " MB");
        term.PrintLine("Used RAM : " + std::to_string(total - free) + " MB");
        term.PrintLine("Free RAM : " + std::to_string(free) + " MB");
    #endif
    }

    // ===================== UPTIME =====================
    if (show_uptime || show_all) {
        term.PrintLine("\n\033[1;34m=== Uptime ===\033[0m");

    #ifdef _WIN32
        ULONGLONG ms = GetTickCount64();
        long sec = ms / 1000;
    #else
        struct sysinfo si;
        sysinfo(&si);
        long sec = si.uptime;
    #endif

        long h = sec / 3600;
        long m = (sec % 3600) / 60;
        long s = sec % 60;

        term.PrintLine("Uptime: " + std::to_string(h) + "h " +
                       std::to_string(m) + "m " +
                       std::to_string(s) + "s");
    }

    // ===================== NETWORK INFO =====================
    if (show_net || show_all) {
        term.PrintLine("\n\033[1;34m=== Network Interfaces ===\033[0m");

    #ifdef _WIN32
        PIP_ADAPTER_ADDRESSES addrs, cur;
        ULONG size = 15000;
        addrs = (IP_ADAPTER_ADDRESSES*)malloc(size);

        if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, addrs, &size) == ERROR_SUCCESS) {
            for (cur = addrs; cur != NULL; cur = cur->Next) {
                term.PrintLine("Interface: " + std::string(cur->AdapterName));

                // IPv4 + IPv6
                for (PIP_ADAPTER_UNICAST_ADDRESS ua = cur->FirstUnicastAddress; ua; ua = ua->Next) {
                    char ip[100];
                    getnameinfo(ua->Address.lpSockaddr, ua->Address.iSockaddrLength,
                                ip, sizeof(ip), NULL, 0, NI_NUMERICHOST);
                    term.PrintLine("  IP: " + std::string(ip));
                }

                // MAC
                std::stringstream ss;
                for (ULONG i = 0; i < cur->PhysicalAddressLength; i++) {
                    ss << std::hex << std::setw(2) << std::setfill('0') << (int)cur->PhysicalAddress[i];
                    if (i < cur->PhysicalAddressLength - 1) ss << ":";
                }
                term.PrintLine("  MAC: " + ss.str());
            }
        }
        free(addrs);

    #else
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1)
            term.PrintLine("Failed to retrieve network info.");
        else {
            std::set<std::string> seen;
            for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
                if (!ifa->ifa_addr) continue;

                std::string name = ifa->ifa_name;
                if (seen.count(name)) continue;
                seen.insert(name);

                int family = ifa->ifa_addr->sa_family;
                std::string addr_str = "?";

                if (family == AF_INET) {
                    char buf[INET_ADDRSTRLEN];
                    auto sa = (sockaddr_in*)ifa->ifa_addr;
                    inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
                    addr_str = buf;
                }
                if (family == AF_INET6) {
                    char buf[INET6_ADDRSTRLEN];
                    auto sa6 = (sockaddr_in6*)ifa->ifa_addr;
                    inet_ntop(AF_INET6, &sa6->sin6_addr, buf, sizeof(buf));
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
    #endif
    }
}
