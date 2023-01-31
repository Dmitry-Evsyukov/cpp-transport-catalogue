#include <fstream>
#include <iostream>
#include <string_view>
#include "domain.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        if (transport_manager::input::MakeBase() != 0) {
            return 2;
        }

    } else if (mode == "process_requests"sv) {

        if (transport_manager::output::ProcessRequests() != 0) {
            return 3;
        }

    } else {
        PrintUsage();
        return 1;
    }
}