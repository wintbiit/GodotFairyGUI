#include "g_progress_bar.h"

#include "g_button.h"
#include "g_text_field.h"

#include <cmath>

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GProgressBar::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_min", "min"), &GProgressBar::set_min);
    ClassDB::bind_method(D_METHOD("get_min"), &GProgressBar::get_min);
    ClassDB::bind_method(D_METHOD("set_max", "max"), &GProgressBar::set_max);
    ClassDB::bind_method(D_METHOD("get_max"), &GProgressBar::get_max);
    ClassDB::bind_method(D_METHOD("set_value", "value"), &GProgressBar::set_value);
    ClassDB::bind_method(D_METHOD("get_value"), &GProgressBar::get_value);
    ClassDB::bind_method(D_METHOD("set_title_type", "title_type"), &GProgressBar::set_title_type);
    ClassDB::bind_method(D_METHOD("get_title_type"), &GProgressBar::get_title_type);
    ClassDB::bind_method(D_METHOD("set_reverse", "reverse"), &GProgressBar::set_reverse);
    ClassDB::bind_method(D_METHOD("is_reverse"), &GProgressBar::is_reverse);
    ClassDB::bind_method(D_METHOD("get_percent"), &GProgressBar::get_percent);
    ClassDB::bind_method(D_METHOD("get_title_text"), &GProgressBar::get_title_text);

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min"), "set_min", "get_min");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max"), "set_max", "get_max");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value"), "set_value", "get_value");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "title_type"), "set_title_type", "get_title_type");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reverse"), "set_reverse", "is_reverse");
}

void GProgressBar::_notification(int p_what) {
    GComponent::_notification(p_what);
    if (p_what == NOTIFICATION_RESIZED) {
        update_progress();
    }
}

void GProgressBar::construct_extension_from_package(const fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    construct_extension(buffer);
}

void GProgressBar::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_after_add(p_buffer, p_begin_pos);

    if (p_buffer.seek(p_begin_pos, 6)) {
        const fgui::ObjectType object_type = static_cast<fgui::ObjectType>(p_buffer.read_byte());
        if (object_type == fgui::ObjectType::ProgressBar) {
            value = p_buffer.read_int();
            max = p_buffer.read_int();
            if (p_buffer.version >= 2) {
                min = p_buffer.read_int();
            }
            if (p_buffer.version >= 5) {
                const String sound = p_buffer.read_s();
                if (!sound.is_empty()) {
                    p_buffer.read_float();
                }
            }
        }
    }

    update_progress();
}

void GProgressBar::set_min(double p_min) {
    min = p_min;
    update_progress();
}

double GProgressBar::get_min() const {
    return min;
}

void GProgressBar::set_max(double p_max) {
    max = p_max;
    update_progress();
}

double GProgressBar::get_max() const {
    return max;
}

void GProgressBar::set_value(double p_value) {
    value = p_value;
    update_progress();
}

double GProgressBar::get_value() const {
    return value;
}

void GProgressBar::set_title_type(int32_t p_title_type) {
    title_type = static_cast<fgui::ProgressTitleType>(CLAMP(p_title_type, 0, 3));
    update_progress();
}

int32_t GProgressBar::get_title_type() const {
    return static_cast<int32_t>(title_type);
}

void GProgressBar::set_reverse(bool p_reverse) {
    reverse = p_reverse;
    update_progress();
}

bool GProgressBar::is_reverse() const {
    return reverse;
}

double GProgressBar::get_percent() const {
    if (max == min) {
        return 0.0;
    }
    return CLAMP((value - min) / (max - min), 0.0, 1.0);
}

String GProgressBar::get_title_text() const {
    return title_text;
}

void GProgressBar::construct_extension(fgui::ByteBuffer &p_buffer) {
    if (p_buffer.seek(0, 6)) {
        title_type = static_cast<fgui::ProgressTitleType>(p_buffer.read_byte());
        reverse = p_buffer.read_bool();
    }

    refresh_children();
    update_progress();
}

void GProgressBar::refresh_children() {
    title_object = nullptr;
    bar = nullptr;
    bar_v = nullptr;

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
        }
    }
}

void GProgressBar::update_progress() {
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
    }

    if (bar_v != nullptr) {
        const double full_height = MAX(0.0, get_size().y - bar_height_delta);
        const double height = std::round(full_height * percent);
        bar_v->set_size(Vector2(bar_v->get_size().x, height));
        bar_v->set_position(Vector2(bar_v->get_position().x, reverse ? bar_start_y : bar_start_y + (full_height - height)));
    }
}

void GProgressBar::set_title_object_text(const String &p_text) {
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
