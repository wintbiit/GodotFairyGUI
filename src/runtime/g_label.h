#pragma once

#include "g_component.h"

namespace godot {

class GLabel : public GComponent {
    GDCLASS(GLabel, GComponent)

protected:
    static void _bind_methods();
    void construct_extension_from_package(const fgui::PackageItem &p_item) override;

public:
    void setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_title(const String &p_title);
    String get_title() const;
    void set_text(const String &p_text);
    String get_text() const;

    void set_icon(const String &p_icon);
    String get_icon() const;
    void set_title_color(const Color &p_color);
    Color get_title_color() const;
    void set_title_font_size(int32_t p_size);
    int32_t get_title_font_size() const;
    class GTextField *get_text_field() const;
    void set_editable(bool p_editable);
    bool is_editable() const;
    void set_prompt_text(const String &p_prompt_text);
    String get_prompt_text() const;
    void set_restrict(const String &p_restrict);
    String get_restrict() const;
    void set_max_length(int32_t p_max_length);
    int32_t get_max_length() const;
    void set_display_as_password(bool p_display_as_password);
    bool is_display_as_password() const;

private:
    GObject *title_object = nullptr;
    GObject *icon_object = nullptr;
    String title;
    String icon;
    Color title_color = Color(0, 0, 0, 1);
    int32_t title_font_size = 0;

    void refresh_child_refs();
    void apply_title_to_object();
    void apply_icon_to_object();
};

} // namespace godot
