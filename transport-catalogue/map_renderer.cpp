#include "map_renderer.h"
#include "geo.h"

using namespace std;
RenderSet ParseRenderSet(const json::Dict& node) {
    RenderSet render_set;
    render_set.width = node.at("width").AsDouble();
    render_set.height = node.at("height").AsDouble();
    render_set.padding = node.at("padding").AsDouble();
    render_set.line_width = node.at("line_width").AsDouble();
    render_set.stop_radius = node.at("stop_radius").AsDouble();
    if (node.count("bus_label_font_size")) {
        render_set.bus_label_font_size = node.at("bus_label_font_size").AsDouble();
    }
    if (node.count("bus_label_offset")) {
        render_set.bus_label_offset.x = node.at("bus_label_offset").AsArray()[0].AsDouble();
        render_set.bus_label_offset.y = node.at("bus_label_offset").AsArray()[1].AsDouble();
    }
    if (node.count("stop_label_font_size")) {
        render_set.stop_label_font_size = node.at("stop_label_font_size").AsDouble();
    }
    if (node.count("stop_label_offset")) {
        render_set.stop_label_offset.x = node.at("stop_label_offset").AsArray()[0].AsDouble();
        render_set.stop_label_offset.y = node.at("stop_label_offset").AsArray()[1].AsDouble();
    }
    if (node.count("underlayer_color")) {
        if (node.at("underlayer_color").IsString()) {
            render_set.underlayer_color = node.at("underlayer_color").AsString();
        } else {
            if (node.at("underlayer_color").AsArray().size() == 3) {
                render_set.underlayer_color = svg::Rgb(node.at("underlayer_color").AsArray()[0].AsDouble(),
                                                       node.at("underlayer_color").AsArray()[1].AsDouble(),
                                                       node.at("underlayer_color").AsArray()[2].AsDouble());
            } else {
                render_set.underlayer_color = svg::Rgba(node.at("underlayer_color").AsArray()[0].AsDouble(),
                                                       node.at("underlayer_color").AsArray()[1].AsDouble(),
                                                       node.at("underlayer_color").AsArray()[2].AsDouble(),
                                                       node.at("underlayer_color").AsArray()[3].AsDouble());
            }
        }
    }
    if (node.count("underlayer_width")) {

        render_set.underlayer_width = node.at("underlayer_width").AsDouble();

    }
    auto& colors = node.at("color_palette").AsArray();
    for (const auto& color : colors) {
        svg::Color col;
        if (color.IsString()) {
            col = color.AsString();
        } else {
            if (color.AsArray().size() == 3) {
                col = svg::Rgb(color.AsArray()[0].AsDouble(),
                               color.AsArray()[1].AsDouble(),
                               color.AsArray()[2].AsDouble());
            } else {
                col = svg::Rgba(color.AsArray()[0].AsDouble(),
                                color.AsArray()[1].AsDouble(),
                                color.AsArray()[2].AsDouble(),
                                color.AsArray()[3].AsDouble());
            }
        }
        render_set.colors.push_back(col);
    }


    return render_set;
}

    svg::Point SphereProjector::operator()(transport_manager::compute_length::Coordinates coords) const {
        return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }


    svg::Document RendererMap::operator()() {
        svg::Document trips;

        int i = 0;

        for (const auto& node : buses) {
            svg::Polyline trip;
            vector<string> stops_on_that_trip;
            for (const auto &node: node.AsMap().at("stops").AsArray()) {
                stops_on_that_trip.push_back(node.AsString());
            }
            if (!node.AsMap().at("is_roundtrip").AsBool()) {
                vector<string> stops_ = stops_on_that_trip;
                for (auto iter = stops_.rbegin() + 1; iter != stops_.rend(); ++iter) {
                    stops_on_that_trip.push_back(move(*iter));
                }
            }
            if (stops_on_that_trip.empty()) continue;
            for (const auto& stop : stops_on_that_trip) {
                auto buffer = transport_manager.GetStop(stop);
                trip.AddPoint(projector({buffer.latitude, buffer.longitude}));
            }
            trip.SetFillColor("none").SetStrokeWidth(render_set.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetStrokeColor(render_set.colors[i]);
            ++i;
            if (i == render_set.colors.size()) i = 0;
            trips.Add(trip);
        }

        i = 0;
        for (const auto& node : buses) {
            auto stops_on_this_trip = node.AsMap().at("stops").AsArray();
            if (stops_on_this_trip.empty()) continue;

            svg::Text bus_name;
            svg::Text bus_name_dlc;
            auto stop = transport_manager.GetStop(stops_on_this_trip[0].AsString());
            bus_name.SetPosition(projector({stop.latitude, stop.longitude}))
                    .SetFillColor(render_set.colors[i])
                    .SetOffset(render_set.bus_label_offset)
                    .SetFontSize(render_set.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(node.AsMap().at("name").AsString());
            bus_name_dlc.SetPosition(projector({stop.latitude, stop.longitude}))
                    .SetFillColor(render_set.underlayer_color)
                    .SetStrokeColor(render_set.underlayer_color)
                    .SetOffset(render_set.bus_label_offset)
                    .SetFontSize(render_set.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold")
                    .SetData(node.AsMap().at("name").AsString())
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeWidth(render_set.underlayer_width);
            trips.Add(bus_name_dlc);
            trips.Add(bus_name);
            if (!node.AsMap().at("is_roundtrip").AsBool()) {
                auto end_stop_name = transport_manager.GetStop(stops_on_this_trip[stops_on_this_trip.size() - 1].AsString());
                if (end_stop_name.name != node.AsMap().at("stops").AsArray()[0].AsString()) {
                    bus_name.SetPosition(projector({end_stop_name.latitude, end_stop_name.longitude}));
                    bus_name_dlc.SetPosition(projector({end_stop_name.latitude, end_stop_name.longitude}));
                    trips.Add(bus_name_dlc);
                    trips.Add(bus_name);
                }

            }

            ++i;
            if (i == render_set.colors.size()) i = 0;
        }

        for (const auto& stop : stops) {
            svg::Circle circle;
            auto name = stop.AsMap().at("name").AsString();
            auto s = transport_manager.GetStop(name);
            if (s.buses.empty()) continue;
            circle.SetRadius(render_set.stop_radius).SetCenter(projector({s.latitude, s.longitude})).SetFillColor("white");
            trips.Add(circle);
        }

        for (const auto& stop : stops) {
            svg::Text text;
            svg::Text text_dlc;
            auto name = stop.AsMap().at("name").AsString();
            auto s = transport_manager.GetStop(name);
            if (s.buses.empty()) continue;
            text.SetPosition(projector({s.latitude, s.longitude}))
                    .SetData(name)
                    .SetOffset(render_set.stop_label_offset)
                    .SetFontSize(render_set.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFillColor("black");
            text_dlc.SetPosition(projector({s.latitude, s.longitude}))
                    .SetData(name)
                    .SetOffset(render_set.stop_label_offset)
                    .SetFontSize(render_set.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFillColor(render_set.underlayer_color)
                    .SetStrokeColor(render_set.underlayer_color)
                    .SetStrokeWidth(render_set.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            trips.Add(text_dlc);
            trips.Add(text);
        }
        return trips;
    }



