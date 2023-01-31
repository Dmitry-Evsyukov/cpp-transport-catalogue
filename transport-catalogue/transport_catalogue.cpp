#include "transport_catalogue.h"
#include <algorithm>
namespace transport_manager {
    using namespace std;

    void TransportManager::AddStop(const std::string &name, const double latitude, const double longitude) {
        stops_.push_front({name, latitude, longitude, stops_.size()});
        stop_buses_[stops_[0].name];
        stop_pointers_[stops_[0].name] = &stops_[0];
    }

    void TransportManager::AddStopLength(const std::string &name, std::vector<std::pair<std::string, int>> &stops_length) {
        for (const auto &[stop, length]: stops_length) {
            stop_to_stop_length_[stop_pointers_[name]->name].insert({stop_pointers_[stop]->name, length});
        }

    }

    void TransportManager::AddBus(const string_view number, const std::vector<std::string> &stops) {

        buses_.push_front({string(number), buses_.size()});
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
            if (stop_pointers_.count(name)) {
                answer.flag = true;
            }
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


transport_catalogue_serialization::TransportManager transport_manager::TransportManager::Serialize() const {

    transport_catalogue_serialization::TransportManager transport_catalogue_serialize;
    int x = 0;
    int stop_to_stop_l_index = 0;
    for (const auto& stop : stops_) {

        transport_catalogue_serialization::Stop stop_serialize;
        stop_serialize.set_name(stop.name);
        stop_serialize.set_latitude(stop.latitude);
        stop_serialize.set_longitude(stop.longitude);

        transport_catalogue_serialize.add_stops();
        *transport_catalogue_serialize.mutable_stops(x) = stop_serialize;

        transport_catalogue_serialization::StopToBuses stop_to_buses;
        stop_to_buses.set_stop_index(stop.index);

        for (const auto& bus : stop_buses_.at(stop.name)) {
            stop_to_buses.add_bus_indexes(bus_pointers_.at(bus)->index);
        }
        transport_catalogue_serialize.add_stop_to_buses();
        *transport_catalogue_serialize.mutable_stop_to_buses(x) = stop_to_buses;

        if (stop_to_stop_length_.count(stop.name)) {

            transport_catalogue_serialization::StopToStopsLength stops_to_stops_length;
            stops_to_stops_length.set_stop_index(stop.index);
            int y = 0;
            for (const auto &[s, l]: stop_to_stop_length_.at(stop.name)) {
                transport_catalogue_serialization::StopLengthPair stop_length_pair;
                stop_length_pair.set_stop_index(stop_pointers_.at(s)->index);
                stop_length_pair.set_length(l);
                stops_to_stops_length.add_stop_length();
                *stops_to_stops_length.mutable_stop_length(y) = stop_length_pair;
                ++y;
            }
            transport_catalogue_serialize.add_stop_to_stops_length();
            *transport_catalogue_serialize.mutable_stop_to_stops_length(stop_to_stop_l_index) = stops_to_stops_length;
            ++stop_to_stop_l_index;
        }
        ++x;
    }
    x = 0;
    for (const auto& bus : buses_) {
        transport_catalogue_serialize.add_buses(bus.number);

        transport_catalogue_serialization::BusToStops bus_to_stops;
        bus_to_stops.set_bus_index(bus_pointers_.at(bus.number)->index);
        for (const auto& stop : bus_stops_.at(bus.number)) {
            bus_to_stops.add_stop_index(stop->index);
        }
        transport_catalogue_serialize.add_bus_to_stops();
        *transport_catalogue_serialize.mutable_bus_to_stops(x) = bus_to_stops;
        ++x;
    }

    return transport_catalogue_serialize;
}

 transport_manager::TransportManager transport_manager::TransportManager::DeserializeTM(
         const transport_catalogue_serialization::TransportManager &transport_catalogue) {
    transport_manager::TransportManager t_m;

    for (int i = 0; i < transport_catalogue.buses_size(); ++i) {
        t_m.buses_.push_front({transport_catalogue.buses()[i], static_cast<size_t>(i)});
        t_m.bus_pointers_[t_m.buses_[0].number] = &t_m.buses_[0];
    }

    for (int i = 0; i < transport_catalogue.stops_size(); ++i) {
        auto stop = transport_catalogue.stops(i);
        t_m.stops_.push_front({stop.name(), stop.latitude(), stop.longitude(), static_cast<size_t>(i)});
        t_m.stop_pointers_[t_m.stops_[0].name] = &t_m.stops_[0];
    }

    for (const auto& stop_to_buses : transport_catalogue.stop_to_buses()) {
        for (const auto& bus_index : stop_to_buses.bus_indexes()) {
            t_m.stop_buses_[t_m.stops_[stop_to_buses.stop_index()].name].insert(t_m.buses_[bus_index].number);
        }
    }


    for (const auto& bus_to_stops : transport_catalogue.bus_to_stops()) {
        for (const auto& stop_index : bus_to_stops.stop_index()) {
            t_m.bus_stops_[t_m.buses_[bus_to_stops.bus_index()].number].push_back(t_m.stop_pointers_.at(t_m.stops_[stop_index].name));
        }
    }

    for (const auto& stop_to_stop_l : transport_catalogue.stop_to_stops_length()) {
        for (const auto& s_l : stop_to_stop_l.stop_length()) {
            t_m.stop_to_stop_length_[t_m.stops_[stop_to_stop_l.stop_index()].name][t_m.stops_[s_l.stop_index()].name] = s_l.length();
        }
    }

    return move(t_m);
}