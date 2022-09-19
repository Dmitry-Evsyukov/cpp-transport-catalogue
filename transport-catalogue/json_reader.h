#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"


namespace transport_manager::parse_query {
    std::tuple<std::string, double, double> ParseStop(const json::Node& stop_json);

    std::pair<std::string, std::vector<std::string>> ParseBus(const json::Node& stop_json);

    std::pair<std::string, std::vector<std::pair<std::string, int>>> ParseStopLength(const json::Node& stop_json);
}


namespace transport_manager::output {
    void PrintJSONQueries(const json::Array& stat_requests, const transport_manager::TransportManager& transport_manager, const json::Array& buses, const json::Array& stops, const RenderSet& render_set, const SphereProjector& projector);
}
