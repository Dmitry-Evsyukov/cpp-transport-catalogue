#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace transport_manager::parse_query {
    std::tuple<std::string, double, double> ParseStop(std::string_view line);

    std::pair<std::string, std::vector<std::string>> ParseBus(std::string_view line);

    std::pair<std::string, std::vector<std::pair<std::string, int>>> ParseStopLength(std::string_view line);
}