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

    void load_content();
    void update_layout();
    void clear_content();
};

} // namespace godot
