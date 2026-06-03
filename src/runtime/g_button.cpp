#include "g_button.h"

#include "g_loader.h"
#include "ui_package.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GButton::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_selected", "selected"), &GButton::set_selected);
    ClassDB::bind_method(D_METHOD("is_selected"), &GButton::is_selected);
    ClassDB::bind_method(D_METHOD("set_title", "title"), &GButton::set_title);
    ClassDB::bind_method(D_METHOD("get_title"), &GButton::get_title);
    ClassDB::bind_method(D_METHOD("handle_click"), &GButton::handle_click);
    ClassDB::bind_method(D_METHOD("get_mode"), &GButton::get_mode);
    ClassDB::bind_method(D_METHOD("set_sound_url", "sound_url"), &GButton::set_sound_url);
    ClassDB::bind_method(D_METHOD("get_sound_url"), &GButton::get_sound_url);
    ClassDB::bind_method(D_METHOD("set_sound_volume_scale", "sound_volume_scale"), &GButton::set_sound_volume_scale);
    ClassDB::bind_method(D_METHOD("get_sound_volume_scale"), &GButton::get_sound_volume_scale);
    ClassDB::bind_method(D_METHOD("has_sound_stream"), &GButton::has_sound_stream);
    ClassDB::bind_method(D_METHOD("play_sound"), &GButton::play_sound);
    ClassDB::bind_method(D_METHOD("has_sound_player"), &GButton::has_sound_player);
    ClassDB::bind_method(D_METHOD("set_selected_title", "title"), &GButton::set_selected_title);
    ClassDB::bind_method(D_METHOD("get_selected_title"), &GButton::get_selected_title);
    ClassDB::bind_method(D_METHOD("set_icon", "icon"), &GButton::set_icon);
    ClassDB::bind_method(D_METHOD("get_icon"), &GButton::get_icon);
    ClassDB::bind_method(D_METHOD("set_selected_icon", "icon"), &GButton::set_selected_icon);
    ClassDB::bind_method(D_METHOD("get_selected_icon"), &GButton::get_selected_icon);
    ClassDB::bind_method(D_METHOD("set_title_color", "color"), &GButton::set_title_color);
    ClassDB::bind_method(D_METHOD("get_title_color"), &GButton::get_title_color);
    ClassDB::bind_method(D_METHOD("set_title_font_size", "size"), &GButton::set_title_font_size);
    ClassDB::bind_method(D_METHOD("get_title_font_size"), &GButton::get_title_font_size);
    ClassDB::bind_method(D_METHOD("set_related_controller_index", "controller_index"), &GButton::set_related_controller_index);
    ClassDB::bind_method(D_METHOD("get_related_controller_index"), &GButton::get_related_controller_index);
    ClassDB::bind_method(D_METHOD("set_related_page_id", "page_id"), &GButton::set_related_page_id);
    ClassDB::bind_method(D_METHOD("get_related_page_id"), &GButton::get_related_page_id);
    ClassDB::bind_method(D_METHOD("set_linked_popup", "popup"), &GButton::set_linked_popup);
    ClassDB::bind_method(D_METHOD("get_linked_popup"), &GButton::get_linked_popup);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sound_url"), "set_sound_url", "get_sound_url");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sound_volume_scale"), "set_sound_volume_scale", "get_sound_volume_scale");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "selected_title"), "set_selected_title", "get_selected_title");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "icon"), "set_icon", "get_icon");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "selected_icon"), "set_selected_icon", "get_selected_icon");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "title_color"), "set_title_color", "get_title_color");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "title_font_size"), "set_title_font_size", "get_title_font_size");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "related_controller_index"), "set_related_controller_index", "get_related_controller_index");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "related_page_id"), "set_related_page_id", "get_related_page_id");

    ADD_SIGNAL(MethodInfo("fgui_selected_changed", PropertyInfo(Variant::BOOL, "selected")));
}

void GButton::_gui_input(const Ref<InputEvent> &p_event) {
    GComponent::_gui_input(p_event);

    Ref<InputEventMouseButton> mouse_button = p_event;
    if (mouse_button.is_valid() && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT && !mouse_button->is_pressed()) {
        handle_click();
    }
}

void GButton::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_after_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 6)) {
        return;
    }

    mode = static_cast<ButtonMode>(p_buffer.read_byte());
    String value = p_buffer.read_s();
    if (!value.is_empty()) {
        set_title(value);
    }
    p_buffer.read_s(); // selected title
    p_buffer.read_s(); // icon
    p_buffer.read_s(); // selected icon
    if (p_buffer.read_bool()) {
        const Color title_color = p_buffer.read_color();
        (void)title_color;
    }
    p_buffer.read_int(); // title font size
    p_buffer.read_short(); // related controller
    p_buffer.read_s(); // related page id
    const String read_sound = p_buffer.read_s();
    if (!read_sound.is_empty()) {
        set_sound_url(read_sound);
    }
    if (p_buffer.read_bool()) {
        set_sound_volume_scale(p_buffer.read_float());
    }
    set_selected(p_buffer.read_bool());
}

void GButton::set_selected(bool p_selected) {
    if (selected == p_selected) return;
    selected = p_selected;
    emit_signal("fgui_selected_changed", selected);
}

bool GButton::is_selected() const {
    return selected;
}

void GButton::set_title(const String &p_title) {
    title = p_title;
}

String GButton::get_title() const {
    return title;
}

void GButton::handle_click() {
    if (mode == MODE_CHECK) {
        set_selected(!selected);
    } else if (mode == MODE_RADIO && !selected) {
        set_selected(true);
    }
    play_sound();
}

int32_t GButton::get_mode() const {
    return static_cast<int32_t>(mode);
}

void GButton::set_sound_url(const String &p_sound_url) {
    sound_url = p_sound_url;
}

String GButton::get_sound_url() const {
    return sound_url;
}

void GButton::set_sound_volume_scale(double p_sound_volume_scale) {
    sound_volume_scale = CLAMP(p_sound_volume_scale, 0.0, 1.0);
}

double GButton::get_sound_volume_scale() const {
    return sound_volume_scale;
}

bool GButton::has_sound_stream() const {
    if (sound_url.is_empty()) {
        return false;
    }
    return UIPackage::get_sound_audio_stream_by_url(sound_url).is_valid();
}

bool GButton::play_sound() {
    if (sound_url.is_empty() || sound_volume_scale <= 0.0 || !is_inside_tree()) {
        return false;
    }

    Ref<AudioStream> stream = UIPackage::get_sound_audio_stream_by_url(sound_url);
    if (stream.is_null()) {
        return false;
    }

    if (sound_player == nullptr) {
        sound_player = memnew(AudioStreamPlayer);
        sound_player->set_name("_SoundPlayer");
        sound_player->set_max_polyphony(8);
        add_child(sound_player);
    }
    sound_player->set_stream(stream);
    sound_player->set_volume_linear(static_cast<float>(sound_volume_scale));
    sound_player->play();
    return true;
}

bool GButton::has_sound_player() const {
    return sound_player != nullptr;
}

void GButton::set_selected_title(const String &p_title) { selected_title = p_title; }
String GButton::get_selected_title() const { return selected_title; }

void GButton::set_icon(const String &p_icon) {
    icon = p_icon;
    GLoader *loader = Object::cast_to<GLoader>(get_child_by_name("icon"));
    if (loader != nullptr) loader->set_url(p_icon);
}
String GButton::get_icon() const { return icon; }

void GButton::set_selected_icon(const String &p_icon) { selected_icon = p_icon; }
String GButton::get_selected_icon() const { return selected_icon; }

void GButton::set_title_color(const Color &p_color) { title_color = p_color; }
Color GButton::get_title_color() const { return title_color; }

void GButton::set_title_font_size(int32_t p_size) { title_font_size = p_size; }
int32_t GButton::get_title_font_size() const { return title_font_size; }

void GButton::set_related_controller_index(int32_t p_controller_index) { related_controller_index = p_controller_index; }
int32_t GButton::get_related_controller_index() const { return related_controller_index; }

void GButton::set_related_page_id(const String &p_page_id) { related_page_id = p_page_id; }
String GButton::get_related_page_id() const { return related_page_id; }

void GButton::set_linked_popup(GObject *p_popup) { linked_popup = p_popup; }
GObject *GButton::get_linked_popup() const { return linked_popup; }
