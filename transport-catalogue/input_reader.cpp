#include "input_reader.h"

using namespace std;
namespace transport_manager::parse_query {
    std::tuple<std::string, double, double> ParseStop(std::string_view line) {
        line.remove_prefix(5);
        string name = string(line.substr(0,line.find(':')));
        line.remove_prefix(name.size() + 2);
        string latitude = string(line.substr(0,line.find(',')));
        line.remove_prefix(latitude.size() + 2);
        if (line.find(',') != line.npos) {
            string longitude = string(line.substr(0,line.find(',')));
            return tuple(name, atof(latitude.c_str()), atof(longitude.c_str()));
        }
        string longitude = string(line);
        return tuple(name, atof(latitude.c_str()), atof(longitude.c_str()));
    }
    std::pair<string, std::vector<std::string>> ParseBus(std::string_view line) {
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
            return pair(bus, stops);
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
        return pair(bus, stops);
    }


    std::pair<std::string, std::vector<pair<std::string, int>>> ParseStopLength(std::string_view line) {
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
}