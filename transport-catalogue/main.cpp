#include <iostream>
#include <string>
#include <vector>
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

using namespace std;
using namespace json;

int main() {
    const auto dict = Load(cin).GetRoot().AsDict();
    const auto base_requests = dict.at("base_requests").AsArray();
    const auto stat_requests = dict.at("stat_requests").AsArray();
    const auto render_settings = dict.at("render_settings").AsDict();

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

    for (const auto& node : stops) {
        auto [stop, stops_length] = transport_manager::parse_query::ParseStopLength(node);
        transport_manager.AddStopLength(stop, stops_length);
    }

    for (const auto& node : buses) {
        auto [bus, stops_here] = transport_manager::parse_query::ParseBus(node);
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
    transport_manager::output::PrintJSONQueries(stat_requests, renderer, transport_manager);

    return 0;
}