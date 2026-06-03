#pragma once

#include "g_object.h"

#include <godot_cpp/classes/texture2d.hpp>

namespace godot {

class GMovieClip : public GObject {
    GDCLASS(GMovieClip, GObject)

protected:
    static void _bind_methods();

public:
    GMovieClip();

    void _draw() override;
    void _process(double p_delta) override;
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_package_movie_clip(const String &p_package_id_or_name, const String &p_item_id_or_name);
    String get_package_name() const;
    String get_item_name() const;

    void set_playing(bool p_playing);
    bool is_playing() const;
    void set_frame(int32_t p_frame);
    int32_t get_frame() const;
    int32_t get_frame_count() const;
    void set_time_scale(double p_time_scale);
    double get_time_scale() const;
    void set_interval(double p_interval);
    double get_interval() const;
    void set_repeat_delay(double p_repeat_delay);
    double get_repeat_delay() const;
    void set_swing(bool p_swing);
    bool is_swing() const;

    void rewind();
    void advance(double p_time);
    void set_play_settings(int32_t p_start = 0, int32_t p_end = -1, int32_t p_times = 0, int32_t p_end_at = -1);
    Ref<Texture2D> get_current_texture() const;

private:
    Vector<fgui::MovieClipFrame> frames;
    String package_name;
    String item_name;
    Ref<Texture2D> current_texture;
    Rect2 current_rect;
    double interval = 0.1;
    double repeat_delay = 0.0;
    double time_scale = 1.0;
    double frame_elapsed = 0.0;
    int32_t current_frame = 0;
    int32_t start_frame = 0;
    int32_t end_frame = -1;
    int32_t remaining_times = 0;
    int32_t end_at_frame = -1;
    int32_t status = 0;
    int32_t repeated_count = 0;
    bool playing = true;
    bool swing = false;
    bool reversed = false;

    void update_process_state();
    void draw_current_frame();
    void step_frame();
};

} // namespace godot
