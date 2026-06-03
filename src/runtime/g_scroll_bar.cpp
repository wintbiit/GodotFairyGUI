#include "g_scroll_bar.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GScrollBar::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_vertical", "vertical"), &GScrollBar::set_vertical);
    ClassDB::bind_method(D_METHOD("is_vertical"), &GScrollBar::is_vertical);
    ClassDB::bind_method(D_METHOD("set_fixed_grip_size", "fixed"), &GScrollBar::set_fixed_grip_size);
    ClassDB::bind_method(D_METHOD("is_fixed_grip_size"), &GScrollBar::is_fixed_grip_size);
    ClassDB::bind_method(D_METHOD("set_display_perc", "value"), &GScrollBar::set_display_perc);
    ClassDB::bind_method(D_METHOD("get_display_perc"), &GScrollBar::get_display_perc);
    ClassDB::bind_method(D_METHOD("set_scroll_perc", "value"), &GScrollBar::set_scroll_perc);
    ClassDB::bind_method(D_METHOD("get_scroll_perc"), &GScrollBar::get_scroll_perc);
    ClassDB::bind_method(D_METHOD("get_min_size"), &GScrollBar::get_min_size);
    ClassDB::bind_method(D_METHOD("is_grip_dragging"), &GScrollBar::is_grip_dragging);
    ClassDB::bind_method(D_METHOD("has_grip"), &GScrollBar::has_grip);
    ClassDB::bind_method(D_METHOD("has_bar"), &GScrollBar::has_bar);
    ClassDB::bind_method(D_METHOD("has_arrow1"), &GScrollBar::has_arrow1);
    ClassDB::bind_method(D_METHOD("has_arrow2"), &GScrollBar::has_arrow2);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical"), "set_vertical", "is_vertical");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fixed_grip_size"), "set_fixed_grip_size", "is_fixed_grip_size");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "display_perc"), "set_display_perc", "get_display_perc");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_perc"), "set_scroll_perc", "get_scroll_perc");

    ADD_SIGNAL(MethodInfo("fgui_scroll_perc_changed", PropertyInfo(Variant::FLOAT, "scroll_perc")));
    ADD_SIGNAL(MethodInfo("fgui_arrow1"));
    ADD_SIGNAL(MethodInfo("fgui_arrow2"));
    ADD_SIGNAL(MethodInfo("fgui_bar_page", PropertyInfo(Variant::INT, "direction")));
}

void GScrollBar::_gui_input(const Ref<InputEvent> &p_event) {
    GComponent::_gui_input(p_event);

    Ref<InputEventMouseButton> mouse_button = p_event;
    if (!mouse_button.is_valid() || mouse_button->get_button_index() != MouseButton::MOUSE_BUTTON_LEFT || !mouse_button->is_pressed()) {
        return;
    }

    const Vector2 local_position = mouse_button->get_position();
    if (arrow1 != nullptr && Rect2(arrow1->get_position(), arrow1->get_size()).has_point(local_position)) {
        emit_signal("fgui_arrow1");
        accept_event();
        return;
    }
    if (arrow2 != nullptr && Rect2(arrow2->get_position(), arrow2->get_size()).has_point(local_position)) {
        emit_signal("fgui_arrow2");
        accept_event();
        return;
    }
    if (grip != nullptr && Rect2(grip->get_position(), grip->get_size()).has_point(local_position)) {
        grip_dragging = true;
        accept_event();
        return;
    }

    emit_bar_click(local_position);
}

void GScrollBar::construct_extension_from_package(const fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    construct_extension(buffer);
}

void GScrollBar::set_vertical(bool p_vertical) {
    vertical = p_vertical;
    update_grip();
}

bool GScrollBar::is_vertical() const {
    return vertical;
}

void GScrollBar::set_fixed_grip_size(bool p_fixed) {
    fixed_grip_size = p_fixed;
    update_grip();
}

bool GScrollBar::is_fixed_grip_size() const {
    return fixed_grip_size;
}

void GScrollBar::set_display_perc(double p_value) {
    display_perc = CLAMP(p_value, 0.0, 1.0);
    update_grip();
}

double GScrollBar::get_display_perc() const {
    return display_perc;
}

void GScrollBar::set_scroll_perc(double p_value) {
    const double next = CLAMP(p_value, 0.0, 1.0);
    const bool changed = next != scroll_perc;
    scroll_perc = next;
    update_grip();
    if (changed) {
        emit_signal("fgui_scroll_perc_changed", scroll_perc);
    }
}

double GScrollBar::get_scroll_perc() const {
    return scroll_perc;
}

double GScrollBar::get_min_size() const {
    if (vertical) {
        return (arrow1 != nullptr ? arrow1->get_size().y : 0.0) + (arrow2 != nullptr ? arrow2->get_size().y : 0.0);
    }
    return (arrow1 != nullptr ? arrow1->get_size().x : 0.0) + (arrow2 != nullptr ? arrow2->get_size().x : 0.0);
}

bool GScrollBar::is_grip_dragging() const {
    return grip_dragging;
}

bool GScrollBar::has_grip() const {
    return grip != nullptr;
}

bool GScrollBar::has_bar() const {
    return bar != nullptr;
}

bool GScrollBar::has_arrow1() const {
    return arrow1 != nullptr;
}

bool GScrollBar::has_arrow2() const {
    return arrow2 != nullptr;
}

void GScrollBar::construct_extension(fgui::ByteBuffer &p_buffer) {
    if (p_buffer.seek(0, 6)) {
        fixed_grip_size = p_buffer.read_bool();
    }

    refresh_children();
    infer_vertical();
    update_grip();
}

void GScrollBar::refresh_children() {
    grip = nullptr;
    bar = nullptr;
    arrow1 = nullptr;
    arrow2 = nullptr;

    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child == nullptr) {
            continue;
        }

        const StringName child_name = child->get_name();
        if (child_name == StringName("grip")) {
            grip = child;
        } else if (child_name == StringName("bar")) {
            bar = child;
        } else if (child_name == StringName("arrow1")) {
            arrow1 = child;
        } else if (child_name == StringName("arrow2")) {
            arrow2 = child;
        }
    }
}

void GScrollBar::infer_vertical() {
    if (bar != nullptr) {
        vertical = bar->get_size().y >= bar->get_size().x;
        return;
    }
    vertical = get_size().y >= get_size().x;
}

void GScrollBar::update_grip() {
    if (grip == nullptr || bar == nullptr) {
        return;
    }

    if (vertical) {
        if (!fixed_grip_size) {
            grip->set_size(Vector2(grip->get_size().x, Math::floor(display_perc * bar->get_size().y)));
        }
        grip->set_visible(display_perc > 0.0 && display_perc < 1.0);
        grip->set_position(Vector2(grip->get_position().x, Math::round(bar->get_position().y + (bar->get_size().y - grip->get_size().y) * scroll_perc)));
    } else {
        if (!fixed_grip_size) {
            grip->set_size(Vector2(Math::floor(display_perc * bar->get_size().x), grip->get_size().y));
        }
        grip->set_visible(display_perc > 0.0 && display_perc < 1.0);
        grip->set_position(Vector2(Math::round(bar->get_position().x + (bar->get_size().x - grip->get_size().x) * scroll_perc), grip->get_position().y));
    }
}

void GScrollBar::emit_bar_click(const Vector2 &p_local_position) {
    if (grip == nullptr) {
        return;
    }

    const int32_t direction = vertical ? (p_local_position.y < grip->get_position().y ? -1 : 1) : (p_local_position.x < grip->get_position().x ? -1 : 1);
    emit_signal("fgui_bar_page", direction);
    accept_event();
}

