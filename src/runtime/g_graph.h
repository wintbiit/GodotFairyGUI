#pragma once

#include "g_object.h"

#include <godot_cpp/variant/packed_vector2_array.hpp>

namespace godot {

class GGraph : public GObject {
    GDCLASS(GGraph, GObject)

protected:
    static void _bind_methods();

public:
    enum ShapeType {
        SHAPE_NONE = 0,
        SHAPE_RECT = 1,
        SHAPE_ELLIPSE = 2,
        SHAPE_POLYGON = 3,
        SHAPE_REGULAR_POLYGON = 4,
    };

    void _draw() override;
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void draw_rect_shape(float p_width, float p_height, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color);
    void draw_ellipse_shape(float p_width, float p_height, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color);
    void draw_polygon_shape(float p_width, float p_height, const PackedVector2Array &p_points, int32_t p_line_size, const Color &p_line_color, const Color &p_fill_color);
    void clear_shape();

    int32_t get_shape_type() const;
    int32_t get_line_size() const;
    Color get_line_color() const;
    Color get_fill_color() const;
    int32_t get_point_count() const;
    bool is_empty() const;

private:
    ShapeType shape_type = SHAPE_NONE;
    int32_t line_size = 0;
    Color line_color = Color(0, 0, 0, 1);
    Color fill_color = Color(0, 0, 0, 0);
    PackedVector2Array points;

    void draw_outline_polygon(const PackedVector2Array &p_points);
    PackedVector2Array make_ellipse_points() const;
    PackedVector2Array make_regular_polygon_points(int32_t p_sides, float p_start_angle, const Vector<float> &p_distances) const;
};

} // namespace godot
