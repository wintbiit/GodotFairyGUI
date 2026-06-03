#include "g_label.h"

#include "g_button.h"
#include "g_image.h"
#include "g_loader.h"
#include "g_text_field.h"
#include "g_text_input.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GLabel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_title", "title"), &GLabel::set_title);
    ClassDB::bind_method(D_METHOD("get_title"), &GLabel::get_title);
    ClassDB::bind_method(D_METHOD("set_text", "text"), &GLabel::set_text);
    ClassDB::bind_method(D_METHOD("get_text"), &GLabel::get_text);
    ClassDB::bind_method(D_METHOD("set_icon", "icon"), &GLabel::set_icon);
    ClassDB::bind_method(D_METHOD("get_icon"), &GLabel::get_icon);
    ClassDB::bind_method(D_METHOD("set_title_color", "color"), &GLabel::set_title_color);
    ClassDB::bind_method(D_METHOD("get_title_color"), &GLabel::get_title_color);
    ClassDB::bind_method(D_METHOD("set_title_font_size", "size"), &GLabel::set_title_font_size);
    ClassDB::bind_method(D_METHOD("get_title_font_size"), &GLabel::get_title_font_size);
    ClassDB::bind_method(D_METHOD("set_editable", "editable"), &GLabel::set_editable);
    ClassDB::bind_method(D_METHOD("is_editable"), &GLabel::is_editable);
    ClassDB::bind_method(D_METHOD("set_prompt_text", "prompt_text"), &GLabel::set_prompt_text);
    ClassDB::bind_method(D_METHOD("get_prompt_text"), &GLabel::get_prompt_text);
    ClassDB::bind_method(D_METHOD("set_restrict", "restrict"), &GLabel::set_restrict);
    ClassDB::bind_method(D_METHOD("get_restrict"), &GLabel::get_restrict);
    ClassDB::bind_method(D_METHOD("set_max_length", "max_length"), &GLabel::set_max_length);
    ClassDB::bind_method(D_METHOD("get_max_length"), &GLabel::get_max_length);
    ClassDB::bind_method(D_METHOD("set_display_as_password", "display_as_password"), &GLabel::set_display_as_password);
    ClassDB::bind_method(D_METHOD("is_display_as_password"), &GLabel::is_display_as_password);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "icon"), "set_icon", "get_icon");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "title_color"), "set_title_color", "get_title_color");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "title_font_size"), "set_title_font_size", "get_title_font_size");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editable"), "set_editable", "is_editable");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "prompt_text"), "set_prompt_text", "get_prompt_text");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "restrict"), "set_restrict", "get_restrict");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_length"), "set_max_length", "get_max_length");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_as_password"), "set_display_as_password", "is_display_as_password");
}

void GLabel::construct_extension_from_package(const fgui::PackageItem &p_item) {
    GComponent::construct_extension_from_package(p_item);
    refresh_child_refs();
}

void GLabel::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_after_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 6)) {
        return;
    }

    const fgui::ObjectType object_type = static_cast<fgui::ObjectType>(p_buffer.read_byte());
    if (object_type != fgui::ObjectType::Label) {
        return;
    }

    const String read_title = p_buffer.read_s();
    if (!read_title.is_empty()) {
        set_title(read_title);
    }

    const String read_icon = p_buffer.read_s();
    if (!read_icon.is_empty()) {
        set_icon(read_icon);
    }

    if (p_buffer.read_bool()) {
        set_title_color(p_buffer.read_color());
    }

    const int32_t read_font_size = p_buffer.read_int();
    if (read_font_size != 0) {
        set_title_font_size(read_font_size);
    }

    if (p_buffer.read_bool()) {
        GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
        if (input != nullptr) {
            const String prompt = p_buffer.read_s();
            if (!prompt.is_empty()) {
                input->set_prompt_text(prompt);
            }

            const String restrict = p_buffer.read_s();
            if (!restrict.is_empty()) {
                input->set_restrict(restrict);
            }

            const int32_t max_length = p_buffer.read_int();
            if (max_length != 0) {
                input->set_max_length(max_length);
            }

            const int32_t keyboard_type = p_buffer.read_int();
            if (keyboard_type != 0) {
                input->set_keyboard_type(keyboard_type);
            }

            if (p_buffer.read_bool()) {
                input->set_display_as_password(true);
            }
        } else {
            p_buffer.skip(13);
        }
    }

    if (p_buffer.version >= 5) {
        const String sound = p_buffer.read_s();
        if (!sound.is_empty()) {
            p_buffer.read_float();
        } else {
            p_buffer.skip(4);
        }
    }
}

void GLabel::set_title(const String &p_title) {
    title = p_title;
    apply_title_to_object();
}

String GLabel::get_title() const {
    return title;
}

void GLabel::set_text(const String &p_text) {
    set_title(p_text);
}

String GLabel::get_text() const {
    return title;
}

void GLabel::set_icon(const String &p_icon) {
    icon = p_icon;
    apply_icon_to_object();
}

String GLabel::get_icon() const {
    return icon;
}

void GLabel::set_title_color(const Color &p_color) {
    title_color = p_color;
    GTextField *text_field = get_text_field();
    if (text_field != nullptr) {
        text_field->set_text_color(title_color);
    }
}

Color GLabel::get_title_color() const {
    return title_color;
}

void GLabel::set_title_font_size(int32_t p_size) {
    title_font_size = p_size;
    GTextField *text_field = get_text_field();
    if (text_field != nullptr && title_font_size > 0) {
        text_field->set_font_size(title_font_size);
    }
}

int32_t GLabel::get_title_font_size() const {
    return title_font_size;
}

void GLabel::set_editable(bool p_editable) {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    if (input != nullptr) {
        input->set_editable(p_editable);
    }
}

bool GLabel::is_editable() const {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    return input != nullptr ? input->is_editable() : false;
}

void GLabel::set_prompt_text(const String &p_prompt_text) {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    if (input != nullptr) {
        input->set_prompt_text(p_prompt_text);
    }
}

String GLabel::get_prompt_text() const {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    return input != nullptr ? input->get_prompt_text() : String();
}

void GLabel::set_restrict(const String &p_restrict) {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    if (input != nullptr) {
        input->set_restrict(p_restrict);
    }
}

String GLabel::get_restrict() const {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    return input != nullptr ? input->get_restrict() : String();
}

void GLabel::set_max_length(int32_t p_max_length) {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    if (input != nullptr) {
        input->set_max_length(p_max_length);
    }
}

int32_t GLabel::get_max_length() const {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    return input != nullptr ? input->get_max_length() : 0;
}

void GLabel::set_display_as_password(bool p_display_as_password) {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    if (input != nullptr) {
        input->set_display_as_password(p_display_as_password);
    }
}

bool GLabel::is_display_as_password() const {
    GTextInput *input = Object::cast_to<GTextInput>(get_text_field());
    return input != nullptr ? input->is_display_as_password() : false;
}

void GLabel::refresh_child_refs() {
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

void GLabel::apply_title_to_object() {
    if (title_object == nullptr) {
        return;
    }

    GTextField *text_field = Object::cast_to<GTextField>(title_object);
    if (text_field != nullptr) {
        text_field->set_text(title);
        if (title_font_size > 0) {
            text_field->set_font_size(title_font_size);
        }
        text_field->set_text_color(title_color);
        return;
    }

    GButton *button = Object::cast_to<GButton>(title_object);
    if (button != nullptr) {
        button->set_title(title);
        return;
    }

    GLabel *label = Object::cast_to<GLabel>(title_object);
    if (label != nullptr) {
        label->set_title(title);
    }
}

void GLabel::apply_icon_to_object() {
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

GTextField *GLabel::get_text_field() const {
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
