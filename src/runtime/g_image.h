#pragma once

#include "g_object.h"
#include "package/package_defs.h"

#include <godot_cpp/classes/texture2d.hpp>

namespace godot {

class GImage : public GObject {
    GDCLASS(GImage, GObject)

protected:
    static void _bind_methods();

public:
    void _draw() override;
    bool _has_point(const Vector2 &p_point) const override;

    void set_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_texture() const;

    void set_package_image(const String &p_package_id_or_name, const String &p_item_id_or_name);
    void set_package_name(const String &p_package_id_or_name);
    String get_package_name() const;
    void set_item_name(const String &p_item_id_or_name);
    String get_item_name() const;
    void set_scale9_grid(const Rect2 &p_grid);
    Rect2 get_scale9_grid() const;
    void clear_scale9_grid();
    bool has_scale9_grid() const;
    void set_pixel_hit_test_data(const fgui::PixelHitTestData &p_data, float p_source_width, float p_source_height);
    bool has_pixel_hit_test() const;
    bool hit_test_point(const Vector2 &p_point) const;

private:
    Ref<Texture2D> texture;
    String package_name;
    String item_name;
    bool scale9_enabled = false;
    Rect2 scale9_grid;
    bool has_pixel_hit_test_data = false;
    fgui::PixelHitTestData pixel_hit_test_data;
    float source_width = 0.0f;
    float source_height = 0.0f;

    void draw_scale9();
    void draw_texture_slice(const Rect2 &p_dst, const Rect2 &p_src);
};

} // namespace godot
