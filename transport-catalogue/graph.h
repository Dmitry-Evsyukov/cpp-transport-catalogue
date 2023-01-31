#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>
#include <graph.pb.h>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
    int span_count;
    bool is_bus = true;
    std::string bus_name;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;

    explicit DirectedWeightedGraph(size_t vertex_count);
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    transport_router_serialization::Graph Serialize() const {
        transport_router_serialization::Graph graph_serialized;
        int x = 0;
        for (const auto& i_l : incidence_lists_) {
            transport_router_serialization::IncidenceList incidence_list;
            for (const uint32_t& edge_id : i_l) {
                incidence_list.add_edge_id(edge_id);
            }

            graph_serialized.add_incidence_lists();
            *graph_serialized.mutable_incidence_lists(x) = incidence_list;
            ++x;
        }
        x = 0;
        for (const auto& edge : edges_) {
            transport_router_serialization::Edge edge_ser;

            edge_ser.set_from(edge.from);
            edge_ser.set_to(edge.to);
            edge_ser.set_weight(edge.weight);
            edge_ser.set_span_count(edge.span_count);
            edge_ser.set_is_bus(edge.is_bus);
            edge_ser.set_bus_name(edge.bus_name);

            graph_serialized.add_edges();
            *graph_serialized.mutable_edges(x) = edge_ser;
            ++x;
        }
        return graph_serialized;
    }

    static  DirectedWeightedGraph<double> DeserializeGraph(const transport_router_serialization::Graph& graph_ser) {
        DirectedWeightedGraph<double> graph;
        graph.incidence_lists_.clear();
        graph.edges_.clear();
        for (const auto& incid_l_ser : graph_ser.incidence_lists()) {
            IncidenceList incid_l;
            for (const auto& edge_id : incid_l_ser.edge_id()) {
                incid_l.push_back(edge_id);
            }
            graph.incidence_lists_.push_back(std::move(incid_l));
        }

        for (const auto& edge_ser : graph_ser.edges()) {
            Edge<double> edge;

            edge.weight = edge_ser.weight();
            edge.bus_name = edge_ser.bus_name();
            edge.is_bus = edge_ser.is_bus();
            edge.span_count = edge_ser.span_count();
            edge.to = edge_ser.to();
            edge.from = edge_ser.from();

            graph.edges_.push_back(std::move(edge));
        }
        return std::move(graph);
    }

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}
}  // namespace graph