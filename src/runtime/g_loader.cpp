#include "g_loader.h"

#include "g_movie_clip.h"
#include "ui_package.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GLoader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_url", "url"), &GLoader::set_url);
    ClassDB::bind_method(D_METHOD("get_url"), &GLoader::get_url);
    ClassDB::bind_method(D_METHOD("get_texture"), &GLoader::get_texture);
    ClassDB::bind_method(D_METHOD("has_content_node"), &GLoader::has_content_node);
    ClassDB::bind_method(D_METHOD("set_align", "align"), &GLoader::set_align);
    ClassDB::bind_method(D_METHOD("get_align"), &GLoader::get_align);
    ClassDB::bind_method(D_METHOD("set_vertical_align", "vertical_align"), &GLoader::set_vertical_align);
    ClassDB::bind_method(D_METHOD("get_vertical_align"), &GLoader::get_vertical_align);
    ClassDB::bind_method(D_METHOD("set_fill", "fill"), &GLoader::set_fill);
    ClassDB::bind_method(D_METHOD("get_fill"), &GLoader::get_fill);
    ClassDB::bind_method(D_METHOD("set_shrink_only", "shrink_only"), &GLoader::set_shrink_only);
    ClassDB::bind_method(D_METHOD("is_shrink_only"), &GLoader::is_shrink_only);
    ClassDB::bind_method(D_METHOD("set_auto_size", "auto_size"), &GLoader::set_auto_size);
    ClassDB::bind_method(D_METHOD("is_auto_size"), &GLoader::is_auto_size);
    ClassDB::bind_method(D_METHOD("set_use_resize", "use_resize"), &GLoader::set_use_resize);
    ClassDB::bind_method(D_METHOD("is_use_resize"), &GLoader::is_use_resize);
    ClassDB::bind_method(D_METHOD("get_content_rect"), &GLoader::get_content_rect);
    ClassDB::bind_method(D_METHOD("set_playing", "playing"), &GLoader::set_playing);
    ClassDB::bind_method(D_METHOD("is_playing"), &GLoader::is_playing);
    ClassDB::bind_method(D_METHOD("get_frame"), &GLoader::get_frame);
    ClassDB::bind_method(D_METHOD("set_frame", "frame"), &GLoader::set_frame);
    ClassDB::bind_method(D_METHOD("get_time_scale"), &GLoader::get_time_scale);
    ClassDB::bind_method(D_METHOD("set_time_scale", "time_scale"), &GLoader::set_time_scale);
    ClassDB::bind_method(D_METHOD("advance", "time"), &GLoader::advance);
    ClassDB::bind_method(D_METHOD("set_color", "color"), &GLoader::set_color);
    ClassDB::bind_method(D_METHOD("get_color"), &GLoader::get_color);
    ClassDB::bind_method(D_METHOD("set_show_error_sign", "show_error_sign"), &GLoader::set_show_error_sign);
    ClassDB::bind_method(D_METHOD("is_show_error_sign"), &GLoader::is_show_error_sign);
    ClassDB::bind_method(D_METHOD("set_fill_method", "fill_method"), &GLoader::set_fill_method);
    ClassDB::bind_method(D_METHOD("get_fill_method"), &GLoader::get_fill_method);
    ClassDB::bind_method(D_METHOD("set_fill_origin", "fill_origin"), &GLoader::set_fill_origin);
    ClassDB::bind_method(D_METHOD("get_fill_origin"), &GLoader::get_fill_origin);
    ClassDB::bind_method(D_METHOD("set_fill_clockwise", "fill_clockwise"), &GLoader::set_fill_clockwise);
    ClassDB::bind_method(D_METHOD("is_fill_clockwise"), &GLoader::is_fill_clockwise);
    ClassDB::bind_method(D_METHOD("set_fill_amount", "fill_amount"), &GLoader::set_fill_amount);
    ClassDB::bind_method(D_METHOD("get_fill_amount"), &GLoader::get_fill_amount);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "url"), "set_url", "get_url");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "align"), "set_align", "get_align");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_align"), "set_vertical_align", "get_vertical_align");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fill"), "set_fill", "get_fill");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shrink_only"), "set_shrink_only", "is_shrink_only");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_size"), "set_auto_size", "is_auto_size");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_resize"), "set_use_resize", "is_use_resize");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing"), "set_playing", "is_playing");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "set_frame", "get_frame");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_scale"), "set_time_scale", "get_time_scale");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_error_sign"), "set_show_error_sign", "is_show_error_sign");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_method"), "set_fill_method", "get_fill_method");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fill_origin"), "set_fill_origin", "get_fill_origin");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fill_clockwise"), "set_fill_clockwise", "is_fill_clockwise");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fill_amount"), "set_fill_amount", "get_fill_amount");
}

void GLoader::_draw() {
    if (texture.is_null()) {
        return;
    }
    draw_texture_rect(texture, content_rect, false);
}

void GLoader::_notification(int p_what) {
    if (p_what == NOTIFICATION_RESIZED) {
        update_layout();
    }
}

void GLoader::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GObject::setup_before_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 5)) {
        return;
    }

    url = p_buffer.read_s();
    align = p_buffer.read_byte();
    vertical_align = p_buffer.read_byte();
    fill = p_buffer.read_byte();
    shrink_only = p_buffer.read_bool();
    auto_size = p_buffer.read_bool();
    p_buffer.read_bool(); // show error sign
    p_buffer.read_bool(); // playing
    p_buffer.read_int(); // frame
    if (p_buffer.read_bool()) {
        p_buffer.read_color();
    }
    const uint8_t fill_method = p_buffer.read_byte();
    if (fill_method != 0) {
        p_buffer.read_byte();
        p_buffer.read_bool();
        p_buffer.read_float();
    }
    if (p_buffer.version >= 7) {
        use_resize = p_buffer.read_bool();
    }

    load_content();
}

void GLoader::set_url(const String &p_url) {
    url = p_url;
    load_content();
}

String GLoader::get_url() const {
    return url;
}

Ref<Texture2D> GLoader::get_texture() const {
    return texture;
}

bool GLoader::has_content_node() const {
    return content_node != nullptr;
}

void GLoader::set_align(int32_t p_align) {
    align = CLAMP(p_align, 0, 2);
    update_layout();
}

int32_t GLoader::get_align() const {
    return align;
}

void GLoader::set_vertical_align(int32_t p_vertical_align) {
    vertical_align = CLAMP(p_vertical_align, 0, 2);
    update_layout();
}

int32_t GLoader::get_vertical_align() const {
    return vertical_align;
}

void GLoader::set_fill(int32_t p_fill) {
    fill = CLAMP(p_fill, 0, 5);
    update_layout();
}

int32_t GLoader::get_fill() const {
    return fill;
}

void GLoader::set_shrink_only(bool p_shrink_only) {
    shrink_only = p_shrink_only;
    update_layout();
}

bool GLoader::is_shrink_only() const {
    return shrink_only;
}

void GLoader::set_auto_size(bool p_auto_size) {
    auto_size = p_auto_size;
    update_layout();
}

bool GLoader::is_auto_size() const {
    return auto_size;
}

void GLoader::set_use_resize(bool p_use_resize) {
    use_resize = p_use_resize;
    update_layout();
}

bool GLoader::is_use_resize() const {
    return use_resize;
}

Rect2 GLoader::get_content_rect() const {
    return content_rect;
}

void GLoader::load_content() {
    clear_content();
    if (url.is_empty()) {
        return;
    }

    if (url.begins_with("ui://")) {
        texture = UIPackage::get_image_texture_by_url(url);
        if (texture.is_valid()) {
            update_layout();
            return;
        }

        content_node = UIPackage::create_object_from_url(url);
        if (content_node != nullptr) {
            add_child(content_node);
            update_layout();
        }
        return;
    }

    texture = UIPackage::get_resource_texture(url);
    update_layout();
}

void GLoader::update_layout() {
    if (updating_layout) {
        return;
    }

    Vector2 source_size;
    if (texture.is_valid()) {
        source_size = texture->get_size();
    } else if (content_node != nullptr) {
        source_size = content_node->get_size();
        if (source_size == Vector2()) {
            source_size = content_node->get_custom_minimum_size();
        }
    }

    if (source_size.x <= 0.0f) {
        source_size.x = 50.0f;
    }
    if (source_size.y <= 0.0f) {
        source_size.y = 30.0f;
    }

    if (auto_size) {
        updating_layout = true;
        set_custom_minimum_size(source_size);
        set_size(source_size);
        updating_layout = false;
    }

    const Vector2 loader_size = get_size();
    Vector2 content_size = source_size;
    float sx = 1.0f;
    float sy = 1.0f;

    if (fill != 0 && source_size.x > 0.0f && source_size.y > 0.0f) {
        sx = loader_size.x / source_size.x;
        sy = loader_size.y / source_size.y;

        if (fill == 2) {
            sx = sy;
        } else if (fill == 3) {
            sy = sx;
        } else if (fill == 1) {
            const float scale = MIN(sx, sy);
            sx = scale;
            sy = scale;
        } else if (fill == 5) {
            const float scale = MAX(sx, sy);
            sx = scale;
            sy = scale;
        }

        if (shrink_only) {
            sx = MIN(sx, 1.0f);
            sy = MIN(sy, 1.0f);
        }

        content_size = Vector2(source_size.x * sx, source_size.y * sy);
    }

    Vector2 content_position;
    if (align == 1) {
        content_position.x = (loader_size.x - content_size.x) * 0.5f;
    } else if (align == 2) {
        content_position.x = loader_size.x - content_size.x;
    }

    if (vertical_align == 1) {
        content_position.y = (loader_size.y - content_size.y) * 0.5f;
    } else if (vertical_align == 2) {
        content_position.y = loader_size.y - content_size.y;
    }

    content_rect = Rect2(content_position, content_size);
    if (content_node != nullptr) {
        content_node->set_position(content_position);
        if (use_resize) {
            content_node->set_scale(Vector2(1, 1));
            content_node->set_size(content_size);
        } else {
            content_node->set_size(source_size);
            content_node->set_scale(Vector2(sx, sy));
        }
    }
    queue_redraw();
}

void GLoader::clear_content() {
    texture = Ref<Texture2D>();
    content_rect = Rect2();
    if (content_node != nullptr) {
        remove_child(content_node);
        content_node->queue_free();
        content_node = nullptr;
    }
    queue_redraw();
}

void GLoader::set_playing(bool p_playing) {
    playing = p_playing;
    GMovieClip *mc = Object::cast_to<GMovieClip>(content_node);
    if (mc != nullptr) mc->set_playing(p_playing);
}
bool GLoader::is_playing() const { return playing; }

int32_t GLoader::get_frame() const {
    GMovieClip *mc = Object::cast_to<GMovieClip>(content_node);
    return mc != nullptr ? mc->get_frame() : frame;
}
void GLoader::set_frame(int32_t p_frame) {
    frame = p_frame;
    GMovieClip *mc = Object::cast_to<GMovieClip>(content_node);
    if (mc != nullptr) mc->set_frame(p_frame);
}

float GLoader::get_time_scale() const { return time_scale; }
void GLoader::set_time_scale(float p_time_scale) {
    time_scale = p_time_scale;
    GMovieClip *mc = Object::cast_to<GMovieClip>(content_node);
    if (mc != nullptr) mc->set_time_scale(p_time_scale);
}

void GLoader::advance(float p_time) {
    GMovieClip *mc = Object::cast_to<GMovieClip>(content_node);
    if (mc != nullptr) mc->advance(p_time);
}

void GLoader::set_color(const Color &p_color) { color = p_color; queue_redraw(); }
Color GLoader::get_color() const { return color; }

void GLoader::set_show_error_sign(bool p_show_error_sign) { show_error_sign = p_show_error_sign; }
bool GLoader::is_show_error_sign() const { return show_error_sign; }

void GLoader::set_fill_method(int32_t p_fill_method) { fill_method = p_fill_method; queue_redraw(); }
int32_t GLoader::get_fill_method() const { return fill_method; }
void GLoader::set_fill_origin(int32_t p_fill_origin) { fill_origin = p_fill_origin; queue_redraw(); }
int32_t GLoader::get_fill_origin() const { return fill_origin; }
void GLoader::set_fill_clockwise(bool p_fill_clockwise) { fill_clockwise = p_fill_clockwise; queue_redraw(); }
bool GLoader::is_fill_clockwise() const { return fill_clockwise; }
void GLoader::set_fill_amount(float p_fill_amount) { fill_amount = p_fill_amount; queue_redraw(); }
float GLoader::get_fill_amount() const { return fill_amount; }
