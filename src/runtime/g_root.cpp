#include "g_root.h"

#include <godot_cpp/core/class_db.hpp>

namespace godot {

GRoot *GRoot::s_instance = nullptr;

GRoot::GRoot() {
    set_name("GRoot");
    set_anchors_preset(PRESET_FULL_RECT);
}

GRoot::~GRoot() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void GRoot::_bind_methods() {
    ClassDB::bind_static_method("GRoot", D_METHOD("get_instance"), &GRoot::get_instance);

    ClassDB::bind_method(D_METHOD("set_design_resolution_x", "x"), &GRoot::set_design_resolution_x);
    ClassDB::bind_method(D_METHOD("get_design_resolution_x"), &GRoot::get_design_resolution_x);
    ClassDB::bind_method(D_METHOD("set_design_resolution_y", "y"), &GRoot::set_design_resolution_y);
    ClassDB::bind_method(D_METHOD("get_design_resolution_y"), &GRoot::get_design_resolution_y);
    ClassDB::bind_method(D_METHOD("get_content_scale_factor"), &GRoot::get_content_scale_factor);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "design_resolution_x"), "set_design_resolution_x", "get_design_resolution_x");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "design_resolution_y"), "set_design_resolution_y", "get_design_resolution_y");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "content_scale_factor"), "", "get_content_scale_factor");
}

GRoot *GRoot::get_instance() {
    if (!s_instance) {
        s_instance = memnew(GRoot);
    }
    return s_instance;
}

void GRoot::set_design_resolution_x(int p_x) {
    design_resolution_x = p_x;
    apply_content_scale_factor();
}

int GRoot::get_design_resolution_x() const {
    return design_resolution_x;
}

void GRoot::set_design_resolution_y(int p_y) {
    design_resolution_y = p_y;
    apply_content_scale_factor();
}

int GRoot::get_design_resolution_y() const {
    return design_resolution_y;
}

float GRoot::get_content_scale_factor() const {
    return content_scale_factor;
}

void GRoot::_ready() {
    apply_content_scale_factor();
}

void GRoot::_notification(int p_what) {
    if (p_what == NOTIFICATION_RESIZED) {
        apply_content_scale_factor();
    }
}

void GRoot::apply_content_scale_factor() {
    if (design_resolution_x <= 0 || design_resolution_y <= 0) {
        content_scale_factor = 1.0f;
        set_scale(Vector2(1.0f, 1.0f));
        return;
    }
    const Vector2 panel_size = get_size();
    if (panel_size.x <= 0.0f || panel_size.y <= 0.0f) {
        return;
    }
    const float s1 = panel_size.x / static_cast<float>(design_resolution_x);
    const float s2 = panel_size.y / static_cast<float>(design_resolution_y);
    content_scale_factor = MIN(s1, s2);
    set_scale(Vector2(content_scale_factor, content_scale_factor));
}

} // namespace godot
