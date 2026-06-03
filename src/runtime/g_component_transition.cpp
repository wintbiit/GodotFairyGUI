#include "g_component.h"

#include "g_button.h"
#include "g_combo_box.h"
#include "g_label.h"
#include "g_loader.h"
#include "g_movie_clip.h"
#include "g_text_field.h"
#include "g_tween.h"
#include "ui_package.h"

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/object.hpp>

using namespace godot;

bool GComponent::play_transition(const String &p_name) {
    return play_transition_at(find_transition_index(p_name));
}

bool GComponent::play_transition_at(int32_t p_index) {
    if (p_index < 0 || p_index >= transitions.size()) {
        return false;
    }

    bool scheduled = false;
    GTween *tween = get_transition_tween();
    TransitionData &transition = transitions.write[p_index];
    for (int32_t i = 0; i < transition.items.size(); i++) {
        TransitionItem &item = transition.items.write[i];
        Object *target = resolve_transition_target(item);
        if (target == nullptr) {
            continue;
        }

        int32_t tween_id = 0;
        switch (item.type) {
            case TRANSITION_XY: {
                Control *control = Object::cast_to<Control>(target);
                if (control == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1 && item.start_value.b2) {
                    control->set_position(Vector2(item.start_value.f1, item.start_value.f2));
                }
                tween_id = tween->to_position(control, Vector2(item.end_value.f1, item.end_value.f2), item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_SIZE: {
                Control *control = Object::cast_to<Control>(target);
                if (control == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1 && item.start_value.b2) {
                    control->set_size(Vector2(item.start_value.f1, item.start_value.f2));
                }
                tween_id = tween->to_property(control, StringName("size"), Vector2(item.end_value.f1, item.end_value.f2), item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_SCALE: {
                Control *control = Object::cast_to<Control>(target);
                if (control == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1) {
                    control->set_scale(Vector2(item.start_value.f1, item.start_value.f2));
                }
                tween_id = tween->to_property(control, StringName("scale"), Vector2(item.end_value.f1, item.end_value.f2), item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_PIVOT: {
                Control *control = Object::cast_to<Control>(target);
                if (control == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1) {
                    control->set_pivot_offset(Vector2(item.start_value.f1, item.start_value.f2));
                }
                tween_id = tween->to_pivot(control, Vector2(item.end_value.f1, item.end_value.f2), item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_ALPHA: {
                CanvasItem *canvas_item = Object::cast_to<CanvasItem>(target);
                if (canvas_item == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1) {
                    Color modulate = canvas_item->get_modulate();
                    modulate.a = CLAMP(static_cast<double>(item.start_value.f1), 0.0, 1.0);
                    canvas_item->set_modulate(modulate);
                }
                tween_id = tween->to_alpha(canvas_item, item.end_value.f1, item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_ROTATION: {
                Control *control = Object::cast_to<Control>(target);
                if (control == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1) {
                    control->set_rotation(Math::deg_to_rad(static_cast<double>(item.start_value.f1)));
                }
                tween_id = tween->to_property(control, StringName("rotation"), Math::deg_to_rad(static_cast<double>(item.end_value.f1)), item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_COLOR: {
                CanvasItem *canvas_item = Object::cast_to<CanvasItem>(target);
                if (canvas_item == nullptr) continue;
                if (item.has_tween && item.time <= 0.0f && item.start_value.b1) {
                    canvas_item->set_modulate(item.start_value.color);
                }
                tween_id = tween->to_property(canvas_item, StringName("modulate"), item.end_value.color, item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_COLOR_FILTER: {
                CanvasItem *canvas_item = Object::cast_to<CanvasItem>(target);
                if (canvas_item == nullptr) continue;
                Color end_color(item.end_value.f1, item.end_value.f2, item.end_value.f3, item.end_value.f4);
                tween_id = tween->to_property(canvas_item, StringName("modulate"), end_color, item.has_tween ? item.duration : 0.0);
                break;
            }
            case TRANSITION_VISIBLE:
                tween_id = tween->to_property(target, StringName("visible"), item.end_value.visible, 0.0);
                break;
            case TRANSITION_TEXT: {
                GTextField *text_field = Object::cast_to<GTextField>(target);
                if (text_field != nullptr) { tween_id = tween->to_property(text_field, StringName("text"), item.end_value.text, 0.0); break; }
                GLabel *label = Object::cast_to<GLabel>(target);
                if (label != nullptr) { tween_id = tween->to_property(label, StringName("text"), item.end_value.text, 0.0); break; }
                GComboBox *combo_box = Object::cast_to<GComboBox>(target);
                if (combo_box != nullptr) { tween_id = tween->to_property(combo_box, StringName("text"), item.end_value.text, 0.0); break; }
                GButton *button = Object::cast_to<GButton>(target);
                if (button != nullptr) { tween_id = tween->to_property(button, StringName("title"), item.end_value.text, 0.0); }
                break;
            }
            case TRANSITION_ICON: {
                GLoader *loader = Object::cast_to<GLoader>(target);
                if (loader != nullptr) { tween_id = tween->to_property(loader, StringName("url"), item.end_value.text, 0.0); break; }
                GLabel *label = Object::cast_to<GLabel>(target);
                if (label != nullptr) { tween_id = tween->to_property(label, StringName("icon"), item.end_value.text, 0.0); break; }
                GComboBox *combo_box = Object::cast_to<GComboBox>(target);
                if (combo_box != nullptr) { tween_id = tween->to_property(combo_box, StringName("icon"), item.end_value.text, 0.0); }
                break;
            }
            case TRANSITION_ANIMATION: {
                GMovieClip *movie_clip = Object::cast_to<GMovieClip>(target);
                if (movie_clip != nullptr) {
                    if (item.anim_frame >= 0) movie_clip->set_frame(item.anim_frame);
                    movie_clip->set_playing(item.anim_playing);
                }
                tween_id = tween->delayed_call(0, Callable());
                break;
            }
            case TRANSITION_SOUND: {
                if (!item.sound_url.is_empty()) {
                    Ref<AudioStream> stream = UIPackage::get_sound_audio_stream_by_url(item.sound_url);
                    if (stream.is_valid()) {
                        if (transition_sound_player == nullptr) {
                            transition_sound_player = memnew(AudioStreamPlayer);
                            transition_sound_player->set_max_polyphony(4);
                            add_child(transition_sound_player);
                        }
                        transition_sound_player->set_stream(stream);
                        if (item.sound_volume > 0.0f && item.sound_volume != 1.0f) {
                            double db = 20.0 * Math::log(static_cast<double>(MAX(0.0f, item.sound_volume))) / Math::log(10.0);
                            transition_sound_player->set_volume_db(static_cast<float>(db));
                        }
                        transition_sound_player->play();
                    }
                }
                tween_id = tween->delayed_call(0, Callable());
                break;
            }
            case TRANSITION_TRANSITION: {
                GComponent *comp = Object::cast_to<GComponent>(target);
                if (comp != nullptr && !item.trans_target_name.is_empty()) {
                    comp->play_transition(item.trans_target_name);
                }
                tween_id = tween->delayed_call(0, Callable());
                break;
            }
            case TRANSITION_SHAKE: {
                Control *control = Object::cast_to<Control>(target);
                if (control != nullptr) {
                    tween_id = tween->to_shake(control, item.shake_amplitude, MAX(0.01, item.shake_duration));
                }
                break;
            }
            case TRANSITION_SKEW:
                tween_id = tween->delayed_call(0, Callable());
                break;
            default:
                break;
        }

        if (tween_id > 0) {
            item.active_tween_id = tween_id;
            tween->set_tween_delay(tween_id, item.time);
            tween->set_tween_ease(tween_id, item.ease_type);
            if (item.repeat != 0) {
                tween->set_tween_repeat(tween_id, item.repeat, item.yoyo);
            }
            scheduled = true;
        }
    }
    return scheduled;
}

void GComponent::advance_transitions(double p_delta) {
    if (transition_tween != nullptr) {
        transition_tween->advance(p_delta);
    }
}

void GComponent::add_transition_xy(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_XY;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.b2 = true;
    item.end_value.f1 = p_end.x;
    item.end_value.f2 = p_end.y;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_size(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_SIZE;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.b2 = true;
    item.end_value.f1 = p_end.x;
    item.end_value.f2 = p_end.y;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_scale(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_SCALE;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.b2 = true;
    item.end_value.f1 = p_end.x;
    item.end_value.f2 = p_end.y;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_alpha(const String &p_name, Object *p_target, double p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_ALPHA;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.f1 = CLAMP(p_end, 0.0, 1.0);
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_rotation(const String &p_name, Object *p_target, double p_end_degrees, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_ROTATION;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.f1 = p_end_degrees;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_color(const String &p_name, Object *p_target, const Color &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_COLOR;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.ease_type = 0;
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.color = p_end;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_visible(const String &p_name, Object *p_target, bool p_visible, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_VISIBLE;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.end_value.visible = p_visible;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_text(const String &p_name, Object *p_target, const String &p_text, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_TEXT;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.end_value.text = p_text;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_icon(const String &p_name, Object *p_target, const String &p_icon, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_ICON;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.end_value.text = p_icon;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_pivot(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_PIVOT;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.b2 = true;
    item.end_value.f1 = p_end.x;
    item.end_value.f2 = p_end.y;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_shake(const String &p_name, Object *p_target, float p_amplitude, float p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_SHAKE;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.shake_amplitude = p_amplitude;
    item.shake_duration = MAX(0.01f, p_duration);
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + p_duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_color_filter(const String &p_name, Object *p_target, const Vector4 &p_end, double p_duration, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_COLOR_FILTER;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.duration = MAX(0.0, p_duration);
    item.has_tween = true;
    item.end_value.b1 = true;
    item.end_value.f1 = p_end.x;
    item.end_value.f2 = p_end.y;
    item.end_value.f3 = p_end.z;
    item.end_value.f4 = p_end.w;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time + item.duration);
    transitions.write[index].items.push_back(item);
}

void GComponent::add_transition_sound(const String &p_name, Object *p_target, const String &p_sound_url, float p_volume, double p_delay) {
    const int32_t index = get_or_create_transition_index(p_name);

    TransitionItem item;
    item.type = TRANSITION_SOUND;
    item.target_instance_id = p_target != nullptr ? p_target->get_instance_id() : get_instance_id();
    item.time = MAX(0.0, p_delay);
    item.sound_url = p_sound_url;
    item.sound_volume = p_volume;
    transitions.write[index].total_duration = MAX(transitions[index].total_duration, item.time);
    transitions.write[index].items.push_back(item);
}

void GComponent::apply_transition_value(Object *p_target, const TransitionItem &p_item) {
    if (p_target == nullptr) return;

    switch (p_item.type) {
        case TRANSITION_XY: {
            Control *control = Object::cast_to<Control>(p_target);
            if (control == nullptr) break;
            if (p_item.end_value.b1 && p_item.end_value.b2) {
                control->set_position(Vector2(p_item.end_value.f1, p_item.end_value.f2));
            }
            break;
        }
        case TRANSITION_SIZE: {
            Control *control = Object::cast_to<Control>(p_target);
            if (control == nullptr) break;
            if (p_item.end_value.b1 && p_item.end_value.b2) {
                control->set_size(Vector2(p_item.end_value.f1, p_item.end_value.f2));
            }
            break;
        }
        case TRANSITION_VISIBLE: {
            CanvasItem *canvas_item = Object::cast_to<CanvasItem>(p_target);
            if (canvas_item != nullptr) {
                canvas_item->set_visible(p_item.end_value.visible);
            }
            break;
        }
        case TRANSITION_ANIMATION: {
            GMovieClip *movie_clip = Object::cast_to<GMovieClip>(p_target);
            if (movie_clip != nullptr) {
                if (p_item.anim_frame >= 0) movie_clip->set_frame(p_item.anim_frame);
                movie_clip->set_playing(p_item.anim_playing);
            }
            break;
        }
        default:
            break;
    }
}

void GComponent::stop_transition_item(TransitionItem &p_item, bool p_set_to_complete) {
    if (p_item.active_tween_id > 0 && transition_tween != nullptr) {
        transition_tween->kill(p_item.active_tween_id, p_set_to_complete);
        p_item.active_tween_id = 0;
    }
}

void GComponent::check_auto_play_transitions() {
    if (transitions.is_empty()) return;

    for (int32_t i = 0; i < transitions.size(); i++) {
        const TransitionData &transition = transitions[i];
        if (transition.auto_play) {
            play_transition_at(i);
        }
    }
}

void GComponent::setup_transitions(fgui::ByteBuffer &p_buffer, const Vector<GObject *> &p_children) {
    transitions.clear();
    if (!p_buffer.seek(0, 5)) {
        return;
    }

    const int32_t transition_count = p_buffer.read_short();
    for (int32_t i = 0; i < transition_count; i++) {
        const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();
        TransitionData transition;
        transition.name = p_buffer.read_s();
        p_buffer.read_int();
        transition.auto_play = p_buffer.read_bool();
        transition.auto_play_times = p_buffer.read_int();
        transition.auto_play_delay = p_buffer.read_float();

        const int32_t item_count = p_buffer.read_short();
        for (int32_t item_index = 0; item_index < item_count; item_index++) {
            const int32_t data_len = p_buffer.read_short();
            const int64_t item_begin_pos = p_buffer.get_position();
            if (!p_buffer.seek(item_begin_pos, 0)) {
                p_buffer.set_position(item_begin_pos + data_len);
                continue;
            }

            const uint8_t type_byte = p_buffer.read_byte();
            TransitionItem item;
            item.type = static_cast<TransitionActionType>(type_byte);
            item.time = p_buffer.read_float();
            const int32_t target_index = p_buffer.read_short();
            if (target_index >= 0 && target_index < p_children.size() && p_children[target_index] != nullptr) {
                item.target_item_id = p_children[target_index]->get_package_item_id();
                item.target_instance_id = p_children[target_index]->get_instance_id();
            } else {
                item.target_instance_id = get_instance_id();
            }
            item.label = p_buffer.read_s();
            item.has_tween = p_buffer.read_bool();

            if (item.has_tween) {
                if (p_buffer.seek(item_begin_pos, 1)) {
                    item.duration = p_buffer.read_float();
                    item.ease_type = p_buffer.read_byte();
                    item.repeat = p_buffer.read_int();
                    item.yoyo = p_buffer.read_bool();
                    item.end_label = p_buffer.read_s();
                }
                if (p_buffer.seek(item_begin_pos, 2)) {
                    decode_transition_value(p_buffer, item.type, item.start_value);
                }
                if (p_buffer.seek(item_begin_pos, 3)) {
                    decode_transition_value(p_buffer, item.type, item.end_value);
                }
                transition.total_duration = MAX(transition.total_duration, item.time + item.duration);
            } else {
                if (p_buffer.seek(item_begin_pos, 2)) {
                    decode_transition_value(p_buffer, item.type, item.end_value);
                }
                switch (item.type) {
                    case TRANSITION_ANIMATION:
                        item.anim_frame = p_buffer.read_int();
                        item.anim_playing = p_buffer.read_bool();
                        if (p_buffer.version >= 3) {
                            item.anim_animation_name = p_buffer.read_s();
                            item.anim_skin_name = p_buffer.read_s();
                        }
                        break;
                    case TRANSITION_SOUND:
                        item.sound_url = p_buffer.read_s();
                        item.sound_volume = p_buffer.read_float();
                        break;
                    case TRANSITION_TRANSITION:
                        item.trans_target_name = p_buffer.read_s();
                        item.trans_play_times = p_buffer.read_int();
                        break;
                    case TRANSITION_SHAKE:
                        item.shake_amplitude = p_buffer.read_float();
                        item.shake_duration = p_buffer.read_float();
                        break;
                    default:
                        break;
                }
                transition.total_duration = MAX(transition.total_duration, item.time);
            }

            transition.items.push_back(item);
            p_buffer.set_position(item_begin_pos + data_len);
        }

        transitions.push_back(transition);
        p_buffer.set_position(next_pos);
    }
}

void GComponent::decode_transition_value(fgui::ByteBuffer &p_buffer, TransitionActionType p_type, TransitionValue &r_value) {
    switch (p_type) {
        case TRANSITION_XY:
        case TRANSITION_SIZE:
        case TRANSITION_PIVOT:
        case TRANSITION_SKEW:
            r_value.b1 = p_buffer.read_bool();
            r_value.b2 = p_buffer.read_bool();
            r_value.f1 = p_buffer.read_float();
            r_value.f2 = p_buffer.read_float();
            if (p_buffer.version >= 2 && p_type == TRANSITION_XY) {
                r_value.b3 = p_buffer.read_bool();
            }
            break;
        case TRANSITION_SCALE:
            r_value.b1 = true;
            r_value.b2 = true;
            r_value.f1 = p_buffer.read_float();
            r_value.f2 = p_buffer.read_float();
            break;
        case TRANSITION_ALPHA:
        case TRANSITION_ROTATION:
            r_value.b1 = true;
            r_value.f1 = p_buffer.read_float();
            break;
        case TRANSITION_COLOR:
            r_value.b1 = true;
            r_value.color = p_buffer.read_color();
            break;
        case TRANSITION_COLOR_FILTER:
            r_value.b1 = true;
            r_value.f1 = p_buffer.read_float();
            r_value.f2 = p_buffer.read_float();
            r_value.f3 = p_buffer.read_float();
            r_value.f4 = p_buffer.read_float();
            break;
        case TRANSITION_VISIBLE:
            r_value.b1 = true;
            r_value.visible = p_buffer.read_bool();
            break;
        case TRANSITION_TEXT:
        case TRANSITION_ICON:
            r_value.b1 = true;
            r_value.text = p_buffer.read_s();
            break;
        case TRANSITION_ANIMATION:
        case TRANSITION_SOUND:
        case TRANSITION_TRANSITION:
        case TRANSITION_SHAKE:
            break;
        default:
            break;
    }
}

GTween *GComponent::get_transition_tween() {
    if (transition_tween == nullptr) {
        transition_tween = memnew(GTween);
        transition_tween->set_name("_GTween");
        add_child(transition_tween);
    }
    return transition_tween;
}

Object *GComponent::resolve_transition_target(const TransitionItem &p_item) {
    if (p_item.target_instance_id == 0) {
        return this;
    }

    Object *target = ObjectDB::get_instance(p_item.target_instance_id);
    return target != nullptr ? target : this;
}

int32_t GComponent::find_transition_index(const String &p_name) const {
    for (int32_t i = 0; i < transitions.size(); i++) {
        if (transitions[i].name == p_name) {
            return i;
        }
    }
    return -1;
}

int32_t GComponent::get_or_create_transition_index(const String &p_name) {
    int32_t index = find_transition_index(p_name);
    if (index >= 0) {
        return index;
    }

    TransitionData transition;
    transition.name = p_name;
    transitions.push_back(transition);
    return transitions.size() - 1;
}
