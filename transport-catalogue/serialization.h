#include "map_renderer.h"
#include "transport_router.h"
renderer_set_serialization::RenderSet SerializeRenderSet(const RenderSet& render_set);

RenderSet DeserializeRenderSet(const renderer_set_serialization::RenderSet& r_s);

sphere_projector_ser::Stops SerializeStops(const json::Array& stops);
sphere_projector_ser::AllBuses SerializeAllBuses(const json::Array& buses);


std::vector<std::string> DeSerializeStops(const sphere_projector_ser::Stops& stops_ser);
std::vector<BusStops> DeSerializeAllBuses(const sphere_projector_ser::AllBuses& buses_ser);

void SerializeTransportCatalogue(std::ostream& out, const transport_manager::TransportManager& t_m, const RenderSet& r_s, const SphereProjector& s_p, const json::Array& stops, const json::Array& buses, const graph::TransportRouter& router);

