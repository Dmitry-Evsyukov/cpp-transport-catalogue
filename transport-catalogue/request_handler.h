#include "json.h"
#include "map_renderer.h"
#include "graph.h"
#include "transport_router.h"


namespace transport_manager::output {
    int ProcessRequests();
    void PrintJSONQueries(const json::Array& stat_requests, const TransportManager& transport_manager, RendererMap& renderer, graph::TransportRouter& transport_router);
}