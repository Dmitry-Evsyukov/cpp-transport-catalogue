#include "serialization.h"
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <sstream>
#include "transport_catalogue.h"
renderer_set_serialization::RenderSet SerializeRenderSet(const RenderSet& render_set){
    renderer_set_serialization::RenderSet render_set_serialization;
    render_set_serialization.set_width(render_set.width);
    render_set_serialization.set_height(render_set.height);
    render_set_serialization.set_padding(render_set.padding);
    render_set_serialization.set_line_width(render_set.line_width);
    render_set_serialization.set_stop_radius(render_set.stop_radius);
    render_set_serialization.set_bus_label_font_size(render_set.bus_label_font_size);
    renderer_set_serialization::Point bus_label_offset;
    bus_label_offset.set_x(render_set.bus_label_offset.x);
    bus_label_offset.set_y(render_set.bus_label_offset.y);
    *render_set_serialization.mutable_bus_label_offset() = bus_label_offset;
    render_set_serialization.set_stop_label_font_size(render_set.stop_label_font_size);
    renderer_set_serialization::Point stop_label_offset;
    stop_label_offset.set_x(render_set.stop_label_offset.x);
    stop_label_offset.set_y(render_set.stop_label_offset.y);
    *render_set_serialization.mutable_stop_label_offset() = stop_label_offset;
    render_set_serialization.set_underlayer_width(render_set.underlayer_width);

    std::ostringstream os_underlayer_color;

    os_underlayer_color << render_set.underlayer_color;
    render_set_serialization.set_underlayer_color(os_underlayer_color.str());


    for (const auto& color : render_set.colors) {
        std::ostringstream os;
        os << color;
        render_set_serialization.add_colors(os.str());
        os.clear();
    }
    return render_set_serialization;
}

RenderSet DeserializeRenderSet(const renderer_set_serialization::RenderSet& r_s) {
    RenderSet render_set;

    render_set.width = r_s.width();
    render_set.height = r_s.height();
    render_set.padding = r_s.padding();
    render_set.line_width = r_s.line_width();
    render_set.stop_radius = r_s.stop_radius();
    render_set.bus_label_font_size = r_s.bus_label_font_size();
    render_set.stop_label_font_size = r_s.stop_label_font_size();
    render_set.underlayer_color = r_s.underlayer_color();
    render_set.underlayer_width = r_s.underlayer_width();
    render_set.bus_label_offset.x = r_s.bus_label_offset().x();
    render_set.bus_label_offset.y = r_s.bus_label_offset().y();
    render_set.stop_label_offset.x = r_s.stop_label_offset().x();
    render_set.stop_label_offset.y = r_s.stop_label_offset().y();
    render_set.colors.clear();
    for (const auto& color : r_s.colors()) {
        render_set.colors.push_back(color);
    }
    return std::move(render_set);
}

sphere_projector_ser::Stops SerializeStops(const json::Array& stops) {
    sphere_projector_ser::Stops stops_ser;
    for (const auto& node : stops) {
        stops_ser.add_stops(node.AsDict().at("name").AsString());
    }
    return stops_ser;
}
sphere_projector_ser::AllBuses SerializeAllBuses(const json::Array& buses) {
    sphere_projector_ser::AllBuses all_buses_ser;

    int x = 0;
    for (const auto& node : buses) {
        sphere_projector_ser::Bus bus_ser;
        bus_ser.set_is_roundtrip(node.AsDict().at("is_roundtrip").AsBool());
        bus_ser.set_bus(node.AsDict().at("name").AsString());
        for (const auto& stop_name : node.AsDict().at("stops").AsArray()) {
            bus_ser.add_stops(stop_name.AsString());
        }

        all_buses_ser.add_buses();
        *all_buses_ser.mutable_buses(x) = bus_ser;
        ++x;
    }
    return all_buses_ser;

}


std::vector<std::string> DeSerializeStops(const sphere_projector_ser::Stops& stops_ser) {
    std::vector<std::string> stops;
    for (const auto& stop : stops_ser.stops()) {
        stops.push_back(std::move(stop));
    }
    return move(stops);
}

std::vector<BusStops> DeSerializeAllBuses(const sphere_projector_ser::AllBuses& buses_ser) {
    std::vector<BusStops> buses;
    for (const auto& bus_ser : buses_ser.buses()) {
        BusStops bus;
        bus.name = bus_ser.bus();
        bus.is_roundtrip = bus_ser.is_roundtrip();
        for (const auto& stop : bus_ser.stops()) {
            bus.stops.push_back(stop);
        }
        buses.push_back(std::move(bus));
    }
    return std::move(buses);
}

void SerializeTransportCatalogue(std::ostream& out, const transport_manager::TransportManager& t_m, const RenderSet& r_s, const SphereProjector& s_p, const json::Array& stops, const json::Array& buses, const graph::TransportRouter& transport_router){
    transport_catalogue_serialization::TransportCatalogue transport_catalogue;
    *transport_catalogue.mutable_t_m() = t_m.Serialize();
    *transport_catalogue.mutable_r_s() = SerializeRenderSet(r_s);
    *transport_catalogue.mutable_s_p() = s_p.SerializeSP();
    *transport_catalogue.mutable_stops() = SerializeStops(stops);
    *transport_catalogue.mutable_buses() = SerializeAllBuses(buses);
    *transport_catalogue.mutable_transport_router() = transport_router.Serialize();

    transport_catalogue.SerializeToOstream(&out);
}
