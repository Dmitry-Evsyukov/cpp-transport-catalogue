#include <sstream>
#include "json_reader.h"
#include "json.h"
#include "json_builder.h"
#include "transport_router.h"

using namespace std;
namespace transport_manager::parse_query {
    std::tuple<std::string, double, double> ParseStop(const json::Node &stop_json) {
        const auto &dict = stop_json.AsDict();
        return tuple(dict.at("name").AsString(), dict.at("longitude").AsDouble(), dict.at("latitude").AsDouble());
    }

    std::pair<std::string, std::vector<pair<std::string, int>>> ParseStopLength(const json::Node &stop_json) {
        const string &name = stop_json.AsDict().at("name").AsString();
        const auto &stops_length = stop_json.AsDict().at("road_distances").AsDict();
        vector<pair<string, int>> stops_response;
        for (const auto &[other_stop, length]: stops_length) {
            stops_response.push_back({other_stop, length.AsInt()});
        }
        return {name, stops_response};
    }


    std::pair<string, std::vector<std::string>> ParseBus(const json::Node &bus_json) {
        vector<string> stops;
        for (const auto &node: bus_json.AsDict().at("stops").AsArray()) {
            stops.push_back(node.AsString());
        }
        if (!bus_json.AsDict().at("is_roundtrip").AsBool()) {
            vector<string> stops_ = stops;
            for (auto iter = stops_.rbegin() + 1; iter != stops_.rend(); ++iter) {
                stops.push_back(move(*iter));
            }
        }
        return pair(bus_json.AsDict().at("name").AsString(), stops);
    }
}
namespace transport_manager::output {
    void PrintJSONQueries(const json::Array& stat_requests, RendererMap& renderer, const TransportManager& transport_manager,  graph::TransportRouter& transport_router) {
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
            } else if (query.AsDict().at("type").AsString() == "Map") {
                auto doc = renderer();
                ostringstream os;
                doc.Render(os);
                auto file = os.str();
                node["map"] = json::Node(file);
            } else {
                if (!transport_router.IsInitialized()) transport_router.Initialize();
                auto answer = transport_router.FindNearestWay(query.AsDict().at("from").AsString(), query.AsDict().at("to").AsString());
                if (!answer) {
                    node["error_message"] = json::Node("not found");
                } else {
                    node["total_time"] = json::Node(answer.value().second);
                    node["items"] = json::Node(answer.value().first);
                }
            }
            root.push_back(json::Node(node));
        }
        json::Print(json::Document(json::Builder{}.Value(root).Build()), cout);
    }
}
