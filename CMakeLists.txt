cmake_minimum_required(VERSION 3.22)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

add_executable( transport-catalogue
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
        transport-catalogue/transport_catalogue.h
        transport-catalogue/graph.h
        transport-catalogue/router.h
        transport-catalogue/transport_router.h
        transport-catalogue/transport_router.cpp
    )
