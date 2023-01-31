#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <transport_router.pb.h>

namespace graph {

template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    Router(Graph* gr) : graph_(gr){};
    explicit Router(Graph* graph, bool flag);

    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };
    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

    transport_router_serialization::Router Serialize() const{
        transport_router_serialization::Router router_ser;
        int y = 0;
        for (const std::vector<std::optional<RouteInternalData>>& routes_int_data : routes_internal_data_) {
            transport_router_serialization::RoutesInternalData routes_internal_data_ser;
            int x = 0;
            for (int i = 0; i < routes_int_data.size(); ++i) {
                transport_router_serialization::RoutesIntDataOptional routes_int_data_ser;
                if (routes_int_data[i]) {
                    routes_int_data_ser.set_weight(routes_int_data[i].value().weight);
                    if (routes_int_data[i].value().prev_edge) {
                        transport_router_serialization::EdgeId e_i_ser;
                        e_i_ser.set_edge_id(routes_int_data[i].value().prev_edge.value());
                        *routes_int_data_ser.mutable_prev_edge() = e_i_ser;
                    }

                }
                routes_internal_data_ser.add_routes_int_data();
                *routes_internal_data_ser.mutable_routes_int_data(x) = routes_int_data_ser;
                ++x;
            }
            router_ser.add_routes_internal_data();
            *router_ser.mutable_routes_internal_data(y) = routes_internal_data_ser;
            ++y;
        }
        return router_ser;
    }

    static Router<Weight> DeserializeRouter(const transport_router_serialization::Router& router_ser, graph::DirectedWeightedGraph<double>* gr) {
        Router<double> router(gr);
        for (const auto& routes_internal_data : router_ser.routes_internal_data()) {
            std::vector<std::optional<RouteInternalData>> rid_vec;
            for (const auto& routes_int_data : routes_internal_data.routes_int_data()) {
                RouteInternalData rid;
                rid.weight = routes_int_data.weight();
                if (routes_int_data.has_prev_edge()) {
                    rid.prev_edge = routes_int_data.prev_edge().edge_id();
                } else {
                    rid.prev_edge = std::nullopt;
                }
                rid_vec.push_back(rid);
            }
            router.routes_internal_data_.push_back(std::move(rid_vec));
        }
        return std::move(router);
    }
    void SetGraph(Graph* graph_ref) {
        graph_ = graph_ref;
    }
private:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    void InitializeRoutesInternalData(const Graph* graph) {
        const size_t vertex_count = graph->GetVertexCount();
        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};
            for (const EdgeId edge_id : graph->GetIncidentEdges(vertex)) {
                const auto& edge = graph->GetEdge(edge_id);
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }
                auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                if (!route_internal_data || route_internal_data->weight > edge.weight) {
                    route_internal_data = RouteInternalData{edge.weight, edge_id};
                }
            }
        }
    }

    void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from,
                    const RouteInternalData& route_to) {
        auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
        const Weight candidate_weight = route_from.weight + route_to.weight;
        if (!route_relaxing || candidate_weight < route_relaxing->weight) {
            route_relaxing = {candidate_weight,
                              route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
        }
    }




    void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                    if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                        RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                    }
                }
            }
        }
    }

    static constexpr Weight ZERO_WEIGHT{};
    Graph* graph_;
    RoutesInternalData routes_internal_data_;
};

template <typename Weight>
Router<Weight>::Router(Graph* graph, bool flag)
    : graph_(graph)
    , routes_internal_data_(graph->GetVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph->GetVertexCount()))
{
    InitializeRoutesInternalData(graph);

    const size_t vertex_count = graph->GetVertexCount();
    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
    }
}



template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                             VertexId to) const {
    const auto& route_internal_data = routes_internal_data_.at(from).at(to);
    if (!route_internal_data || route_internal_data->weight == 0 && !route_internal_data.value().prev_edge) {
        return std::nullopt;
    }
    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_[from][graph_->GetEdge(*edge_id).from]->prev_edge)
    {
        edges.push_back(*edge_id);
    }
    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

}  // namespace graph