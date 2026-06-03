#include "g_movie_clip.h"

#include "ui_package.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GMovieClip::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_package_movie_clip", "package_id_or_name", "item_id_or_name"), &GMovieClip::set_package_movie_clip);
    ClassDB::bind_method(D_METHOD("get_package_name"), &GMovieClip::get_package_name);
    ClassDB::bind_method(D_METHOD("get_item_name"), &GMovieClip::get_item_name);
    ClassDB::bind_method(D_METHOD("set_playing", "playing"), &GMovieClip::set_playing);
    ClassDB::bind_method(D_METHOD("is_playing"), &GMovieClip::is_playing);
    ClassDB::bind_method(D_METHOD("set_frame", "frame"), &GMovieClip::set_frame);
    ClassDB::bind_method(D_METHOD("get_frame"), &GMovieClip::get_frame);
    ClassDB::bind_method(D_METHOD("get_frame_count"), &GMovieClip::get_frame_count);
    ClassDB::bind_method(D_METHOD("set_time_scale", "time_scale"), &GMovieClip::set_time_scale);
    ClassDB::bind_method(D_METHOD("get_time_scale"), &GMovieClip::get_time_scale);
    ClassDB::bind_method(D_METHOD("set_interval", "interval"), &GMovieClip::set_interval);
    ClassDB::bind_method(D_METHOD("get_interval"), &GMovieClip::get_interval);
    ClassDB::bind_method(D_METHOD("set_repeat_delay", "repeat_delay"), &GMovieClip::set_repeat_delay);
    ClassDB::bind_method(D_METHOD("get_repeat_delay"), &GMovieClip::get_repeat_delay);
    ClassDB::bind_method(D_METHOD("set_swing", "swing"), &GMovieClip::set_swing);
    ClassDB::bind_method(D_METHOD("is_swing"), &GMovieClip::is_swing);
    ClassDB::bind_method(D_METHOD("rewind"), &GMovieClip::rewind);
    ClassDB::bind_method(D_METHOD("advance", "time"), &GMovieClip::advance);
    ClassDB::bind_method(D_METHOD("set_play_settings", "start", "end", "times", "end_at"), &GMovieClip::set_play_settings, DEFVAL(0), DEFVAL(-1), DEFVAL(0), DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("get_current_texture"), &GMovieClip::get_current_texture);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "package_name"), "", "get_package_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "item_name"), "", "get_item_name");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "playing"), "set_playing", "is_playing");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "set_frame", "get_frame");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "time_scale"), "set_time_scale", "get_time_scale");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "interval"), "set_interval", "get_interval");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "repeat_delay"), "set_repeat_delay", "get_repeat_delay");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "swing"), "set_swing", "is_swing");

    ADD_SIGNAL(MethodInfo("fgui_play_end"));
}

GMovieClip::GMovieClip() {
    set_process(false);
}

void GMovieClip::_draw() {
    if (current_texture.is_null()) {
        return;
    }
    const Rect2 target = current_rect.size == Vector2() ? Rect2(Vector2(), get_size()) : current_rect;
    draw_texture_rect(current_texture, target, false);
}

void GMovieClip::_process(double p_delta) {
    if (!playing || frames.is_empty() || status == 3) {
        update_process_state();
        return;
    }

    frame_elapsed += p_delta * time_scale;
    double frame_time = interval + frames[current_frame].add_delay;
    if (current_frame == 0 && repeated_count > 0) {
        frame_time += repeat_delay;
    }
    if (frame_elapsed < frame_time) {
        return;
    }

    frame_elapsed -= frame_time;
    if (frame_elapsed > interval) {
        frame_elapsed = interval;
    }

    step_frame();

    if (status == 1) {
        current_frame = start_frame;
        frame_elapsed = 0.0;
        status = 0;
        draw_current_frame();
    } else if (status == 2) {
        current_frame = end_at_frame;
        frame_elapsed = 0.0;
        status = 3;
        draw_current_frame();
        emit_signal("fgui_play_end");
    } else {
        draw_current_frame();
        if (current_frame == end_frame) {
            if (remaining_times > 0) {
                remaining_times--;
                status = remaining_times == 0 ? 2 : 1;
            } else if (start_frame != 0) {
                status = 1;
            }
        }
    }

    update_process_state();
}

void GMovieClip::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GObject::setup_before_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 5)) {
        return;
    }

    if (p_buffer.read_bool()) {
        set_modulate(p_buffer.read_color());
    }
    p_buffer.read_byte(); // flip
    set_frame(p_buffer.read_int());
    set_playing(p_buffer.read_bool());
}

void GMovieClip::set_package_movie_clip(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    package_name = p_package_id_or_name;
    item_name = p_item_id_or_name;
    frames = UIPackage::get_movie_clip_frames(package_name, item_name);
    interval = UIPackage::get_movie_clip_interval(package_name, item_name);
    repeat_delay = UIPackage::get_movie_clip_repeat_delay(package_name, item_name);
    swing = UIPackage::get_movie_clip_swing(package_name, item_name);
    current_frame = 0;
    set_play_settings();
    draw_current_frame();
    update_process_state();
}

String GMovieClip::get_package_name() const {
    return package_name;
}

String GMovieClip::get_item_name() const {
    return item_name;
}

void GMovieClip::set_playing(bool p_playing) {
    playing = p_playing;
    update_process_state();
}

bool GMovieClip::is_playing() const {
    return playing;
}

void GMovieClip::set_frame(int32_t p_frame) {
    if (frames.is_empty()) {
        current_frame = MAX(0, p_frame);
        return;
    }
    current_frame = CLAMP(p_frame, 0, frames.size() - 1);
    frame_elapsed = 0.0;
    draw_current_frame();
}

int32_t GMovieClip::get_frame() const {
    return current_frame;
}

int32_t GMovieClip::get_frame_count() const {
    return frames.size();
}

void GMovieClip::set_time_scale(double p_time_scale) {
    time_scale = p_time_scale;
}

double GMovieClip::get_time_scale() const {
    return time_scale;
}

void GMovieClip::set_interval(double p_interval) {
    interval = MAX(0.0, p_interval);
}

double GMovieClip::get_interval() const {
    return interval;
}

void GMovieClip::set_repeat_delay(double p_repeat_delay) {
    repeat_delay = MAX(0.0, p_repeat_delay);
}

double GMovieClip::get_repeat_delay() const {
    return repeat_delay;
}

void GMovieClip::set_swing(bool p_swing) {
    swing = p_swing;
}

bool GMovieClip::is_swing() const {
    return swing;
}

void GMovieClip::rewind() {
    current_frame = 0;
    frame_elapsed = 0.0;
    reversed = false;
    repeated_count = 0;
    status = 0;
    draw_current_frame();
    update_process_state();
}

void GMovieClip::advance(double p_time) {
    if (frames.is_empty() || p_time <= 0.0) {
        return;
    }

    const int32_t begin_frame = current_frame;
    const bool begin_reversed = reversed;
    const double backup_time = p_time;
    while (true) {
        double frame_time = interval + frames[current_frame].add_delay;
        if (current_frame == 0 && repeated_count > 0) {
            frame_time += repeat_delay;
        }
        if (p_time < frame_time) {
            frame_elapsed = 0.0;
            break;
        }
        p_time -= frame_time;
        step_frame();
        if (current_frame == begin_frame && reversed == begin_reversed) {
            const double round_time = backup_time - p_time;
            if (round_time > 0.0) {
                p_time -= Math::floor(p_time / round_time) * round_time;
            }
        }
    }
    draw_current_frame();
}

void GMovieClip::set_play_settings(int32_t p_start, int32_t p_end, int32_t p_times, int32_t p_end_at) {
    start_frame = MAX(0, p_start);
    end_frame = p_end;
    if (frames.is_empty()) {
        end_frame = -1;
        end_at_frame = -1;
    } else {
        if (start_frame > frames.size() - 1) {
            start_frame = frames.size() - 1;
        }
        if (end_frame == -1 || end_frame > frames.size() - 1) {
            end_frame = frames.size() - 1;
        }
        end_at_frame = p_end_at == -1 ? end_frame : CLAMP(p_end_at, 0, frames.size() - 1);
    }
    remaining_times = MAX(0, p_times);
    status = 0;
    set_frame(start_frame);
}

Ref<Texture2D> GMovieClip::get_current_texture() const {
    return current_texture;
}

void GMovieClip::update_process_state() {
    set_process(playing && !frames.is_empty() && status != 3);
}

void GMovieClip::draw_current_frame() {
    if (frames.is_empty() || package_name.is_empty()) {
        current_texture = Ref<Texture2D>();
        current_rect = Rect2();
        queue_redraw();
        return;
    }

    current_frame = CLAMP(current_frame, 0, frames.size() - 1);
    const fgui::MovieClipFrame &frame = frames[current_frame];
    current_texture = UIPackage::get_movie_clip_frame_texture(package_name, item_name, current_frame);
    current_rect = frame.rect;
    queue_redraw();
}

void GMovieClip::step_frame() {
    if (frames.is_empty()) {
        return;
    }
    if (swing) {
        if (reversed) {
            current_frame--;
            if (current_frame <= 0) {
                current_frame = 0;
                repeated_count++;
                reversed = !reversed;
            }
        } else {
            current_frame++;
            if (current_frame > frames.size() - 1) {
                current_frame = MAX(0, frames.size() - 2);
                repeated_count++;
                reversed = !reversed;
            }
        }
    } else {
        current_frame++;
        if (current_frame > frames.size() - 1) {
            current_frame = 0;
            repeated_count++;
        }
    }
}
