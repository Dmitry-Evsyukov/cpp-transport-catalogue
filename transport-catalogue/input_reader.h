#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "transport_catalogue.h"

namespace transport_manager::parse_query {
    struct ParseStructStop {
        std::string name;
        double longitude = 0;
        double latitude = 0;
    };

    struct ParseStructBus {
        std::string name;
        std::vector<std::string> stops;
    };

    struct ParseStructStopLength {
        std::string name;
        std::vector<std::pair<std::string, int>> stops_distances;
    };

    ParseStructStop ParseStop(std::string_view line);

    ParseStructBus ParseBus(std::string_view line);

    ParseStructStopLength ParseStopLength(std::string_view line);

    transport_manager::TransportManager ParseQueries(std::istream& input);
}