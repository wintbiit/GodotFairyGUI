#include "g_combo_box.h"

#include "g_button.h"
#include "g_image.h"
#include "g_label.h"
#include "g_loader.h"
#include "g_text_field.h"
#include "ui_package.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GComboBox::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_title", "title"), &GComboBox::set_title);
    ClassDB::bind_method(D_METHOD("get_title"), &GComboBox::get_title);
    ClassDB::bind_method(D_METHOD("set_text", "text"), &GComboBox::set_text);
    ClassDB::bind_method(D_METHOD("get_text"), &GComboBox::get_text);
    ClassDB::bind_method(D_METHOD("set_icon", "icon"), &GComboBox::set_icon);
    ClassDB::bind_method(D_METHOD("get_icon"), &GComboBox::get_icon);
    ClassDB::bind_method(D_METHOD("set_selected_index", "index"), &GComboBox::set_selected_index);
    ClassDB::bind_method(D_METHOD("get_selected_index"), &GComboBox::get_selected_index);
    ClassDB::bind_method(D_METHOD("set_value", "value"), &GComboBox::set_value);
    ClassDB::bind_method(D_METHOD("get_value"), &GComboBox::get_value);
    ClassDB::bind_method(D_METHOD("set_visible_item_count", "count"), &GComboBox::set_visible_item_count);
    ClassDB::bind_method(D_METHOD("get_visible_item_count"), &GComboBox::get_visible_item_count);
    ClassDB::bind_method(D_METHOD("get_popup_direction"), &GComboBox::get_popup_direction);
    ClassDB::bind_method(D_METHOD("get_dropdown_url"), &GComboBox::get_dropdown_url);
    ClassDB::bind_method(D_METHOD("has_dropdown"), &GComboBox::has_dropdown);
    ClassDB::bind_method(D_METHOD("clear_items"), &GComboBox::clear_items);
    ClassDB::bind_method(D_METHOD("add_item", "text", "value", "icon"), &GComboBox::add_item, DEFVAL(String()), DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("apply_list_change"), &GComboBox::apply_list_change);
    ClassDB::bind_method(D_METHOD("get_item_count"), &GComboBox::get_item_count);
    ClassDB::bind_method(D_METHOD("get_item_text", "index"), &GComboBox::get_item_text);
    ClassDB::bind_method(D_METHOD("get_item_value", "index"), &GComboBox::get_item_value);
    ClassDB::bind_method(D_METHOD("get_item_icon", "index"), &GComboBox::get_item_icon);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "icon"), "set_icon", "get_icon");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "selected_index"), "set_selected_index", "get_selected_index");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "value"), "set_value", "get_value");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "visible_item_count"), "set_visible_item_count", "get_visible_item_count");

    ADD_SIGNAL(MethodInfo("fgui_changed"));
}

void GComboBox::construct_extension_from_package(const fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    construct_extension(buffer);
}

void GComboBox::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_after_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 6)) {
        return;
    }

    const fgui::ObjectType object_type = static_cast<fgui::ObjectType>(p_buffer.read_byte());
    if (object_type != fgui::ObjectType::ComboBox) {
        return;
    }

    items.clear();
    values.clear();
    icons.clear();

    const int32_t item_count = p_buffer.read_short();
    for (int32_t i = 0; i < item_count; i++) {
        const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();
        items.push_back(p_buffer.read_s());
        values.push_back(p_buffer.read_s());
        icons.push_back(p_buffer.read_s());
        p_buffer.set_position(next_pos);
    }

    const String selected_title = p_buffer.read_s();
    if (!selected_title.is_empty()) {
        set_title(selected_title);
        selected_index = -1;
        for (int32_t i = 0; i < items.size(); i++) {
            if (items[i] == selected_title) {
                selected_index = i;
                break;
            }
        }
    } else if (!items.is_empty()) {
        selected_index = 0;
        update_selection(false);
    } else {
        selected_index = -1;
    }

    const String selected_icon = p_buffer.read_s();
    if (!selected_icon.is_empty()) {
        set_icon(selected_icon);
    }

    if (p_buffer.read_bool()) {
        title_color = p_buffer.read_color();
        GTextField *text_field = get_text_field();
        if (text_field != nullptr) {
            text_field->set_text_color(title_color);
        }
    }

    const int32_t read_visible_count = p_buffer.read_int();
    if (read_visible_count > 0) {
        visible_item_count = read_visible_count;
    }
    popup_direction = p_buffer.read_byte();
    p_buffer.read_short(); // selection controller index

    if (p_buffer.version >= 5) {
        p_buffer.read_s(); // sound
        p_buffer.read_float(); // sound volume scale
    }
}

void GComboBox::set_title(const String &p_title) {
    title = p_title;
    apply_title_to_object();
}

String GComboBox::get_title() const {
    return title;
}

void GComboBox::set_text(const String &p_text) {
    set_title(p_text);
}

String GComboBox::get_text() const {
    return title;
}

void GComboBox::set_icon(const String &p_icon) {
    icon = p_icon;
    apply_icon_to_object();
}

String GComboBox::get_icon() const {
    return icon;
}

void GComboBox::set_selected_index(int32_t p_index) {
    if (p_index < -1) {
        p_index = -1;
    }
    if (p_index >= items.size()) {
        p_index = items.size() - 1;
    }
    if (selected_index == p_index) {
        return;
    }
    selected_index = p_index;
    update_selection(true);
}

int32_t GComboBox::get_selected_index() const {
    return selected_index;
}

void GComboBox::set_value(const String &p_value) {
    for (int32_t i = 0; i < values.size(); i++) {
        if (values[i] == p_value) {
            set_selected_index(i);
            return;
        }
    }
    if (!items.is_empty()) {
        set_selected_index(0);
    }
}

String GComboBox::get_value() const {
    if (selected_index >= 0 && selected_index < values.size()) {
        return values[selected_index];
    }
    return String();
}

void GComboBox::set_visible_item_count(int32_t p_count) {
    visible_item_count = MAX(0, p_count);
}

int32_t GComboBox::get_visible_item_count() const {
    return visible_item_count;
}

int32_t GComboBox::get_popup_direction() const {
    return popup_direction;
}

String GComboBox::get_dropdown_url() const {
    return dropdown_url;
}

bool GComboBox::has_dropdown() const {
    return dropdown != nullptr;
}

void GComboBox::clear_items() {
    items.clear();
    values.clear();
    icons.clear();
    selected_index = -1;
    set_title(String());
    set_icon(String());
}

void GComboBox::add_item(const String &p_text, const String &p_value, const String &p_icon) {
    items.push_back(p_text);
    values.push_back(p_value);
    icons.push_back(p_icon);
    apply_list_change();
}

void GComboBox::apply_list_change() {
    if (!items.is_empty()) {
        if (selected_index < 0 || selected_index >= items.size()) {
            selected_index = 0;
        }
        update_selection(false);
    } else {
        selected_index = -1;
        set_title(String());
        set_icon(String());
    }
}

int32_t GComboBox::get_item_count() const {
    return items.size();
}

String GComboBox::get_item_text(int32_t p_index) const {
    return p_index >= 0 && p_index < items.size() ? items[p_index] : String();
}

String GComboBox::get_item_value(int32_t p_index) const {
    return p_index >= 0 && p_index < values.size() ? values[p_index] : String();
}

String GComboBox::get_item_icon(int32_t p_index) const {
    return p_index >= 0 && p_index < icons.size() ? icons[p_index] : String();
}

void GComboBox::refresh_child_refs() {
    title_object = nullptr;
    icon_object = nullptr;
    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child == nullptr) {
            continue;
        }
        const StringName child_name = child->get_name();
        if (child_name == StringName("title")) {
            title_object = child;
        } else if (child_name == StringName("icon")) {
            icon_object = child;
        }
    }
    apply_title_to_object();
    apply_icon_to_object();
}

void GComboBox::construct_extension(fgui::ByteBuffer &p_buffer) {
    refresh_child_refs();
    if (!p_buffer.seek(0, 6)) {
        return;
    }
    dropdown_url = p_buffer.read_s();
    if (!dropdown_url.is_empty()) {
        dropdown = Object::cast_to<GComponent>(UIPackage::create_object_from_url(dropdown_url));
    }
}

void GComboBox::apply_title_to_object() {
    if (title_object == nullptr) {
        return;
    }
    GTextField *text_field = Object::cast_to<GTextField>(title_object);
    if (text_field != nullptr) {
        text_field->set_text(title);
        return;
    }
    GLabel *label = Object::cast_to<GLabel>(title_object);
    if (label != nullptr) {
        label->set_title(title);
        return;
    }
    GButton *button = Object::cast_to<GButton>(title_object);
    if (button != nullptr) {
        button->set_title(title);
    }
}

void GComboBox::apply_icon_to_object() {
    if (icon_object == nullptr) {
        return;
    }
    GLoader *loader = Object::cast_to<GLoader>(icon_object);
    if (loader != nullptr) {
        loader->set_url(icon);
        return;
    }
    GImage *image = Object::cast_to<GImage>(icon_object);
    if (image != nullptr && icon.begins_with("ui://")) {
        const int64_t slash = icon.substr(5).find("/");
        if (slash >= 0) {
            const String body = icon.substr(5);
            image->set_package_image(body.substr(0, slash), body.substr(slash + 1));
        }
    }
}

void GComboBox::update_selection(bool p_emit_signal) {
    if (selected_index >= 0 && selected_index < items.size()) {
        set_title(items[selected_index]);
        if (selected_index < icons.size()) {
            set_icon(icons[selected_index]);
        }
    } else {
        set_title(String());
        set_icon(String());
    }
    if (p_emit_signal) {
        emit_signal("fgui_changed");
    }
}

GTextField *GComboBox::get_text_field() const {
    GTextField *text_field = Object::cast_to<GTextField>(title_object);
    if (text_field != nullptr) {
        return text_field;
    }
    GLabel *label = Object::cast_to<GLabel>(title_object);
    if (label != nullptr) {
        return label->get_text_field();
    }
    return nullptr;
}
