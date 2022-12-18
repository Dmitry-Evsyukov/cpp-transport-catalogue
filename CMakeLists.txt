cmake_minimum_required(VERSION 3.22)
project(12_sprint)

set(CMAKE_CXX_STANDARD 17)

include_directories(transport-catalogue)

add_executable(12_sprint
        transport-catalogue/geo.h
        transport-catalogue/json.cpp
        transport-catalogue/json.h
        transport-catalogue/json_builder.cpp
        transport-catalogue/json_builder.h
        transport-catalogue/json_reader.cpp
        transport-catalogue/json_reader.h
        transport-catalogue/main.cpp
        transport-catalogue/map_renderer.cpp
        transport-catalogue/map_renderer.h
        transport-catalogue/svg.cpp
        transport-catalogue/svg.h
        transport-catalogue/transport_catalogue.cpp
        transport-catalogue/transport_catalogue.h)
