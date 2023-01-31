#include "domain.h"
#include <fstream>
#include "serialization.h"
using namespace std;
using namespace json;

namespace transport_manager::input {
int MakeBase() {
    const auto dict = Load(cin).GetRoot().AsDict();
    const auto base_requests = dict.at("base_requests").AsArray();
    const auto serialization_settings = dict.at("serialization_settings").AsDict();
    const auto render_settings = dict.at("render_settings").AsDict();
    const auto routing_settings = dict.at("routing_settings").AsDict();
    json::Array buses;
    json::Array stops;

    transport_manager::TransportManager transport_manager;

    vector<transport_manager::compute_length::Coordinates> points;
    for (const auto &node: base_requests) {
        if (node.AsDict().at("type").AsString() == "Stop") {
            stops.push_back(node);
            auto [name, lon, lat] = transport_manager::parse_query::ParseStop(node);
            transport_manager.AddStop(name, lat, lon);
        } else {
            buses.push_back(node);
        }
    }

    vector<string> stop_names;
    for (const auto &node: stops) {
        auto [stop, stops_length] = transport_manager::parse_query::ParseStopLength(node);
        transport_manager.AddStopLength(stop, stops_length);
        stop_names.push_back(stop);
    }



    std::vector<std::pair<string, vector<string>>> bus_stops;
    for (const auto &node: buses) {
        auto [bus, stops_here] = transport_manager::parse_query::ParseBus(node);
        bus_stops.push_back({bus, stops_here});
        transport_manager.AddBus(bus, stops_here);
        for (const auto& stop : stops_here) {
        auto buf = transport_manager.GetStop(stop);
        points.push_back(transport_manager::compute_length::Coordinates(buf.latitude, buf.longitude));
        }
    }
    const auto filename = serialization_settings.at("file").AsString();
    ofstream out_file(filename, ios::binary);
    if (!out_file) {
        return -1;
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

    graph::TransportRouter t_r(bus_stops, transport_manager.GetStSLength(), routing_settings.at("bus_wait_time").AsInt(), routing_settings.at("bus_velocity").AsDouble(), stop_names.size() * 2, stop_names);
    t_r.Initialize();
    t_r.FindNearestWay("Морской вокзал", "Параллельная улица");
    SerializeTransportCatalogue(out_file, transport_manager, render_set, projector, stops, buses, move(t_r));


    return 0;
}
}

