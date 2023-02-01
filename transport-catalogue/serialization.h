#pragma once
#include "transport_router.h"
#include <map_renderer.pb.h>
#include "svg.h"
#include <svg.pb.h>
#include "transport_catalogue.h"
struct BusStops {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};

struct RenderSet {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    double bus_label_font_size = 0;
    svg::Point bus_label_offset;
    double stop_label_font_size = 0;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> colors;
};

renderer_set_serialization::RenderSet SerializeRenderSet(const RenderSet& render_set);

RenderSet DeserializeRenderSet(const renderer_set_serialization::RenderSet& r_s);

sphere_projector_ser::Stops SerializeStops(const json::Array& stops);
sphere_projector_ser::AllBuses SerializeAllBuses(const json::Array& buses);


std::vector<std::string> DeSerializeStops(const sphere_projector_ser::Stops& stops_ser);
std::vector<BusStops> DeSerializeAllBuses(const sphere_projector_ser::AllBuses& buses_ser);
