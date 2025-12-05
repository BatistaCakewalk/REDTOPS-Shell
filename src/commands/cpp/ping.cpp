#include "../headers/ping.hpp"
#include "../../core/TerminalRenderer.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <numeric>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <fstream>

PingCommand::PingCommand() {}

// Helper to resolve host to IP
std::string ResolveHost(const std::string& host) {
    std::string cmd = "getent hosts " + host + " | awk '{ print $1 }' > /tmp/redtops_ip.txt";
    system(cmd.c_str());
    std::ifstream f("/tmp/redtops_ip.txt");
    std::string ip;
    if (f) std::getline(f, ip);
    return ip.empty() ? host : ip;
}

// Helper to fetch GeoIP using ip-api.com
std::string GeoIPLookup(const std::string& ip) {
    std::string cmd = "curl -s http://ip-api.com/line/" + ip + "?fields=country,regionName,city | tr '\\n' ',' | sed 's/,$//' > /tmp/redtops_geo.txt";
    system(cmd.c_str());
    std::ifstream f("/tmp/redtops_geo.txt");
    std::string geo;
    if (f) std::getline(f, geo);
    return geo.empty() ? "Unknown Location" : geo;
}

void PingCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        TerminalRenderer::Instance().PrintLine("Usage: ping [options] <host1> <host2> ...");
        TerminalRenderer::Instance().PrintLine("Options:");
        TerminalRenderer::Instance().PrintLine("  -C       Continuous ping");
        TerminalRenderer::Instance().PrintLine("  -6       IPv6 mode");
        TerminalRenderer::Instance().PrintLine("  -g       GeoIP lookup");
        TerminalRenderer::Instance().PrintLine("  -v       Verbose output");
        return;
    }

    // Parse flags
    bool continuous = false;
    bool ipv6 = false;
    bool geoip = false;
    bool verbose = false;
    std::vector<std::string> hosts;

    for (auto& arg : args) {
        if (arg == "-C") continuous = true;
        else if (arg == "-6") ipv6 = true;
        else if (arg == "-g") geoip = true;
        else if (arg == "-v") verbose = true;
        else hosts.push_back(arg);
    }

    for (auto& host : hosts) {
        std::vector<int> latencies;
        int sent = 0, received = 0;

        TerminalRenderer::Instance().PrintLine("\n\033[1;34m=== Pinging " + host + " ===\033[0m");

        std::string ip = ResolveHost(host);
        TerminalRenderer::Instance().PrintLine("Resolved IP: " + ip);

        if (geoip) {
            std::string location = GeoIPLookup(ip);
            TerminalRenderer::Instance().PrintLine("GeoIP: " + location);
        }

        auto do_ping = [&]() {
            for (int i = 0; i < 4 || continuous; ++i) {
                auto start = std::chrono::high_resolution_clock::now();

                std::string ping_cmd = "ping -c 1 ";
                if (ipv6) ping_cmd += "-6 ";
                ping_cmd += ip + " > /tmp/redtops_ping_output.txt 2>&1";
                system(ping_cmd.c_str());

                auto end = std::chrono::high_resolution_clock::now();
                int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                latencies.push_back(ms);
                ++sent;

                std::stringstream reply;
                reply << "Reply from " << ip << ": time=" << ms << "ms";
                if (verbose) reply << " seq=" << i+1;
                TerminalRenderer::Instance().PrintLine(reply.str());

                ++received;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                if (continuous) break;  // temporary
            }
        };

        do_ping();

        int min_latency = *std::min_element(latencies.begin(), latencies.end());
        int max_latency = *std::max_element(latencies.begin(), latencies.end());
        double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
        double variance = 0.0;
        for (auto t : latencies) variance += (t - avg_latency) * (t - avg_latency);
        double stddev = std::sqrt(variance / latencies.size());

        TerminalRenderer::Instance().PrintLine("\n\033[1;36m--- Ping Statistics ---\033[0m");
        TerminalRenderer::Instance().PrintLine("Host: " + host + " (" + ip + ")");
        TerminalRenderer::Instance().PrintLine("Packets: Sent = " + std::to_string(sent) +
                                                ", Received = " + std::to_string(received) +
                                                ", Lost = " + std::to_string(sent - received));
        TerminalRenderer::Instance().PrintLine("Latency (ms): min=" + std::to_string(min_latency) +
                                                ", max=" + std::to_string(max_latency) +
                                                ", avg=" + std::to_string(static_cast<int>(avg_latency)) +
                                                ", stddev=" + std::to_string(static_cast<int>(stddev)));

        TerminalRenderer::Instance().PrintLine("Latency Graph:");
        int max_bar = 20;
        for (auto t : latencies) {
            int bar_len = static_cast<int>((t - min_latency) / double(max_latency - min_latency + 1) * max_bar);
            std::string bar(bar_len, '=');
            TerminalRenderer::Instance().PrintLine("[" + bar + "] " + std::to_string(t) + "ms");
        }
    }
}
