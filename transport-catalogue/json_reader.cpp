#include <sstream>
#include "json_reader.h"
#include "json.h"

using namespace std;
namespace transport_manager::parse_query {
    std::tuple<std::string, double, double> ParseStop(const json::Node &stop_json) {
        const auto &dict = stop_json.AsMap();
        return tuple(dict.at("name").AsString(), dict.at("longitude").AsDouble(), dict.at("latitude").AsDouble());
    }

    std::pair<std::string, std::vector<pair<std::string, int>>> ParseStopLength(const json::Node &stop_json) {
        const string &name = stop_json.AsMap().at("name").AsString();
        const auto &stops_length = stop_json.AsMap().at("road_distances").AsMap();
        vector<pair<string, int>> stops_response;
        for (const auto &[other_stop, length]: stops_length) {
            stops_response.push_back({other_stop, length.AsInt()});
        }
        return {name, stops_response};
    }


    std::pair<string, std::vector<std::string>> ParseBus(const json::Node &bus_json) {
        vector<string> stops;
        for (const auto &node: bus_json.AsMap().at("stops").AsArray()) {
            stops.push_back(node.AsString());
        }
        if (!bus_json.AsMap().at("is_roundtrip").AsBool()) {
            vector<string> stops_ = stops;
            for (auto iter = stops_.rbegin() + 1; iter != stops_.rend(); ++iter) {
                stops.push_back(move(*iter));
            }
        }
        return pair(bus_json.AsMap().at("name").AsString(), stops);
    }
}
namespace transport_manager::output {
    void PrintJSONQueries(const json::Array& stat_requests, const transport_manager::TransportManager& transport_manager, const json::Array& buses_, const json::Array& stops, const RenderSet& render_set, const SphereProjector& projector) {
        vector<json::Node> root;
        for (const auto& query : stat_requests) {
            json::Dict node;
            node["request_id"] = query.AsMap().at("id");
            if (query.AsMap().at("type").AsString() == "Stop") {

                const auto& answer = transport_manager.GetStop(query.AsMap().at("name").AsString());
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


            } else if (query.AsMap().at("type").AsString() == "Bus") {
                const auto& answer = transport_manager.GetBus(query.AsMap().at("name").AsString());
                if (answer.route_length == -1) {
                    node["error_message"] = json::Node("not found"s);
                    root.push_back(json::Node(node));
                    continue;
                }
                node["curvature"] = json::Node(answer.courvature);
                node["route_length"] = json::Node(answer.route_length);
                node["stop_count"] = json::Node(answer.stops_on_route);
                node["unique_stop_count"] = json::Node(answer.unique_stops);
            } else {
                auto doc = RenderMap(transport_manager, buses_, stops, render_set, projector);
                ostringstream os;
                doc.Render(os);
                auto file = os.str();
                node["map"] = json::Node(file);
            }
            root.push_back(json::Node(node));
        }

        json::Document response(root);

        json::Print(response, cout);
    }
}
