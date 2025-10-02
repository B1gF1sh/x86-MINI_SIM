#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "cpu.h"

class Parser
{
private:
    std::unordered_map<std::string, uint16_t> label_map;
    std::string last_error;

public:
    // Test Parser -> C++ Terminal
    std::vector<uint8_t> parse(const std::string &filename);

    // Parser -> QT C++ UI
    std::vector<uint8_t> parse_from_string(const std::string &code_string);

    std::string get_last_error();
};
