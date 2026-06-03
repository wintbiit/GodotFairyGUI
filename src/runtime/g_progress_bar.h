#pragma once

#include "g_component.h"

namespace godot {

class GProgressBar : public GComponent {
    GDCLASS(GProgressBar, GComponent)

protected:
    static void _bind_methods();
    void construct_extension_from_package(const fgui::PackageItem &p_item) override;

public:
    void _notification(int p_what);
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

    double get_percent() const;
    String get_title_text() const;

private:
    double min = 0.0;
    double max = 100.0;
    double value = 50.0;
    fgui::ProgressTitleType title_type = fgui::ProgressTitleType::Percent;
    bool reverse = false;

    GObject *title_object = nullptr;
    GObject *bar = nullptr;
    GObject *bar_v = nullptr;
    double bar_max_width = 0.0;
    double bar_max_height = 0.0;
    double bar_start_x = 0.0;
    double bar_start_y = 0.0;
    double bar_width_delta = 0.0;
    double bar_height_delta = 0.0;
    String title_text;

    void construct_extension(fgui::ByteBuffer &p_buffer);
    void refresh_children();
    void update_progress();
    void set_title_object_text(const String &p_text);
};

} // namespace godot
