#include "svg.h"
#include <iostream>

namespace svg {

    using namespace std::literals;


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point = {}) {
        points_.push_back(point);
        return *this;
    }
    void Polyline::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool flag = false;
        for (const auto& point : points_) {
            if (!flag) {
                flag = true;
                out << point.x << ',' << point.y;
            } else {
                out << ' ' << point.x << ',' << point.y;
            }

        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << " />"sv;
    }

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }
    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }
    void Text::RenderObject(const RenderContext &context) const {
        auto& out = context.out;
        out << "<text x=\"" << position_.x << "\" y=\"" << position_.y << "\" dx=\""
            << offset_.x <<  "\" dy=\"" << offset_.y
            << "\" font-size=\"" << font_size_ <<"\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        RenderAttrs(context.out);
        out << ">";
        for (const char c : data_) {
            if (c == '"') {
                out << "&quot;";
            } else if (c == '\'') {
                out << "&apos;";
            } else if (c == '<') {
                out << "&lt;";
            } else if (c == '>') {
                out << "&gt;";
            } else if (c == '&') {
                out << "&amp;";
            } else {
                out << c;
            }
        }
        out << "</text>";
    }

    void Document::Render(std::ostream &out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
        RenderContext context(out, 2, 2);
        for (const auto& object : pointers_) {
            object->Render(context);
        }
        out << "</svg>" << std::endl;

    }
}  // namespace svg
