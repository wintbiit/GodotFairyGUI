#pragma once

#include "g_text_field.h"

namespace godot {

class LineEdit;
class TextEdit;

class GTextInput : public GTextField {
    GDCLASS(GTextInput, GTextField)

protected:
    static void _bind_methods();

public:
    void _draw() override;
    void _notification(int p_what);
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_text(const String &p_text) override;
    String get_text() const override;
    void set_text_color(const Color &p_color) override;
    void set_font_size(int32_t p_size) override;

    void set_editable(bool p_editable);
    bool is_editable() const;
    void set_prompt_text(const String &p_prompt_text);
    String get_prompt_text() const;
    void set_restrict(const String &p_restrict);
    String get_restrict() const;
    void set_max_length(int32_t p_max_length);
    int32_t get_max_length() const;
    void set_keyboard_type(int32_t p_keyboard_type);
    int32_t get_keyboard_type() const;
    void set_display_as_password(bool p_display_as_password);
    bool is_display_as_password() const;
    void set_multiline(bool p_multiline);
    bool is_multiline() const;
    void set_hide_input(bool p_hide_input);
    bool is_hide_input() const;
    void set_keyboard_input(bool p_keyboard_input);
    bool is_keyboard_input() const;
    void set_disable_ime(bool p_disable_ime);
    bool is_disable_ime() const;
    void set_mouse_wheel_enabled(bool p_mouse_wheel_enabled);
    bool is_mouse_wheel_enabled() const;
    void set_caret_position(int32_t p_position);
    int32_t get_caret_position() const;
    void set_selection(int32_t p_start, int32_t p_length);
    void replace_selection(const String &p_value);
    bool has_native_input();
    int64_t get_native_input_owner_instance_id();

private:
    LineEdit *line_edit = nullptr;
    TextEdit *text_edit = nullptr;
    bool editable = true;
    bool multiline = false;
    bool display_as_password = false;
    bool hide_input = false;
    bool keyboard_input = true;
    bool disable_ime = false;
    bool text_input_mouse_wheel = false;
    String prompt_text;
    String restrict;
    int32_t max_length = 0;
    int32_t keyboard_type = 0;
    int32_t selection_start = 0;
    int32_t selection_length = 0;
    bool syncing_native = false;

    void update_native_input();
    void clear_native_input();
    Control *get_native_control() const;
    String apply_restrict_and_length(const String &p_value) const;
    bool is_restrict_allowed(char32_t p_char) const;
    void sync_native_text();
    void _handle_line_text_changed(const String &p_value);
    void _handle_text_edit_changed();
    void _handle_line_text_submitted(const String &p_value);
};

} // namespace godot
