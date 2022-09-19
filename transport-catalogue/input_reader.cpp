#include "input_reader.h"
#include <iostream>

using namespace std;
namespace transport_manager::parse_query {
    ParseStructStop ParseStop(std::string_view line) {
        ParseStructStop answer;
        line.remove_prefix(5);
        string name = string(line.substr(0,line.find(':')));
        line.remove_prefix(name.size() + 2);
        string latitude = string(line.substr(0,line.find(',')));
        line.remove_prefix(latitude.size() + 2);
        if (line.find(',') != line.npos) {
            string longitude = string(line.substr(0,line.find(',')));

            return answer;
        }
        string longitude = string(line);
        answer.name = name;
        answer.latitude = atof(latitude.c_str());
        answer.longitude = atof(longitude.c_str());
        return answer;
    }
    ParseStructBus ParseBus(std::string_view line) {
        ParseStructBus answer;
        line.remove_prefix(4);
        string bus = string(line.substr(0, line.find(':')));
        line.remove_prefix(bus.size() + 2);
        vector<string> stops;
        if (line.find('>') != line.npos) {
            while (line.find('>') != line.npos) {
                stops.push_back(string(line.substr(0,line.find('>') - 1)));
                line.remove_prefix(stops[stops.size() - 1].size() + 3);
            }
            stops.push_back(string(line));
            answer.name = move(bus);
            answer.stops = move(stops);
            return answer;
        }

        while (line.find('-') != line.npos) {
            stops.push_back(string(line.substr(0, line.find('-') - 1)));
            line.remove_prefix(stops[stops.size() - 1].size() + 3);
        }
        stops.push_back(string(line));

        vector<string> stops_ = stops;
        for (auto iter = stops_.rbegin() + 1; iter != stops_.rend(); ++iter) {
            stops.push_back(move(*iter));
        }
        answer.name = move(bus);
        answer.stops = move(stops);
        return answer;
    }


    ParseStructStopLength ParseStopLength(std::string_view line) {
        line.remove_prefix(5);
        string name = string(line.substr(0,line.find(':')));
        line.remove_prefix(name.size() + 2);
        line.remove_prefix(line.find(',') + 2);
        line.remove_prefix(line.find(',') + 2);
        vector<pair<string, int>> stops_length;
        while (line.find('m') != line.npos) {
            string number = string(line.substr(0, line.find('m')));
            int num = atoi(number.c_str());
            line.remove_prefix(number.size() + 5);
            if (line.find(',') == line.npos) {
                string stop = string(line);
                stops_length.push_back({stop, num});
                break;
            }
            string stop = string(line.substr(0, line.find(',')));
            line.remove_prefix(stop.size() + 2);
            stops_length.push_back({stop, num});
        }
        return {name, stops_length};
    }

    transport_manager::TransportManager ParseQueries(std::istream& input) {
        int query_count;

        input >> query_count;

        string line;
        getline(input, line);

        vector<string> stops_q;
        vector<string> bus_q;

        for (int i = 0; i < query_count; ++i) {
            getline(input, line);
            if (line[0] == 'B') {
                bus_q.push_back(move(line));
            } else {
                stops_q.push_back(move(line));
            }
        }

        transport_manager::TransportManager transport_manager;
        for (const auto& q : stops_q) {
            auto stop = transport_manager::parse_query::ParseStop(q);
            transport_manager.AddStop(stop.name, stop.latitude , stop.longitude);
        }

        for (const auto& q : stops_q) {
            if (q.find("m to ") == q.npos) {
                continue;
            }
            auto [name, stop_length] = transport_manager::parse_query::ParseStopLength(q);
            transport_manager.AddStopLength(name, stop_length);
        }

        for (const auto q : bus_q) {
            auto [bus, stops] = transport_manager::parse_query::ParseBus(q);
            transport_manager.AddBus(bus, stops);
        }
        return transport_manager;
    }
}


