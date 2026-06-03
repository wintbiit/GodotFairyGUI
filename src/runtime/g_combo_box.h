#pragma once

#include "g_component.h"

namespace godot {

class GComboBox : public GComponent {
    GDCLASS(GComboBox, GComponent)

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
    void set_selected_index(int32_t p_index);
    int32_t get_selected_index() const;
    void set_value(const String &p_value);
    String get_value() const;
    void set_visible_item_count(int32_t p_count);
    int32_t get_visible_item_count() const;
    int32_t get_popup_direction() const;
    String get_dropdown_url() const;
    bool has_dropdown() const;

    void clear_items();
    void add_item(const String &p_text, const String &p_value = String(), const String &p_icon = String());
    void apply_list_change();
    int32_t get_item_count() const;
    String get_item_text(int32_t p_index) const;
    String get_item_value(int32_t p_index) const;
    String get_item_icon(int32_t p_index) const;

private:
    GObject *title_object = nullptr;
    GObject *icon_object = nullptr;
    GComponent *dropdown = nullptr;
    String dropdown_url;
    Vector<String> items;
    Vector<String> values;
    Vector<String> icons;
    int32_t selected_index = -1;
    int32_t visible_item_count = 10;
    int32_t popup_direction = 0;
    String title;
    String icon;
    Color title_color = Color(0, 0, 0, 1);

    void refresh_child_refs();
    void construct_extension(fgui::ByteBuffer &p_buffer);
    void apply_title_to_object();
    void apply_icon_to_object();
    void update_selection(bool p_emit_signal);
    class GTextField *get_text_field() const;
};

} // namespace godot
