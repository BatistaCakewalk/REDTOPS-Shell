#include "../headers/ping.hpp"
#include "../../core/header/TerminalRenderer.hpp"
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
#include <cstdio>

PingCommand::PingCommand() {}

void PingCommand::Execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        TerminalRenderer::Instance().PrintLine("Usage: ping [options] <host1> <host2> ...");
        TerminalRenderer::Instance().PrintLine("Options: -C Continuous, -6 IPv6, -g GeoIP, -v Verbose");
        return;
    }

    bool continuous = false, ipv6 = false, geoip = false, verbose = false;
    std::vector<std::string> hosts;

    for (auto& arg : args) {
        if (arg == "-C") continuous = true;
        else if (arg == "-6") ipv6 = true;
        else if (arg == "-g") geoip = true;
        else if (arg == "-v") verbose = true;
        else hosts.push_back(arg);
    }

    for (auto& host : hosts) {
        TerminalRenderer::Instance().PrintLine("\n\033[1;34m=== Pinging " + host + " ===\033[0m");

        std::string ip = host;

        if (geoip) {
            std::string geo_cmd = "curl -s http://ip-api.com/line/" + ip + "?fields=country,regionName,city | tr '\\n' ',' | sed 's/,$//'";
            FILE* geo_pipe = popen(geo_cmd.c_str(), "r");
            if (geo_pipe) {
                char buf[256];
                if (fgets(buf, sizeof(buf), geo_pipe)) {
                    std::string geo(buf);
                    geo.erase(std::remove(geo.begin(), geo.end(), '\n'), geo.end());
                    TerminalRenderer::Instance().PrintLine("GeoIP: " + geo);
                }
                pclose(geo_pipe);
            }
        }

        std::vector<int> latencies;
        int sent = 0, received = 0;

        auto do_ping = [&]() {
            int count = continuous ? 1 : 4;
            for (int i = 0; i < count; ++i) {
                auto start = std::chrono::high_resolution_clock::now();

                std::string ping_cmd = "ping -c 1 ";
                if (ipv6) ping_cmd += "-6 ";
                ping_cmd += ip + " 2>&1";

                FILE* pipe = popen(ping_cmd.c_str(), "r");
                if (!pipe) {
                    TerminalRenderer::Instance().PrintLine("Failed to run ping command");
                    continue;
                }

                char buffer[256];
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    std::string line(buffer);
                    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
                    if (!line.empty()) TerminalRenderer::Instance().PrintLine(line);
                }
                pclose(pipe);

                auto end = std::chrono::high_resolution_clock::now();
                int ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                latencies.push_back(ms);
                ++sent;
                ++received;

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        };

        do_ping();

        if (!latencies.empty()) {
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
}

