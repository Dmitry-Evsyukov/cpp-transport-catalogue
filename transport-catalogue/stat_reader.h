#pragma once
#include <iostream>
#include <set>
#include <vector>
#include "transport_catalogue.h"

namespace transport_manager::output {

    std::ostream &operator<<(std::ostream &out, const BusResponse &response);

    std::ostream &operator<<(std::ostream &out, const StopResponse &response);

    void ResponseAllQueries(std::istream& in, std::ostream& out, transport_manager::TransportManager& transport_manager);
}
