#pragma once

#include "TerminalRenderer.hpp"

class Shell {
public:
    Shell();
    ~Shell();

    void Start();
    void Stop();

private:
    bool running_ = false;

    // Reference to the singleton
    TerminalRenderer& renderer_ = TerminalRenderer::Instance();

    void RegisterBuiltins();
    void MainLoop();
};
