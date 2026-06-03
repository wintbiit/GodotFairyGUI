#include "g_button.h"

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

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "selected"), "set_selected", "is_selected");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "sound_url"), "set_sound_url", "get_sound_url");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sound_volume_scale"), "set_sound_volume_scale", "get_sound_volume_scale");
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
    selected = p_selected;
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
