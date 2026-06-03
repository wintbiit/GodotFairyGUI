#pragma once

#include "g_component.h"

namespace godot {

class GSlider : public GComponent {
    GDCLASS(GSlider, GComponent)

protected:
    static void _bind_methods();
    void construct_extension_from_package(const fgui::PackageItem &p_item) override;

public:
    void _notification(int p_what);
    void _gui_input(const Ref<InputEvent> &p_event) override;
    void setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_min(double p_min);
    double get_min() const;
    void set_max(double p_max);
    double get_max() const;
    void set_value(double p_value);
    double get_value() const;
    void set_title_type(int32_t p_title_type);
    int32_t get_title_type() const;
    void set_reverse(bool p_reverse);
    bool is_reverse() const;
    void set_whole_numbers(bool p_whole_numbers);
    bool is_whole_numbers() const;
    void set_change_on_click(bool p_change_on_click);
    bool is_change_on_click() const;
    void set_can_drag(bool p_can_drag);
    bool can_drag() const;

    double get_percent() const;
    String get_title_text() const;
    bool update_with_percent(double p_percent, bool p_manual);

private:
    double min = 0.0;
    double max = 100.0;
    double value = 50.0;
    fgui::ProgressTitleType title_type = fgui::ProgressTitleType::Percent;
    bool reverse = false;
    bool whole_numbers = false;
    bool change_on_click = true;
    bool drag_enabled = true;

    GObject *title_object = nullptr;
    GObject *bar = nullptr;
    GObject *bar_v = nullptr;
    GObject *grip = nullptr;
    double bar_max_width = 0.0;
    double bar_max_height = 0.0;
    double bar_start_x = 0.0;
    double bar_start_y = 0.0;
    double bar_width_delta = 0.0;
    double bar_height_delta = 0.0;
    String title_text;

    void construct_extension(fgui::ByteBuffer &p_buffer);
    void refresh_children();
    void update_slider();
    void set_title_object_text(const String &p_text);
};

} // namespace godot
