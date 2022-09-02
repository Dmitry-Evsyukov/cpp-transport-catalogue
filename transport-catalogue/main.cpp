#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;


int main() {
    int query_count;
    cin >> query_count;

    string line;
    getline(cin, line);

    vector<string> stops_q;
    vector<string> bus_q;

    for (int i = 0; i < query_count; ++i) {
        getline(cin, line);
        if (line[0] == 'B') {
            bus_q.push_back(move(line));
        } else {
            stops_q.push_back(move(line));
        }
    }

    transport_manager::TransportManager transport_manager;
    for (const auto& q : stops_q) {
        auto [n, l, lo] = transport_manager::parse_query::ParseStop(q);
        transport_manager.AddStop(n, l , lo);
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

    cin >> query_count;
    getline(cin, line);

    using namespace transport_manager::output;
    for (int i = 0; i < query_count; ++i) {
        getline(cin, line);
        if (line[0] == 'B') {
            line = line.substr(4);
            cout << transport_manager.GetBus(line);
        } else {
            line = line.substr(5);
            cout << transport_manager.GetStop(line);
        }
    }
    return 0;
}
