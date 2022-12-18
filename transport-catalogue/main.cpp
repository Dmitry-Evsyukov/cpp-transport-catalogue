#include <iostream>
#include <string>
#include <vector>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"

using namespace std;
using namespace json;

int main() {
    const auto dict = Load(cin).GetRoot().AsDict();
    const auto base_requests = dict.at("base_requests").AsArray();
    const auto stat_requests = dict.at("stat_requests").AsArray();
    const auto render_settings = dict.at("render_settings").AsDict();
    const auto routing_settings = dict.at("routing_settings").AsDict();

    json::Array buses;
    json::Array stops;
    transport_manager::TransportManager transport_manager;
    vector<transport_manager::compute_length::Coordinates> points;
    for (const auto& node : base_requests) {
        if (node.AsDict().at("type").AsString() == "Stop") {
            stops.push_back(node);
            auto [name, lon, lat] = transport_manager::parse_query::ParseStop(node);
            transport_manager.AddStop(name, lat, lon);
        } else {
            buses.push_back(node);
        }
    }

    vector<string> stop_names;
    for (const auto& node : stops) {
        auto [stop, stops_length] = transport_manager::parse_query::ParseStopLength(node);
        transport_manager.AddStopLength(stop, stops_length);
        stop_names.push_back(stop);
    }

    std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_distances = transport_manager.GetStSLength();

    std::vector<std::pair<string, vector<string>>> bus_stops;
    for (const auto& node : buses) {
        auto [bus, stops_here] = transport_manager::parse_query::ParseBus(node);
        bus_stops.push_back({bus, stops_here});
        transport_manager.AddBus(bus, stops_here);
        for (const auto& stop : stops_here) {
            auto buf = transport_manager.GetStop(stop);
            points.push_back(transport_manager::compute_length::Coordinates(buf.latitude, buf.longitude));
        }
    }
    auto render_set = ParseRenderSet(render_settings);
    SphereProjector projector(points.begin(), points.end(), render_set.width,
                              render_set.height, render_set.padding);

    sort(buses.begin(), buses.end(), [](const Node& lhs, const Node& rhs){
        return lhs.AsDict().at("name").AsString() < rhs.AsDict().at("name").AsString();
    });

    sort(stops.begin(), stops.end(), [](const Node& lhs, const Node& rhs){
        return lhs.AsDict().at("name").AsString() < rhs.AsDict().at("name").AsString();
    });
    RendererMap renderer(transport_manager, buses, stops, render_set, projector);

    graph::TransportRouter transport_router(bus_stops, stop_distances, routing_settings.at("bus_wait_time").AsInt(), routing_settings.at("bus_velocity").AsInt(), stop_names.size() * 2, stop_names);
    transport_manager::output::PrintJSONQueries(stat_requests, renderer, transport_manager, transport_router);

    return 0;
}