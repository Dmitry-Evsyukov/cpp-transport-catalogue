#include <iostream>
#include <sstream>
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;


int main() {
    auto transport_manager = transport_manager::parse_query::ParseQueries(cin);
    transport_manager::output::ResponseAllQueries(cin, cout, transport_manager);

    return 0;
}
