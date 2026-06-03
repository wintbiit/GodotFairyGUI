#include "g_slider.h"

#include "g_button.h"
#include "g_text_field.h"

#include <cmath>

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GSlider::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_min", "min"), &GSlider::set_min);
    ClassDB::bind_method(D_METHOD("get_min"), &GSlider::get_min);
    ClassDB::bind_method(D_METHOD("set_max", "max"), &GSlider::set_max);
    ClassDB::bind_method(D_METHOD("get_max"), &GSlider::get_max);
    ClassDB::bind_method(D_METHOD("set_value", "value"), &GSlider::set_value);
    ClassDB::bind_method(D_METHOD("get_value"), &GSlider::get_value);
    ClassDB::bind_method(D_METHOD("set_title_type", "title_type"), &GSlider::set_title_type);
    ClassDB::bind_method(D_METHOD("get_title_type"), &GSlider::get_title_type);
    ClassDB::bind_method(D_METHOD("set_reverse", "reverse"), &GSlider::set_reverse);
    ClassDB::bind_method(D_METHOD("is_reverse"), &GSlider::is_reverse);
    ClassDB::bind_method(D_METHOD("set_whole_numbers", "whole_numbers"), &GSlider::set_whole_numbers);
    ClassDB::bind_method(D_METHOD("is_whole_numbers"), &GSlider::is_whole_numbers);
    ClassDB::bind_method(D_METHOD("set_change_on_click", "change_on_click"), &GSlider::set_change_on_click);
    ClassDB::bind_method(D_METHOD("is_change_on_click"), &GSlider::is_change_on_click);
    ClassDB::bind_method(D_METHOD("set_can_drag", "can_drag"), &GSlider::set_can_drag);
    ClassDB::bind_method(D_METHOD("can_drag"), &GSlider::can_drag);
    ClassDB::bind_method(D_METHOD("get_percent"), &GSlider::get_percent);
    ClassDB::bind_method(D_METHOD("get_title_text"), &GSlider::get_title_text);
    ClassDB::bind_method(D_METHOD("update_with_percent", "percent", "manual"), &GSlider::update_with_percent);

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min"), "set_min", "get_min");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max"), "set_max", "get_max");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value"), "set_value", "get_value");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "title_type"), "set_title_type", "get_title_type");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reverse"), "set_reverse", "is_reverse");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "whole_numbers"), "set_whole_numbers", "is_whole_numbers");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "change_on_click"), "set_change_on_click", "is_change_on_click");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_drag"), "set_can_drag", "can_drag");

    ADD_SIGNAL(MethodInfo("fgui_changed"));
    ADD_SIGNAL(MethodInfo("fgui_grip_touch_end"));
}

void GSlider::_notification(int p_what) {
    GComponent::_notification(p_what);
    if (p_what == NOTIFICATION_RESIZED) {
        update_slider();
    }
}

void GSlider::_gui_input(const Ref<InputEvent> &p_event) {
    GComponent::_gui_input(p_event);

    Ref<InputEventMouseButton> mouse_button = p_event;
    if (mouse_button.is_valid() && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT && mouse_button->is_pressed() && change_on_click) {
        const Vector2 point = mouse_button->get_position();
        double percent = get_percent();
        if (bar != nullptr) {
            const double full_width = MAX(1.0, get_size().x - bar_width_delta);
            percent = point.x / full_width;
        } else if (bar_v != nullptr) {
            const double full_height = MAX(1.0, get_size().y - bar_height_delta);
            percent = point.y / full_height;
        }
        if (reverse) {
            percent = 1.0 - percent;
        }
        update_with_percent(percent, true);
        accept_event();
    }
}

void GSlider::construct_extension_from_package(const fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    construct_extension(buffer);
}

void GSlider::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_after_add(p_buffer, p_begin_pos);

    if (p_buffer.seek(p_begin_pos, 6)) {
        const fgui::ObjectType object_type = static_cast<fgui::ObjectType>(p_buffer.read_byte());
        if (object_type == fgui::ObjectType::Slider) {
            value = p_buffer.read_int();
            max = p_buffer.read_int();
            if (p_buffer.version >= 2) {
                min = p_buffer.read_int();
            }
        }
    }

    update_slider();
}

void GSlider::set_min(double p_min) {
    min = p_min;
    update_slider();
}

double GSlider::get_min() const {
    return min;
}

void GSlider::set_max(double p_max) {
    max = p_max;
    update_slider();
}

double GSlider::get_max() const {
    return max;
}

void GSlider::set_value(double p_value) {
    value = whole_numbers ? std::round(p_value) : p_value;
    update_slider();
}

double GSlider::get_value() const {
    return value;
}

void GSlider::set_title_type(int32_t p_title_type) {
    title_type = static_cast<fgui::ProgressTitleType>(CLAMP(p_title_type, 0, 3));
    update_slider();
}

int32_t GSlider::get_title_type() const {
    return static_cast<int32_t>(title_type);
}

void GSlider::set_reverse(bool p_reverse) {
    reverse = p_reverse;
    update_slider();
}

bool GSlider::is_reverse() const {
    return reverse;
}

void GSlider::set_whole_numbers(bool p_whole_numbers) {
    whole_numbers = p_whole_numbers;
    if (whole_numbers) {
        value = std::round(value);
    }
    update_slider();
}

bool GSlider::is_whole_numbers() const {
    return whole_numbers;
}

void GSlider::set_change_on_click(bool p_change_on_click) {
    change_on_click = p_change_on_click;
}

bool GSlider::is_change_on_click() const {
    return change_on_click;
}

void GSlider::set_can_drag(bool p_can_drag) {
    drag_enabled = p_can_drag;
}

bool GSlider::can_drag() const {
    return drag_enabled;
}

double GSlider::get_percent() const {
    if (max == min) {
        return 0.0;
    }
    return CLAMP((value - min) / (max - min), 0.0, 1.0);
}

String GSlider::get_title_text() const {
    return title_text;
}

bool GSlider::update_with_percent(double p_percent, bool p_manual) {
    const double percent = CLAMP(p_percent, 0.0, 1.0);
    if (p_manual) {
        double new_value = min + (max - min) * percent;
        new_value = CLAMP(new_value, min, max);
        if (whole_numbers) {
            new_value = std::round(new_value);
        }
        const bool changed = new_value != value;
        value = new_value;
        update_slider();
        if (changed) {
            emit_signal("fgui_changed");
        }
        return changed;
    }

    update_slider();
    return false;
}

void GSlider::construct_extension(fgui::ByteBuffer &p_buffer) {
    if (p_buffer.seek(0, 6)) {
        title_type = static_cast<fgui::ProgressTitleType>(p_buffer.read_byte());
        reverse = p_buffer.read_bool();
        if (p_buffer.version >= 2) {
            whole_numbers = p_buffer.read_bool();
            change_on_click = p_buffer.read_bool();
        }
    }

    refresh_children();
    update_slider();
}

void GSlider::refresh_children() {
    title_object = nullptr;
    bar = nullptr;
    bar_v = nullptr;
    grip = nullptr;

    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child == nullptr) {
            continue;
        }

        const StringName child_name = child->get_name();
        if (child_name == StringName("title")) {
            title_object = child;
        } else if (child_name == StringName("bar")) {
            bar = child;
            bar_max_width = child->get_size().x;
            bar_start_x = child->get_position().x;
            bar_width_delta = get_size().x - bar_max_width;
        } else if (child_name == StringName("bar_v")) {
            bar_v = child;
            bar_max_height = child->get_size().y;
            bar_start_y = child->get_position().y;
            bar_height_delta = get_size().y - bar_max_height;
        } else if (child_name == StringName("grip")) {
            grip = child;
        }
    }
}

void GSlider::update_slider() {
    const double percent = get_percent();
    const int64_t rounded_value = static_cast<int64_t>(std::round(value));
    const int64_t rounded_max = static_cast<int64_t>(std::round(max));

    switch (title_type) {
        case fgui::ProgressTitleType::Percent:
            title_text = String::num_int64(static_cast<int64_t>(std::floor(percent * 100.0))) + String("%");
            break;
        case fgui::ProgressTitleType::ValueAndMax:
            title_text = String::num_int64(rounded_value) + String("/") + String::num_int64(rounded_max);
            break;
        case fgui::ProgressTitleType::Value:
            title_text = String::num_int64(rounded_value);
            break;
        case fgui::ProgressTitleType::Max:
            title_text = String::num_int64(rounded_max);
            break;
    }
    set_title_object_text(title_text);

    if (bar != nullptr) {
        const double full_width = MAX(0.0, get_size().x - bar_width_delta);
        const double width = std::round(full_width * percent);
        bar->set_size(Vector2(width, bar->get_size().y));
        bar->set_position(Vector2(reverse ? bar_start_x + (full_width - width) : bar_start_x, bar->get_position().y));
        if (grip != nullptr) {
            grip->set_position(Vector2((reverse ? bar->get_position().x : bar_start_x + width) - grip->get_size().x * 0.5, grip->get_position().y));
        }
    }

    if (bar_v != nullptr) {
        const double full_height = MAX(0.0, get_size().y - bar_height_delta);
        const double height = std::round(full_height * percent);
        bar_v->set_size(Vector2(bar_v->get_size().x, height));
        bar_v->set_position(Vector2(bar_v->get_position().x, reverse ? bar_start_y : bar_start_y + (full_height - height)));
        if (grip != nullptr) {
            grip->set_position(Vector2(grip->get_position().x, (reverse ? bar_v->get_position().y + height : bar_start_y + (full_height - height)) - grip->get_size().y * 0.5));
        }
    }
}

void GSlider::set_title_object_text(const String &p_text) {
    if (title_object == nullptr) {
        return;
    }

    GTextField *text_field = Object::cast_to<GTextField>(title_object);
    if (text_field != nullptr) {
        text_field->set_text(p_text);
        return;
    }

    GButton *button = Object::cast_to<GButton>(title_object);
    if (button != nullptr) {
        button->set_title(p_text);
    }
}

