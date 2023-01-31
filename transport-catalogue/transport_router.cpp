#include "transport_router.h"
#include <memory>

const int MINUTES_IN_HOUR = 60;
const int METERS_IN_KILOMETR = 1000;
graph::TransportRouter::TransportRouter(
        std::vector<std::pair<std::string, std::vector<std::string>>> buses,
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_distances,
        int bus_wait_time, double bus_velocity, size_t vertex_count, std::vector<std::string> stops) : buses_(move(buses)), stop_distances_(move(stop_distances)), graph_(vertex_count), stops_(stops) {
    bus_velocity_ = bus_velocity;
    bus_wait_time_ = bus_wait_time;
}

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

    router_ = std::make_unique<Router<double>>(&graph_, true);
}

std::optional<std::pair<json::Array, double>> graph::TransportRouter::FindNearestWay(const std::string& from, const std::string& to) const {
    if (!vertex_name_id_.count(from) || !vertex_name_id_.count(to)) return std::nullopt;
    json::Array items;
    if (from == to) {return std::optional<std::pair<json::Array, double>> ({move(items), 0}); };
    auto road = router_->BuildRoute(vertex_name_id_.at(from).at(0), vertex_name_id_.at(to).at(0));
    if (!road) return std::nullopt;



    auto vertexes = road.value();
    for (size_t i = 0; i < vertexes.edges.size(); ++i) {
        json::Dict node;
        auto edge = graph_.GetEdge(vertexes.edges[i]);

        using namespace std::literals;
        if (!edge.is_bus) {
            node["type"] = json::Node("Wait"s);
            node["time"] = bus_wait_time_;
            node["stop_name"] = vertex_stop_.at(edge.from);

            items.push_back(json::Node(move(node)));
        }


        if (edge.is_bus) {
            node["type"] = json::Node(std::string("Bus")).AsString();
            node["span_count"] = json::Node(edge.span_count);
            node["bus"] = json::Node(edge.bus_name);
            node["time"] = edge.weight;

            items.push_back(json::Node(move(node)));
        }



    }
    std::optional<std::pair<json::Array, double>> answer({move(items), road.value().weight});
    return answer;
}

void graph::TransportRouter::SetNewGraph() {
    router_->SetGraph(&graph_);
}

transport_router_serialization::TransportRouter graph::TransportRouter::Serialize() const {
    transport_router_serialization::TransportRouter t_r_ser;
    *t_r_ser.mutable_graph() = graph_.Serialize();
    *t_r_ser.mutable_router() = router_->Serialize();
    t_r_ser.set_bus_velocity(bus_velocity_);
    t_r_ser.set_bus_wait_time(bus_wait_time_);

    int x = 0;
    for (const auto& [stop_name, ids] : vertex_name_id_) {
        transport_router_serialization::StopName_Id stop_name_id;
        stop_name_id.set_stop(stop_name);
        for (const auto& id : ids) {
            stop_name_id.add_vertex_id(id);
        }
        t_r_ser.add_vertex_name_id();
        *t_r_ser.mutable_vertex_name_id(x) = stop_name_id;
        ++x;
    }

    x = 0;
    for (const auto& [id, stop_name] : vertex_stop_) {
        transport_router_serialization::IdName vertex_stop;
        vertex_stop.set_name(stop_name);
        vertex_stop.set_vertex_id(id);

        t_r_ser.add_vertex_stop();
        *t_r_ser.mutable_vertex_stop(x) = vertex_stop;
        ++x;
    }


    return std::move(t_r_ser);
}

graph::TransportRouter graph::TransportRouter::DeserializeTransportRouter(const transport_router_serialization::TransportRouter &t_r_ser) {
    graph::TransportRouter t_r;
    graph::DirectedWeightedGraph<double> graph = graph::DirectedWeightedGraph<double>::DeserializeGraph(t_r_ser.graph());
    t_r.graph_ = std::move(graph);
    auto router = graph::Router<double>::DeserializeRouter(t_r_ser.router(), &t_r.graph_);

    t_r.router_ = std::make_unique<Router<double>>(router);

    t_r.bus_wait_time_ = t_r_ser.bus_wait_time();
    t_r.bus_velocity_ = t_r_ser.bus_velocity();

    for (const auto& vert_n_ids : t_r_ser.vertex_name_id()) {
        for (const auto & id : vert_n_ids.vertex_id()) {
            t_r.vertex_name_id_[vert_n_ids.stop()].push_back(id);
        }
    }

    for (const auto& vertex_id__stop : t_r_ser.vertex_stop()) {
        t_r.vertex_stop_[vertex_id__stop.vertex_id()] = vertex_id__stop.name();
    }
    return std::move(t_r);
}