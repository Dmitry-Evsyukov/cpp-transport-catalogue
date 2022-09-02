#pragma once
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <string>
#include "stat_reader.h"
#include "geo.h"

namespace transport_manager {
    struct Stop {
        std::string name;
        double latitude;
        double longitude;
    };

    struct Bus {
        std::string number;
    };


    class TransportManager {
    public:
        void AddStop(const std::string &name, const double latitude, const double longitude);

        void AddStopLength(const std::string &name, std::vector<std::pair<std::string, int>> &stops_length);

        void AddBus(const std::string_view number, const std::vector<std::string> &stops);

        output::BusResponse GetBus(const std::string_view number) const;

        output::StopResponse GetStop(const std::string_view name) const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        std::unordered_map<std::string_view, const Stop *> stop_pointers_;
        std::unordered_map<std::string_view, const Bus *> bus_pointers_;

        std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;
        std::unordered_map<std::string_view, std::vector<const Stop *>> bus_stops_;

        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_to_stop_length_;
    };
}