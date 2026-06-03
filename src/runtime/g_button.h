#pragma once

#include "g_component.h"

namespace godot {

class AudioStreamPlayer;

class GButton : public GComponent {
    GDCLASS(GButton, GComponent)

protected:
    static void _bind_methods();

public:
    enum ButtonMode {
        MODE_COMMON = 0,
        MODE_CHECK = 1,
        MODE_RADIO = 2,
    };

    void _gui_input(const Ref<InputEvent> &p_event) override;
    void setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_selected(bool p_selected);
    bool is_selected() const;

    void set_title(const String &p_title);
    String get_title() const;
    void handle_click();
    int32_t get_mode() const;
    void set_sound_url(const String &p_sound_url);
    String get_sound_url() const;
    void set_sound_volume_scale(double p_sound_volume_scale);
    double get_sound_volume_scale() const;
    bool has_sound_stream() const;
    bool play_sound();
    bool has_sound_player() const;

private:
    ButtonMode mode = MODE_COMMON;
    bool selected = false;
    String title;
    String sound_url;
    double sound_volume_scale = 1.0;
    AudioStreamPlayer *sound_player = nullptr;
};

} // namespace godot
