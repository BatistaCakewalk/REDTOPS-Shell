#include "../headers/portscan.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <queue>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

// Thread-safe container
static std::mutex result_mutex;
static std::vector<int> open_ports;

static bool is_port_open(const std::string& host, int port, int timeout_ms = 120)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        close(sockfd);
        return false;
    }

    int result = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
    if (result < 0 && errno != EINPROGRESS) {
        close(sockfd);
        return false;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000;

    result = select(sockfd + 1, nullptr, &writefds, nullptr, &tv);

    if (result > 0) {
        int error;
        socklen_t len = sizeof(error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
        close(sockfd);
        return error == 0;
    }

    close(sockfd);
    return false;
}

void scan_worker(const std::string& host, std::queue<int>& tasks, std::mutex& task_mutex)
{
    while (true) {
        int port = -1;

        {   // lock task queue
            std::lock_guard<std::mutex> lock(task_mutex);
            if (tasks.empty())
                return;
            port = tasks.front();
            tasks.pop();
        }

        if (is_port_open(host, port)) {
            std::lock_guard<std::mutex> lock(result_mutex);
            open_ports.push_back(port);
        }
    }
}

void PortScanCommand::Execute(const std::vector<std::string>& args)
{
    if (args.size() != 3) {
        std::cout << "Usage: portscan <host> <start> <end>\n";
        return;
    }

    std::string host = args[0];
    int start_port = std::stoi(args[1]);
    int end_port   = std::stoi(args[2]);

    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        std::cout << "Invalid port range.\n";
        return;
    }

    std::cout << "Scanning " << host << " ports " 
              << start_port << "-" << end_port << "...\n";

    // Prepare a task queue
    std::queue<int> tasks;
    std::mutex task_mutex;

    for (int p = start_port; p <= end_port; p++)
        tasks.push(p);

    open_ports.clear();

    // Launch threads based on hardware
    int threads = std::thread::hardware_concurrency();
    if (threads < 4) threads = 4;  // minimum

    std::vector<std::thread> workers;

    for (int i = 0; i < threads; i++)
        workers.emplace_back(scan_worker, host, std::ref(tasks), std::ref(task_mutex));

    for (auto& t : workers)
        t.join();

    // Print ordered results
    std::sort(open_ports.begin(), open_ports.end());

    if (open_ports.empty()) {
        std::cout << "No open ports found.\n";
    } else {
        for (int p : open_ports)
            std::cout << "[OPEN] " << p << "\n";
    }

    std::cout << "Scan complete.\n";
}

