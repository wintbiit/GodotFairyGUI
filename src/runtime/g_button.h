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
    void set_selected_title(const String &p_title);
    String get_selected_title() const;
    void set_icon(const String &p_icon);
    String get_icon() const;
    void set_selected_icon(const String &p_icon);
    String get_selected_icon() const;

    void set_title_color(const Color &p_color);
    Color get_title_color() const;
    void set_title_font_size(int32_t p_size);
    int32_t get_title_font_size() const;

    void handle_click();
    int32_t get_mode() const;

    void set_related_controller_index(int32_t p_controller_index);
    int32_t get_related_controller_index() const;
    void set_related_page_id(const String &p_page_id);
    String get_related_page_id() const;

    void set_linked_popup(GObject *p_popup);
    GObject *get_linked_popup() const;

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
    String selected_title;
    String icon;
    String selected_icon;
    Color title_color = Color(0, 0, 0, 1);
    int32_t title_font_size = 0;
    int32_t related_controller_index = -1;
    String related_page_id;
    GObject *linked_popup = nullptr;
    String sound_url;
    double sound_volume_scale = 1.0;
    AudioStreamPlayer *sound_player = nullptr;
};

} // namespace godot
