#include "core/Shell.h"
#include "core/CommandRegistry.h"
#include <memory>


// Builtin commands are registered here for the skeleton
int main(int argc, char** argv) {
    // register builtins
    // CommandRegistry::Instance().Register("help", std::make_unique<HelpCommand>());
    // CommandRegistry::Instance().Register("clear", std::make_unique<ClearCommand>());


    Shell shell;
    shell.Start();
    return 0;
}