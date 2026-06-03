#pragma once

#include "g_object.h"

#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/rect2.hpp>

namespace godot {

class GLoader : public GObject {
    GDCLASS(GLoader, GObject)

protected:
    static void _bind_methods();

public:
    void _draw() override;
    void _notification(int p_what);
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    void set_url(const String &p_url);
    String get_url() const;
    Ref<Texture2D> get_texture() const;
    bool has_content_node() const;
    void set_align(int32_t p_align);
    int32_t get_align() const;
    void set_vertical_align(int32_t p_vertical_align);
    int32_t get_vertical_align() const;
    void set_fill(int32_t p_fill);
    int32_t get_fill() const;
    void set_shrink_only(bool p_shrink_only);
    bool is_shrink_only() const;
    void set_auto_size(bool p_auto_size);
    bool is_auto_size() const;
    void set_use_resize(bool p_use_resize);
    bool is_use_resize() const;
    Rect2 get_content_rect() const;

    void set_playing(bool p_playing);
    bool is_playing() const;
    int32_t get_frame() const;
    void set_frame(int32_t p_frame);
    float get_time_scale() const;
    void set_time_scale(float p_time_scale);
    void advance(float p_time);

    void set_color(const Color &p_color);
    Color get_color() const;
    void set_show_error_sign(bool p_show_error_sign);
    bool is_show_error_sign() const;

    void set_fill_method(int32_t p_fill_method);
    int32_t get_fill_method() const;
    void set_fill_origin(int32_t p_fill_origin);
    int32_t get_fill_origin() const;
    void set_fill_clockwise(bool p_fill_clockwise);
    bool is_fill_clockwise() const;
    void set_fill_amount(float p_fill_amount);
    float get_fill_amount() const;

private:
    String url;
    Ref<Texture2D> texture;
    GObject *content_node = nullptr;
    int32_t align = 0;
    int32_t vertical_align = 0;
    int32_t fill = 0;
    bool shrink_only = false;
    bool auto_size = false;
    bool use_resize = false;
    Rect2 content_rect;
    bool updating_layout = false;

    bool playing = false;
    int32_t frame = -1;
    float time_scale = 1.0f;
    Color color = Color(1, 1, 1, 1);
    bool show_error_sign = true;

    int32_t fill_method = 0;
    int32_t fill_origin = 0;
    bool fill_clockwise = true;
    float fill_amount = 1.0f;

    void load_content();
    void update_layout();
    void clear_content();
};

} // namespace godot
