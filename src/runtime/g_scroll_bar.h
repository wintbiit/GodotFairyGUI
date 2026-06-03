#pragma once

#include "g_component.h"

namespace godot {

class GScrollBar : public GComponent {
    GDCLASS(GScrollBar, GComponent)

protected:
    static void _bind_methods();
    void construct_extension_from_package(const fgui::PackageItem &p_item) override;

public:
    void _gui_input(const Ref<InputEvent> &p_event) override;

    void set_vertical(bool p_vertical);
    bool is_vertical() const;
    void set_fixed_grip_size(bool p_fixed);
    bool is_fixed_grip_size() const;
    void set_display_perc(double p_value);
    double get_display_perc() const;
    void set_scroll_perc(double p_value);
    double get_scroll_perc() const;
    double get_min_size() const;
    bool is_grip_dragging() const;
    bool has_grip() const;
    bool has_bar() const;
    bool has_arrow1() const;
    bool has_arrow2() const;

private:
    GObject *grip = nullptr;
    GObject *bar = nullptr;
    GObject *arrow1 = nullptr;
    GObject *arrow2 = nullptr;
    bool vertical = true;
    bool fixed_grip_size = false;
    bool grip_dragging = false;
    double display_perc = 1.0;
    double scroll_perc = 0.0;

    void construct_extension(fgui::ByteBuffer &p_buffer);
    void refresh_children();
    void infer_vertical();
    void update_grip();
    void emit_bar_click(const Vector2 &p_local_position);
};

} // namespace godot
