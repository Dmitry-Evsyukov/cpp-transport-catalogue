#include <iomanip>
#include "stat_reader.h"

namespace transport_manager::output {
    using namespace std;

    std::ostream &operator<<(std::ostream &out, const BusResponse &response) {
        out << "Bus " << response.bus_number << ": ";
        if (response.route_length == -1) {
            out << "not found" << endl;
            return out;
        }
        out << response.stops_on_route << " stops on route, "
            << setprecision(6) << response.unique_stops << " unique stops, "
            << setprecision(6) << response.route_length << " route length, "
            << setprecision(6) << response.courvature << " curvature" << endl;

        return out;
    }

    std::ostream &operator<<(std::ostream &out, const StopResponse &response) {
        out << "Stop " << response.name << ": ";
        if (response.flag == false) {
            out << "not found" << endl;
            return out;
        }
        if (response.buses.size() == 0) {
            out << "no buses" << endl;
            return out;
        }
        out << "buses";
        for (const auto &bus: response.buses) {
            out << " "s << bus;
        }
        out << endl;
        return out;
    }

    void ResponseAllQueries(std::istream& in, std::ostream& out, transport_manager::TransportManager& transport_manager) {
        int query_count;
        std::string line;
        in >> query_count;
        getline(in, line);

        using namespace transport_manager::output;
        for (int i = 0; i < query_count; ++i) {
            getline(in, line);
            if (line[0] == 'B') {
                line = line.substr(4);
                out << transport_manager.GetBus(line);
            } else {
                line = line.substr(5);
                out << transport_manager.GetStop(line);
            }
        }
    }
}
