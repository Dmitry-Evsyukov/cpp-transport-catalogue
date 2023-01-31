#include "request_handler.h"
#include "json_builder.h"
#include <sstream>
#include <fstream>
#include "serialization.h"
using namespace std;

namespace transport_manager::output {

    void PrintJSONQueries(const json::Array& stat_requests, const TransportManager& transport_manager, RendererMap& renderer, graph::TransportRouter& transport_router){
        vector<json::Node> root;
        json::Builder builder;
        builder.StartArray();
        for (const auto& query : stat_requests) {
            json::Dict node;
            node["request_id"] = query.AsDict().at("id");
            if (query.AsDict().at("type").AsString() == "Stop") {

                const auto& answer = transport_manager.GetStop(query.AsDict().at("name").AsString());
                if (answer.flag == false) {
                    node["error_message"] = json::Node("not found"s);
                    root.push_back(json::Node(node));
                    continue;
                }

                vector<json::Node> buses;
                for (const auto& bus : answer.buses) {
                    buses.push_back(json::Node(bus));
                }
                node["buses"] = json::Node(buses);


            } else if (query.AsDict().at("type").AsString() == "Bus") {
                const auto& answer = transport_manager.GetBus(query.AsDict().at("name").AsString());
                if (answer.route_length == -1) {
                    node["error_message"] = json::Node("not found"s);
                    root.push_back(json::Node(node));
                    continue;
                }
                node["curvature"] = json::Node(answer.courvature);
                node["route_length"] = json::Node(answer.route_length);
                node["stop_count"] = json::Node(answer.stops_on_route);
                node["unique_stop_count"] = json::Node(answer.unique_stops);
            } else if (query.AsDict().at("type").AsString() == "Map"){
                auto doc = renderer();
                ostringstream os;
                doc.Render(os);
                auto file = os.str();
                node["map"] = json::Node(file);
            } else {
                //if (!transport_router.IsInitialized()) transport_router.Initialize();
                auto answer = transport_router.FindNearestWay(query.AsDict().at("from").AsString(), query.AsDict().at("to").AsString());
                if (!answer) {

                    node["error_message"] = json::Node("not found"s);
                } else {
                    node["total_time"] = json::Node(answer.value().second);
                    node["items"] = json::Node(answer.value().first);
                }
            }
            root.push_back(json::Node(node));
        }
        json::Print(json::Document(json::Builder{}.Value(root).Build()), cout);
    }

    int ProcessRequests() {
        using namespace json;
        const auto dict = Load(cin).GetRoot().AsDict();
        const auto stat_requests = dict.at("stat_requests").AsArray();
        const auto serialization_settings = dict.at("serialization_settings").AsDict();

        const auto filename = serialization_settings.at("file").AsString();

        ifstream in_file(filename, ios::binary);
        if (!in_file) {
            return -1;
        }

        transport_catalogue_serialization::TransportCatalogue transport_catalogue;
        transport_catalogue.ParseFromIstream(&in_file);
        auto t_m = transport_manager::TransportManager::DeserializeTM(transport_catalogue.t_m());
        auto buses = DeSerializeAllBuses(transport_catalogue.buses());
        auto stops = DeSerializeStops(transport_catalogue.stops());
        auto render_set = DeserializeRenderSet(transport_catalogue.r_s());
        auto s_p = SphereProjector::DeserializeSPFromIstream(transport_catalogue.s_p());
        auto transport_router = graph::TransportRouter::DeserializeTransportRouter(transport_catalogue.transport_router());

        transport_router.SetStops(stops);
        RendererMap renderer(t_m, buses, stops, render_set, s_p);

        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_distances = t_m.GetStSLength();
        transport_router.SetNewGraph();
        PrintJSONQueries(stat_requests, t_m, renderer, transport_router);
        return 0;
    }
}