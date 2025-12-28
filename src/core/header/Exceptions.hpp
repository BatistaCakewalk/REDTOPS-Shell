#pragma once

#include <stdexcept>
#include <string>

namespace RedTops {

class RedTopsException : public std::runtime_error {
public:
    explicit RedTopsException(const std::string& message)
        : std::runtime_error(message) {}
};

class CommandError : public RedTopsException {
public:
    explicit CommandError(const std::string& message)
        : RedTopsException(message) {}
};

class NetworkError : public RedTopsException {
public:
    explicit NetworkError(const std::string& message)
        : RedTopsException(message) {}
};

class PermissionError : public RedTopsException {
public:
    explicit PermissionError(const std::string& message)
        : RedTopsException(message) {}
};

} // namespace RedTops
