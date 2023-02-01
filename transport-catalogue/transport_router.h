#pragma once

#include <memory>
#include "router.h"
#include "json.h"
#include "map_renderer.h"
namespace graph {
    class TransportRouter {
    public:
        TransportRouter(std::vector<std::pair<std::string, std::vector<std::string>>> buses,
                        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_distances,
                        int bus_wait_time, double bus_velocity, size_t vertex_count, std::vector<std::string> stops);

        TransportRouter() = default;

        void Initialize();


        std::optional<std::pair<json::Array, double>> FindNearestWay(const std::string& from, const std::string& to) const;
        void SetStops(std::vector<std::string> stops) {stops_ = std::move(stops);};
        void SetNewGraph();
        bool IsInitialized() const;
        transport_router_serialization::TransportRouter Serialize() const;
        static TransportRouter DeserializeTransportRouter(const transport_router_serialization::TransportRouter& t_r);
    private:
        std::unique_ptr<Router<double>> router_;
        std::vector<std::pair<std::string, std::vector<std::string>>> buses_;
        std::unordered_map<std::string_view , std::unordered_map<std::string_view , int>> stop_distances_;
        int bus_wait_time_;
        double bus_velocity_;
        DirectedWeightedGraph<double> graph_;

        std::vector<std::string> stops_;

        std::map<std::string, std::vector<VertexId>> vertex_name_id_;
        std::unordered_map<VertexId, std::string> vertex_stop_;
    };
}
