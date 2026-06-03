#include "g_graph.h"

#include <cmath>

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace {
constexpr int32_t ELLIPSE_SEGMENTS = 48;
constexpr double TAU = 6.28318530717958647692;
}

void GGraph::_bind_methods() {
    ClassDB::bind_method(D_METHOD("draw_rect_shape", "width", "height", "line_size", "line_color", "fill_color"), &GGraph::draw_rect_shape);
    ClassDB::bind_method(D_METHOD("draw_ellipse_shape", "width", "height", "line_size", "line_color", "fill_color"), &GGraph::draw_ellipse_shape);
    ClassDB::bind_method(D_METHOD("draw_polygon_shape", "width", "height", "points", "line_size", "line_color", "fill_color"), &GGraph::draw_polygon_shape);
    ClassDB::bind_method(D_METHOD("clear_shape"), &GGraph::clear_shape);
    ClassDB::bind_method(D_METHOD("get_shape_type"), &GGraph::get_shape_type);
    ClassDB::bind_method(D_METHOD("get_line_size"), &GGraph::get_line_size);
    ClassDB::bind_method(D_METHOD("get_line_color"), &GGraph::get_line_color);
    ClassDB::bind_method(D_METHOD("get_fill_color"), &GGraph::get_fill_color);
    ClassDB::bind_method(D_METHOD("get_point_count"), &GGraph::get_point_count);
    ClassDB::bind_method(D_METHOD("is_empty"), &GGraph::is_empty);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "shape_type"), "", "get_shape_type");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "line_size"), "", "get_line_size");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "line_color"), "", "get_line_color");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "fill_color"), "", "get_fill_color");
}

void GGraph::_draw() {
    if (shape_type == SHAPE_NONE) {
        return;
    }

    if (shape_type == SHAPE_RECT) {
        draw_rect(Rect2(Vector2(), get_size()), fill_color, true);
        if (line_size > 0) {
            draw_rect(Rect2(Vector2(), get_size()), line_color, false, line_size);
        }
        return;
    }

    PackedVector2Array draw_points = points;
    if (shape_type == SHAPE_ELLIPSE) {
        draw_points = make_ellipse_points();
    }

    if (draw_points.size() < 3) {
        return;
    }

    draw_colored_polygon(draw_points, fill_color);
    draw_outline_polygon(draw_points);
}

void GGraph::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GObject::setup_before_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 5)) {
        return;
    }

    const ShapeType type = static_cast<ShapeType>(p_buffer.read_byte());
    if (type == SHAPE_NONE) {
        clear_shape();
        return;
    }

    const int32_t read_line_size = p_buffer.read_int();
    const Color read_line_color = p_buffer.read_color();
    const Color read_fill_color = p_buffer.read_color();
    const bool rounded_rect = p_buffer.read_bool();
    if (rounded_rect) {
        for (int32_t i = 0; i < 4; i++) {
            p_buffer.read_float();
        }
    }

    if (type == SHAPE_RECT) {
        draw_rect_shape(get_size().x, get_size().y, read_line_size, read_line_color, read_fill_color);
    } else if (type == SHAPE_ELLIPSE) {
        draw_ellipse_shape(get_size().x, get_size().y, read_line_size, read_line_color, read_fill_color);
    } else if (type == SHAPE_POLYGON) {
        const int32_t point_count = p_buffer.read_short() / 2;
        PackedVector2Array read_points;
        read_points.resize(point_count);
        for (int32_t i = 0; i < point_count; i++) {
            read_points.set(i, Vector2(p_buffer.read_float(), p_buffer.read_float()));
        }
        draw_polygon_shape(get_size().x, get_size().y, read_points, read_line_size, read_line_color, read_fill_color);
    } else if (type == SHAPE_REGULAR_POLYGON) {
        const int32_t sides = p_buffer.read_short();
        const float start_angle = p_buffer.read_float();
        const int32_t distance_count = p_buffer.read_short();
        Vector<float> distances;
        distances.resize(distance_count);
        for (int32_t i = 0; i < distance_count; i++) {
            distances.set(i, p_buffer.read_float());
        }

        shape_type = SHAPE_REGULAR_POLYGON;
        line_size = read_line_size;
        line_color = read_line_color;
        fill_color = read_fill_color;
        points = make_regular_polygon_points(sides, start_angle, distances);
        queue_redraw();
    }
}

void GGraph::draw_rect_shape(float p_width, float p_height, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color) {
    set_size(Vector2(p_width, p_height));
    shape_type = SHAPE_RECT;
    line_size = p_line_size;
    line_color = p_line_color;
    fill_color = p_fill_color;
    points.clear();
    queue_redraw();
}

void GGraph::draw_ellipse_shape(float p_width, float p_height, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color) {
    set_size(Vector2(p_width, p_height));
    shape_type = SHAPE_ELLIPSE;
    line_size = p_line_size;
    line_color = p_line_color;
    fill_color = p_fill_color;
    points.clear();
    queue_redraw();
}

void GGraph::draw_polygon_shape(float p_width, float p_height, const PackedVector2Array &p_points, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color) {
    set_size(Vector2(p_width, p_height));
    shape_type = SHAPE_POLYGON;
    line_size = p_line_size;
    line_color = p_line_color;
    fill_color = p_fill_color;
    points = p_points;
    queue_redraw();
}

void GGraph::clear_shape() {
    shape_type = SHAPE_NONE;
    line_size = 0;
    points.clear();
    queue_redraw();
}

int32_t GGraph::get_shape_type() const {
    return shape_type;
}

int32_t GGraph::get_line_size() const {
    return line_size;
}

Color GGraph::get_line_color() const {
    return line_color;
}

Color GGraph::get_fill_color() const {
    return fill_color;
}

int32_t GGraph::get_point_count() const {
    if (shape_type == SHAPE_ELLIPSE) {
        return ELLIPSE_SEGMENTS;
    }
    return points.size();
}

bool GGraph::is_empty() const {
    return shape_type == SHAPE_NONE;
}

void GGraph::draw_outline_polygon(const PackedVector2Array &p_points) {
    if (line_size <= 0 || p_points.size() < 2) {
        return;
    }

    for (int32_t i = 0; i < p_points.size(); i++) {
        draw_line(p_points[i], p_points[(i + 1) % p_points.size()], line_color, line_size);
    }
}

PackedVector2Array GGraph::make_ellipse_points() const {
    PackedVector2Array result;
    result.resize(ELLIPSE_SEGMENTS);
    const Vector2 center = get_size() * 0.5f;
    const Vector2 radius = get_size() * 0.5f;
    for (int32_t i = 0; i < ELLIPSE_SEGMENTS; i++) {
        const double angle = TAU * static_cast<double>(i) / static_cast<double>(ELLIPSE_SEGMENTS);
        result.set(i, Vector2(center.x + static_cast<float>(std::cos(angle)) * radius.x, center.y + static_cast<float>(std::sin(angle)) * radius.y));
    }
    return result;
}

PackedVector2Array GGraph::make_regular_polygon_points(int32_t p_sides, float p_start_angle, const Vector<float> &p_distances) const {
    PackedVector2Array result;
    const int32_t sides = MAX(3, p_sides);
    result.resize(sides);
    const Vector2 center = get_size() * 0.5f;
    const float radius = MIN(get_size().x, get_size().y) * 0.5f;

    for (int32_t i = 0; i < sides; i++) {
        const float distance_scale = p_distances.size() > i ? p_distances[i] : 1.0f;
        const double angle = static_cast<double>(p_start_angle) + TAU * static_cast<double>(i) / static_cast<double>(sides);
        result.set(i, Vector2(center.x + static_cast<float>(std::cos(angle)) * radius * distance_scale, center.y + static_cast<float>(std::sin(angle)) * radius * distance_scale));
    }
    return result;
}
