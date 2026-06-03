#include "g_list.h"

#include "g_scroll_bar.h"
#include "ui_package.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

void GList::_gui_input(const Ref<InputEvent> &p_event) {
    Ref<InputEventMouseButton> mouse_button = p_event;
    if (mouse_button.is_valid()) {
        if (mouse_button->is_pressed()) {
            if (handle_mouse_wheel(static_cast<int32_t>(mouse_button->get_button_index()))) {
                accept_event();
                return;
            }
            if (scroll_touch_effect && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
                scroll_dragging = true;
                scroll_drag_last_pos = mouse_button->get_global_position();
                scroll_velocity = Vector2();
                set_process(true);
            }
        } else {
            if (scroll_dragging && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
                scroll_dragging = false;
            }
        }
        return;
    }

    Ref<InputEventMouseMotion> mouse_motion = p_event;
    if (mouse_motion.is_valid() && scroll_dragging) {
        Vector2 current_pos = mouse_motion->get_global_position();
        Vector2 delta = scroll_drag_last_pos - current_pos;
        scroll_velocity = delta;
        scroll_drag_last_pos = current_pos;

        Vector2 new_pos = scroll_position + delta;
        new_pos = clamp_scroll_position(new_pos);
        if (scroll_position != new_pos) {
            scroll_position = new_pos;
            if (virtual_list) {
                relayout_virtual_items();
            } else {
                relayout_items();
            }
            accept_event();
        }
        return;
    }

    GComponent::_gui_input(p_event);
}

void GList::_process(double p_delta) {
    process_scroll_physics(p_delta);
}

bool GList::handle_mouse_wheel(int32_t p_button_index) {
    if (!mouse_wheel_enabled) {
        return false;
    }

    const MouseButton button = static_cast<MouseButton>(p_button_index);
    const bool prefer_horizontal = scroll_type == fgui::ScrollType::Horizontal || (scroll_type == fgui::ScrollType::Both && content_size.x > get_size().x && content_size.y <= get_size().y);
    if (button == MouseButton::MOUSE_BUTTON_WHEEL_UP) {
        set_scroll_position(scroll_position - (prefer_horizontal ? Vector2(scroll_step, 0) : Vector2(0, scroll_step)));
        return true;
    }
    if (button == MouseButton::MOUSE_BUTTON_WHEEL_DOWN) {
        set_scroll_position(scroll_position + (prefer_horizontal ? Vector2(scroll_step, 0) : Vector2(0, scroll_step)));
        return true;
    }
    if (button == MouseButton::MOUSE_BUTTON_WHEEL_LEFT) {
        set_scroll_position(scroll_position - Vector2(scroll_step, 0));
        return true;
    }
    if (button == MouseButton::MOUSE_BUTTON_WHEEL_RIGHT) {
        set_scroll_position(scroll_position + Vector2(scroll_step, 0));
        return true;
    }
    return false;
}

void GList::setup_scroll(fgui::ByteBuffer &p_buffer) {
    scroll_type = static_cast<fgui::ScrollType>(p_buffer.read_byte());
    scroll_bar_display = static_cast<fgui::ScrollBarDisplayType>(p_buffer.read_byte());
    const int32_t flags = p_buffer.read_int();

    if (p_buffer.read_bool()) {
        p_buffer.skip(16);
    }

    vt_scroll_bar_res = p_buffer.read_s();
    hz_scroll_bar_res = p_buffer.read_s();
    p_buffer.read_s();
    p_buffer.read_s();

    snap_to_item = (flags & 2) != 0;
    page_mode = (flags & 8) != 0;
    scroll_touch_effect = !((flags & 32) != 0);
    if ((flags & 16) != 0) scroll_touch_effect = true;
    scroll_bounce_effect = !((flags & 128) != 0);
    if ((flags & 64) != 0) scroll_bounce_effect = true;
    scroll_inertia_disabled = (flags & 256) != 0;
    mouse_wheel_enabled = scroll_bar_display != fgui::ScrollBarDisplayType::Hidden;
    set_clip_contents(true);

    if (!vt_scroll_bar_res.is_empty()) {
        GObject *obj = UIPackage::create_object_from_url(vt_scroll_bar_res);
        vt_scroll_bar = Object::cast_to<GScrollBar>(obj);
        if (vt_scroll_bar != nullptr) {
            add_child(vt_scroll_bar);
        }
    }
    if (!hz_scroll_bar_res.is_empty()) {
        GObject *obj = UIPackage::create_object_from_url(hz_scroll_bar_res);
        hz_scroll_bar = Object::cast_to<GScrollBar>(obj);
        if (hz_scroll_bar != nullptr) {
            add_child(hz_scroll_bar);
        }
    }
}

void GList::process_scroll_physics(double p_delta) {
    if (scroll_dragging) return;

    bool has_motion = false;
    Vector2 new_pos = scroll_position;

    if (!scroll_inertia_disabled && (Math::abs(scroll_velocity.x) > 0.1 || Math::abs(scroll_velocity.y) > 0.1)) {
        has_motion = true;
        real_t decay = Math::pow(static_cast<real_t>(scroll_deceleration_rate), static_cast<real_t>(p_delta * 60.0));
        scroll_velocity = scroll_velocity * decay;
        new_pos.x += scroll_velocity.x * p_delta;
        new_pos.y += scroll_velocity.y * p_delta;
    }

    new_pos = clamp_scroll_position(new_pos);
    if (scroll_bounce_effect && !scroll_dragging) {
        if (scroll_velocity.length() < 0.1 && new_pos != scroll_position) {
            scroll_velocity = Vector2();
        }
    }

    if (new_pos != scroll_position || has_motion) {
        scroll_position = new_pos;
        sync_scroll_bars();
        if (virtual_list) {
            relayout_virtual_items();
        } else {
            relayout_items();
        }
    } else {
        set_process(false);
    }
}

Vector2 GList::clamp_scroll_position(const Vector2 &p_position) const {
    const Vector2 viewport = get_size();
    const real_t max_x = MAX(static_cast<real_t>(0.0), content_size.x - viewport.x);
    const real_t max_y = MAX(static_cast<real_t>(0.0), content_size.y - viewport.y);
    return Vector2(CLAMP(p_position.x, static_cast<real_t>(0.0), max_x), CLAMP(p_position.y, static_cast<real_t>(0.0), max_y));
}

void GList::sync_scroll_bars() {
    const Vector2 viewport = get_size();
    const real_t content_w = content_size.x;
    const real_t content_h = content_size.y;

    if (vt_scroll_bar != nullptr && content_h > 0.01f) {
        vt_scroll_bar->set_scroll_perc(CLAMP(scroll_position.y / MAX(0.01f, content_h - viewport.y), 0.0f, 1.0f));
        vt_scroll_bar->set_display_perc(CLAMP(viewport.y / MAX(0.01f, content_h), 0.0f, 1.0f));
    }
    if (hz_scroll_bar != nullptr && content_w > 0.01f) {
        hz_scroll_bar->set_scroll_perc(CLAMP(scroll_position.x / MAX(0.01f, content_w - viewport.x), 0.0f, 1.0f));
        hz_scroll_bar->set_display_perc(CLAMP(viewport.x / MAX(0.01f, content_w), 0.0f, 1.0f));
    }
}
