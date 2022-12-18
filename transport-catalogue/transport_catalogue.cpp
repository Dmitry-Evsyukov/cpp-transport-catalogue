#include "transport_catalogue.h"
#include <algorithm>
namespace transport_manager {
    using namespace std;

    void TransportManager::AddStop(const std::string &name, const double latitude, const double longitude) {
        stops_.push_front({name, latitude, longitude});
        stop_buses_[stops_[0].name];
        stop_pointers_[stops_[0].name] = &stops_[0];
    }

    void TransportManager::AddStopLength(const std::string &name, std::vector<std::pair<std::string, int>> &stops_length) {
        for (const auto &[stop, length]: stops_length) {
            stop_to_stop_length_[stop_pointers_[name]->name].insert({stop_pointers_[stop]->name, length});
        }

    }

    void TransportManager::AddBus(const string_view number, const std::vector<std::string> &stops) {

        buses_.push_front({string(number)});
        bus_pointers_[buses_[0].number] = &buses_[0];
        for (const auto stop: stops) {
            bus_stops_[buses_[0].number].push_back(stop_pointers_[stop]);
            stop_buses_[stop_pointers_[stop]->name].insert(buses_[0].number);
        }
    }

    transport_manager::BusResponse TransportManager::GetBus(const string_view number) const {
        BusResponse answer;
        answer.bus_number = number;
        if (bus_stops_.count(number) == 0) {
            answer.route_length = -1;
            return answer;
        }
        answer.stops_on_route = bus_stops_.at(number).size();
        vector<compute_length::Coordinates> coordinates;
        set<string_view> stops_once;
        for (const auto &stop: bus_stops_.at(number)) {
            stops_once.insert(stop_pointers_.at(stop->name)->name);
            coordinates.push_back({stop->latitude, stop->longitude});
        }

        answer.unique_stops = stops_once.size();
        double route_length_theory = 0;
        for (int i = 0; i < coordinates.size() - 1; ++i) {
            route_length_theory += std::abs(ComputeDistance(coordinates[i], coordinates[i + 1]));

            string this_station = bus_stops_.at(number).at(i)->name;
            string next_station = bus_stops_.at(number).at(i + 1)->name;

            if (stop_to_stop_length_.count(this_station) == 0) {
                answer.route_length += stop_to_stop_length_.at(next_station).at(this_station);
                continue;
            }

            if (stop_to_stop_length_.at(this_station).count(next_station) == 0) {
                answer.route_length += stop_to_stop_length_.at(next_station).at(this_station);
            } else {
                answer.route_length += stop_to_stop_length_.at(this_station).at(next_station);
            }
        }

        answer.courvature = static_cast<double>(answer.route_length) / route_length_theory;
        return answer;
    }

    StopResponse TransportManager::GetStop(const std::string_view name) const {
        StopResponse answer;
        answer.name = string(name);
        if (stop_buses_.count(name) == 0) {
            answer.flag = false;
            return answer;
        }
        answer.longitude = stop_pointers_.at(name)->longitude;
        answer.latitude = stop_pointers_.at(name)->latitude;
        answer.flag = true;
        for (const auto stop: stop_buses_.at(name)) {
            answer.buses.push_back(string(stop));
        }
        sort(answer.buses.begin(), answer.buses.end());
        return answer;
    }

    const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> &
    transport_manager::TransportManager::GetStSLength() const { return stop_to_stop_length_; }
}
