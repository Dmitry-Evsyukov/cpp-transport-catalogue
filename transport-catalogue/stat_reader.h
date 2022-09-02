#pragma once
#include <iostream>
#include <set>
#include <vector>

namespace transport_manager::output {
    struct BusResponse {
        std::string_view bus_number;
        int stops_on_route = 0;
        int unique_stops = 0;
        double route_length = 0;
        double courvature = 0;
    };

    struct StopResponse {
        std::vector<std::string> buses;
        bool flag;
        std::string name;
    };

    std::ostream &operator<<(std::ostream &out, const BusResponse &response);

    std::ostream &operator<<(std::ostream &out, const StopResponse &response);
}
