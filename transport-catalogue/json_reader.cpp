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

