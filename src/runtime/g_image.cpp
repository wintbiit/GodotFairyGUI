#include "g_image.h"

#include "ui_package.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GImage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_texture", "texture"), &GImage::set_texture);
    ClassDB::bind_method(D_METHOD("get_texture"), &GImage::get_texture);
    ClassDB::bind_method(D_METHOD("set_package_image", "package_id_or_name", "item_id_or_name"), &GImage::set_package_image);
    ClassDB::bind_method(D_METHOD("set_package_name", "package_id_or_name"), &GImage::set_package_name);
    ClassDB::bind_method(D_METHOD("get_package_name"), &GImage::get_package_name);
    ClassDB::bind_method(D_METHOD("set_item_name", "item_id_or_name"), &GImage::set_item_name);
    ClassDB::bind_method(D_METHOD("get_item_name"), &GImage::get_item_name);
    ClassDB::bind_method(D_METHOD("set_scale9_grid", "grid"), &GImage::set_scale9_grid);
    ClassDB::bind_method(D_METHOD("get_scale9_grid"), &GImage::get_scale9_grid);
    ClassDB::bind_method(D_METHOD("clear_scale9_grid"), &GImage::clear_scale9_grid);
    ClassDB::bind_method(D_METHOD("has_scale9_grid"), &GImage::has_scale9_grid);
    ClassDB::bind_method(D_METHOD("has_pixel_hit_test"), &GImage::has_pixel_hit_test);
    ClassDB::bind_method(D_METHOD("hit_test_point", "point"), &GImage::hit_test_point);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "package_name"), "set_package_name", "get_package_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "item_name"), "set_item_name", "get_item_name");
    ADD_PROPERTY(PropertyInfo(Variant::RECT2, "scale9_grid"), "set_scale9_grid", "get_scale9_grid");
}

void GImage::_draw() {
    if (texture.is_null()) {
        return;
    }

    if (scale9_enabled && scale9_grid.size.x > 0.0f && scale9_grid.size.y > 0.0f) {
        draw_scale9();
    } else {
        draw_texture_rect(texture, Rect2(Vector2(), get_size()), false);
    }
}

bool GImage::_has_point(const Vector2 &p_point) const {
    return hit_test_point(p_point);
}

void GImage::set_texture(const Ref<Texture2D> &p_texture) {
    texture = p_texture;
    queue_redraw();
}

Ref<Texture2D> GImage::get_texture() const {
    return texture;
}

void GImage::set_package_image(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    package_name = p_package_id_or_name;
    item_name = p_item_id_or_name;
    set_texture(UIPackage::get_image_texture(package_name, item_name));
    if (UIPackage::has_image_scale9_grid(package_name, item_name)) {
        set_scale9_grid(UIPackage::get_image_scale9_grid(package_name, item_name));
    } else {
        clear_scale9_grid();
    }
}

void GImage::set_package_name(const String &p_package_id_or_name) {
    package_name = p_package_id_or_name;
    if (!item_name.is_empty()) {
        set_package_image(package_name, item_name);
    }
}

String GImage::get_package_name() const {
    return package_name;
}

void GImage::set_item_name(const String &p_item_id_or_name) {
    item_name = p_item_id_or_name;
    if (!package_name.is_empty()) {
        set_package_image(package_name, item_name);
    }
}

String GImage::get_item_name() const {
    return item_name;
}

void GImage::set_scale9_grid(const Rect2 &p_grid) {
    scale9_grid = p_grid;
    scale9_enabled = p_grid.size.x > 0.0f && p_grid.size.y > 0.0f;
    queue_redraw();
}

Rect2 GImage::get_scale9_grid() const {
    return scale9_grid;
}

void GImage::clear_scale9_grid() {
    scale9_grid = Rect2();
    scale9_enabled = false;
    queue_redraw();
}

bool GImage::has_scale9_grid() const {
    return scale9_enabled;
}

void GImage::set_pixel_hit_test_data(const fgui::PixelHitTestData &p_data, float p_source_width, float p_source_height) {
    pixel_hit_test_data = p_data;
    source_width = p_source_width;
    source_height = p_source_height;
    has_pixel_hit_test_data = true;
}

bool GImage::has_pixel_hit_test() const {
    return has_pixel_hit_test_data;
}

bool GImage::hit_test_point(const Vector2 &p_point) const {
    if (!Rect2(Vector2(), get_size()).has_point(p_point)) {
        return false;
    }

    if (!has_pixel_hit_test_data) {
        return true;
    }

    const float resolved_source_width = source_width > 0.0f ? source_width : get_size().x;
    const float resolved_source_height = source_height > 0.0f ? source_height : get_size().y;
    return pixel_hit_test_data.hit_test(p_point, Rect2(Vector2(), get_size()), 0, 0, resolved_source_width, resolved_source_height);
}

void GImage::draw_scale9() {
    const Vector2 texture_size = texture->get_size();
    const Vector2 target_size = get_size();
    if (texture_size.x <= 0.0f || texture_size.y <= 0.0f || target_size.x <= 0.0f || target_size.y <= 0.0f) {
        return;
    }

    const float left = CLAMP(scale9_grid.position.x, 0.0f, texture_size.x);
    const float top = CLAMP(scale9_grid.position.y, 0.0f, texture_size.y);
    const float right = CLAMP(texture_size.x - scale9_grid.position.x - scale9_grid.size.x, 0.0f, texture_size.x);
    const float bottom = CLAMP(texture_size.y - scale9_grid.position.y - scale9_grid.size.y, 0.0f, texture_size.y);

    const float dst_left = MIN(left, target_size.x * 0.5f);
    const float dst_right = MIN(right, target_size.x - dst_left);
    const float dst_top = MIN(top, target_size.y * 0.5f);
    const float dst_bottom = MIN(bottom, target_size.y - dst_top);

    const float src_x[4] = { 0.0f, left, texture_size.x - right, texture_size.x };
    const float src_y[4] = { 0.0f, top, texture_size.y - bottom, texture_size.y };
    const float dst_x[4] = { 0.0f, dst_left, target_size.x - dst_right, target_size.x };
    const float dst_y[4] = { 0.0f, dst_top, target_size.y - dst_bottom, target_size.y };

    for (int32_t row = 0; row < 3; row++) {
        for (int32_t col = 0; col < 3; col++) {
            const Rect2 src(Vector2(src_x[col], src_y[row]), Vector2(src_x[col + 1] - src_x[col], src_y[row + 1] - src_y[row]));
            const Rect2 dst(Vector2(dst_x[col], dst_y[row]), Vector2(dst_x[col + 1] - dst_x[col], dst_y[row + 1] - dst_y[row]));
            draw_texture_slice(dst, src);
        }
    }
}

void GImage::draw_texture_slice(const Rect2 &p_dst, const Rect2 &p_src) {
    if (p_dst.size.x <= 0.0f || p_dst.size.y <= 0.0f || p_src.size.x <= 0.0f || p_src.size.y <= 0.0f) {
        return;
    }
    draw_texture_rect_region(texture, p_dst, p_src);
}
