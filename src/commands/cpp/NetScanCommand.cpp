#include "../headers/netscan.hpp"
#include "../../core/header/TerminalRenderer.hpp"
#include <iostream>
#include <array>
#include <memory>
#include <cstdlib>

void NetScanCommand::Execute(const std::vector<std::string>& args) {
    std::string subnet = "192.168.1"; // default
    if (!args.empty()) subnet = args[0];

    TerminalRenderer::Instance().PrintLine("Scanning subnet " + subnet + ".0/24 ...");

    std::string cmd = "for ip in $(seq 1 254); do ping -c 1 -W 1 " + subnet + ".$ip >/dev/null 2>&1 && echo \"" + subnet + ".$ip is up\"; done";
    std::system(cmd.c_str());
}

