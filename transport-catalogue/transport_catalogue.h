#pragma once
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <deque>
#include <vector>
#include <string>
#include <transport_catalogue.pb.h>
#include "geo.h"

namespace transport_manager {
    struct Stop {
        std::string name;
        double latitude;
        double longitude;
        size_t index;
    };

    struct Bus {
        std::string number;
        size_t index;
    };

    struct BusResponse {
        std::string_view bus_number;
        int stops_on_route = 0;
        int unique_stops = 0;
        double route_length = 0;
        double courvature = 0;
    };

    struct StopResponse {
        std::vector<std::string> buses;
        bool flag;
        std::string name;
        double latitude;
        double longitude;
    };

    class TransportManager {
    public:
        void AddStop(const std::string &name, const double latitude, const double longitude);

        void AddStopLength(const std::string &name, std::vector<std::pair<std::string, int>> &stops_length);

        void AddBus(const std::string_view number, const std::vector<std::string> &stops);

        BusResponse GetBus(const std::string_view number) const;

        StopResponse GetStop(const std::string_view name) const;

        transport_catalogue_serialization::TransportManager Serialize() const;

        static transport_manager::TransportManager DeserializeTM(const transport_catalogue_serialization::TransportManager& transport_catalogue);

        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>>& GetStSLength() const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;

        int bus_wait_time;
        double bus_velocity;

        std::unordered_map<std::string_view, const Stop *> stop_pointers_;
        std::unordered_map<std::string_view, const Bus *> bus_pointers_;

        std::unordered_map<std::string_view, std::set<std::string_view>> stop_buses_;
        std::unordered_map<std::string_view, std::vector<const Stop *>> bus_stops_;

        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> stop_to_stop_length_;
    };
}

