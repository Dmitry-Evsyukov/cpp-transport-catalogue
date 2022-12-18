#include "transport_router.h"
#include <memory>

const int MINUTES_IN_HOUR = 60;
const int METERS_IN_KILOMETR = 1000;
graph::TransportRouter::TransportRouter(
    std::vector<std::pair<std::string, std::vector<std::string>>> buses,
    std::unordered_map<std::string_view , std::unordered_map<std::string_view, int>> stop_distances, int bus_wait_time,
    double bus_velocity, size_t vertex_count, std::vector<std::string> stops) : buses_(move(buses)), stop_distances_(move(stop_distances)), bus_wait_time_(bus_wait_time),
    bus_velocity_(bus_velocity), graph_(vertex_count), stops_(stops) {}

bool graph::TransportRouter::IsInitialized() const {
    if (!router_) return false;
    return true;
}

void graph::TransportRouter::Initialize() {
    int counter = 0;
    for (const auto& stop : stops_) {
        vertex_name_id_[stop].push_back(counter);
        vertex_name_id_[stop].push_back(counter + 1);

        vertex_stop_[counter] = stop;
        vertex_stop_[counter + 1] = stop;

        counter += 2;
    }
    for (const auto& [bus, stops] : buses_) {
        if (stops.empty()) continue;
        for (size_t i = 0; i < stops.size(); ++i) {
            double current_way = 0;
            int span_count = 1;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                Edge<double> edge;

                edge.from = vertex_name_id_[stops[i]][1];
                edge.to = vertex_name_id_[stops[j]][0];

                if (stop_distances_.count(stops[j - 1]) != 0 && stop_distances_.at(stops[j - 1]).count(stops[j]) != 0) {
                    current_way += stop_distances_[stops[j - 1]][stops[j]] / (bus_velocity_ / MINUTES_IN_HOUR * METERS_IN_KILOMETR);
                } else {
                    current_way += stop_distances_[stops[j]][stops[j - 1]] / (bus_velocity_ / MINUTES_IN_HOUR * METERS_IN_KILOMETR);
                }
                edge.weight = current_way;
                edge.bus_name = bus;
                edge.span_count = span_count;
                graph_.AddEdge(edge);
                ++span_count;
            }
        }
    }
    
    for (const auto& [stop, vertexes] : vertex_name_id_) {
                Edge<double> edge;
                edge.is_bus = false;
                edge.bus_name = stop;
                edge.from = vertexes[0];
                edge.to = vertexes[1];
                edge.weight = bus_wait_time_;
                graph_.AddEdge(edge);
    }

    router_ = std::make_unique<Router<double>>(graph_);
}

std::optional<std::pair<json::Array, double>> graph::TransportRouter::FindNearestWay(const std::string& from, const std::string& to) const {
    if (!vertex_name_id_.count(from) || !vertex_name_id_.count(to)) return std::nullopt;
    auto road = router_->BuildRoute(vertex_name_id_.at(from).at(0), vertex_name_id_.at(to).at(0));
    if (!road) return std::nullopt;

    json::Array items;
    auto vertexes = road.value();
    for (size_t i = 0; i < vertexes.edges.size(); ++i) {
        json::Dict node;
        auto edge = graph_.GetEdge(vertexes.edges[i]);


        if (!edge.is_bus) {
            node["type"] = json::Node("Wait");
            node["time"] = bus_wait_time_;
            node["stop_name"] = vertex_stop_.at(edge.from);

            items.push_back(json::Node(move(node)));
        }


        if (edge.is_bus) {
            node["type"] = json::Node("Bus");
            node["span_count"] = json::Node(edge.span_count);
            node["bus"] = json::Node(edge.bus_name);
            node["time"] = edge.weight;

            items.push_back(json::Node(move(node)));
        }



    }
    std::optional<std::pair<json::Array, double>> answer({move(items), road.value().weight});
    return answer;
}
