#include "common/Logger.hpp"
#include <iostream>

void log(const std::string& tag, const std::string& message) {
    std::cout << "[" << tag << "] " << message << std::endl;
}
