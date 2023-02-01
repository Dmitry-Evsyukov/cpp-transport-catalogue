#pragma once
#include "geo.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include "serialization.h"

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

RenderSet ParseRenderSet(const json::Dict& node);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding) : padding_(padding){
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }
    SphereProjector(double pad, double min_lon, double max_lat, double coeff) : padding_(pad), min_lon_(min_lon), max_lat_(max_lat), zoom_coeff_(coeff) {}

    sphere_projector_ser::SphereProjector SerializeSP() const;
    static SphereProjector DeserializeSPFromIstream(const sphere_projector_ser::SphereProjector& s_p);

    svg::Point operator()(transport_manager::compute_length::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class RendererMap {
public:
    RendererMap(const transport_manager::TransportManager& transport_manager_,
                const std::vector<BusStops>&  buses_,
                const std::vector<std::string>& stops_,
                const RenderSet& render_set_,
                const SphereProjector& projector_) : transport_manager(transport_manager_), buses(buses_), stops(stops_), render_set(render_set_), projector(projector_) {}
    svg::Document operator()();

private:
    const transport_manager::TransportManager& transport_manager;
    const std::vector<BusStops>& buses;
    const std::vector<std::string>& stops;
    const RenderSet& render_set;
    const SphereProjector& projector;
};
