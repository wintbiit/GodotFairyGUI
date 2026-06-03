#include "g_tween.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

int32_t GTween::EASE_LINEAR = 0;
int32_t GTween::EASE_SINE_IN = 1;
int32_t GTween::EASE_SINE_OUT = 2;
int32_t GTween::EASE_SINE_IN_OUT = 3;
int32_t GTween::EASE_QUAD_IN = 4;
int32_t GTween::EASE_QUAD_OUT = 5;
int32_t GTween::EASE_QUAD_IN_OUT = 6;
int32_t GTween::EASE_CUBIC_IN = 7;
int32_t GTween::EASE_CUBIC_OUT = 8;
int32_t GTween::EASE_CUBIC_IN_OUT = 9;
int32_t GTween::EASE_QUART_IN = 10;
int32_t GTween::EASE_QUART_OUT = 11;
int32_t GTween::EASE_QUART_IN_OUT = 12;
int32_t GTween::EASE_QUINT_IN = 13;
int32_t GTween::EASE_QUINT_OUT = 14;
int32_t GTween::EASE_QUINT_IN_OUT = 15;
int32_t GTween::EASE_EXPO_IN = 16;
int32_t GTween::EASE_EXPO_OUT = 17;
int32_t GTween::EASE_EXPO_IN_OUT = 18;
int32_t GTween::EASE_CIRC_IN = 19;
int32_t GTween::EASE_CIRC_OUT = 20;
int32_t GTween::EASE_CIRC_IN_OUT = 21;
int32_t GTween::EASE_ELASTIC_IN = 22;
int32_t GTween::EASE_ELASTIC_OUT = 23;
int32_t GTween::EASE_ELASTIC_IN_OUT = 24;
int32_t GTween::EASE_BACK_IN = 25;
int32_t GTween::EASE_BACK_OUT = 26;
int32_t GTween::EASE_BACK_IN_OUT = 27;
int32_t GTween::EASE_BOUNCE_IN = 28;
int32_t GTween::EASE_BOUNCE_OUT = 29;
int32_t GTween::EASE_BOUNCE_IN_OUT = 30;
int32_t GTween::EASE_CUSTOM = 31;

void GTween::_bind_methods() {
    ClassDB::bind_method(D_METHOD("to_position", "target", "end", "duration"), &GTween::to_position);
    ClassDB::bind_method(D_METHOD("to_alpha", "target", "end", "duration"), &GTween::to_alpha);
    ClassDB::bind_method(D_METHOD("to_property", "target", "property", "end", "duration"), &GTween::to_property);
    ClassDB::bind_method(D_METHOD("to_value", "start", "end", "duration", "update"), &GTween::to_value);
    ClassDB::bind_method(D_METHOD("to_shake", "target", "amplitude", "duration"), &GTween::to_shake);
    ClassDB::bind_method(D_METHOD("delayed_call", "delay", "callback"), &GTween::delayed_call);

    ClassDB::bind_method(D_METHOD("set_tween_delay", "id", "delay"), &GTween::set_tween_delay);
    ClassDB::bind_method(D_METHOD("set_tween_ease", "id", "ease_type"), &GTween::set_tween_ease);
    ClassDB::bind_method(D_METHOD("set_tween_ease_params", "id", "amplitude", "period"), &GTween::set_tween_ease_params);
    ClassDB::bind_method(D_METHOD("set_tween_repeat", "id", "repeat", "yoyo"), &GTween::set_tween_repeat, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("set_tween_breakpoint", "id", "breakpoint"), &GTween::set_tween_breakpoint);
    ClassDB::bind_method(D_METHOD("set_tween_time_scale", "id", "time_scale"), &GTween::set_tween_time_scale);
    ClassDB::bind_method(D_METHOD("set_tween_snapping", "id", "snapping"), &GTween::set_tween_snapping);
    ClassDB::bind_method(D_METHOD("set_tween_paused", "id", "paused"), &GTween::set_tween_paused);
    ClassDB::bind_method(D_METHOD("set_tween_target", "id", "target"), &GTween::set_tween_target);
    ClassDB::bind_method(D_METHOD("set_tween_user_data", "id", "data"), &GTween::set_tween_user_data);
    ClassDB::bind_method(D_METHOD("seek", "id", "time"), &GTween::seek);
    ClassDB::bind_method(D_METHOD("set_tween_on_start", "id", "callback"), &GTween::set_tween_on_start);
    ClassDB::bind_method(D_METHOD("set_tween_on_update", "id", "callback"), &GTween::set_tween_on_update);
    ClassDB::bind_method(D_METHOD("set_tween_on_complete", "id", "callback"), &GTween::set_tween_on_complete);

    ClassDB::bind_method(D_METHOD("kill", "id", "complete"), &GTween::kill, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("kill_target", "target", "complete"), &GTween::kill_target, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("is_tweening", "target"), &GTween::is_tweening);
    ClassDB::bind_method(D_METHOD("get_active_tween_count"), &GTween::get_active_tween_count);
    ClassDB::bind_method(D_METHOD("clear"), &GTween::clear);
    ClassDB::bind_method(D_METHOD("advance", "delta"), &GTween::advance);

    BIND_CONSTANT(EASE_LINEAR);
    BIND_CONSTANT(EASE_SINE_IN);
    BIND_CONSTANT(EASE_SINE_OUT);
    BIND_CONSTANT(EASE_SINE_IN_OUT);
    BIND_CONSTANT(EASE_QUAD_IN);
    BIND_CONSTANT(EASE_QUAD_OUT);
    BIND_CONSTANT(EASE_QUAD_IN_OUT);
    BIND_CONSTANT(EASE_CUBIC_IN);
    BIND_CONSTANT(EASE_CUBIC_OUT);
    BIND_CONSTANT(EASE_CUBIC_IN_OUT);
    BIND_CONSTANT(EASE_QUART_IN);
    BIND_CONSTANT(EASE_QUART_OUT);
    BIND_CONSTANT(EASE_QUART_IN_OUT);
    BIND_CONSTANT(EASE_QUINT_IN);
    BIND_CONSTANT(EASE_QUINT_OUT);
    BIND_CONSTANT(EASE_QUINT_IN_OUT);
    BIND_CONSTANT(EASE_EXPO_IN);
    BIND_CONSTANT(EASE_EXPO_OUT);
    BIND_CONSTANT(EASE_EXPO_IN_OUT);
    BIND_CONSTANT(EASE_CIRC_IN);
    BIND_CONSTANT(EASE_CIRC_OUT);
    BIND_CONSTANT(EASE_CIRC_IN_OUT);
    BIND_CONSTANT(EASE_ELASTIC_IN);
    BIND_CONSTANT(EASE_ELASTIC_OUT);
    BIND_CONSTANT(EASE_ELASTIC_IN_OUT);
    BIND_CONSTANT(EASE_BACK_IN);
    BIND_CONSTANT(EASE_BACK_OUT);
    BIND_CONSTANT(EASE_BACK_IN_OUT);
    BIND_CONSTANT(EASE_BOUNCE_IN);
    BIND_CONSTANT(EASE_BOUNCE_OUT);
    BIND_CONSTANT(EASE_BOUNCE_IN_OUT);
    BIND_CONSTANT(EASE_CUSTOM);

    ADD_SIGNAL(MethodInfo("fgui_tween_complete", PropertyInfo(Variant::INT, "id")));
}

GTween::GTween() {
    set_process(true);
}

void GTween::_process(double p_delta) {
    advance(p_delta);
}

int32_t GTween::to_position(Object *p_target, const Vector2 &p_end, double p_duration) {
    Control *control = Object::cast_to<Control>(p_target);
    if (control == nullptr) {
        return 0;
    }

    Tween tween;
    tween.kind = TWEEN_POSITION;
    tween.target_id = control->get_instance_id();
    tween.start_position = control->get_position();
    tween.end_position = p_end;
    tween.duration = MAX(0.0, p_duration);
    return add_tween(tween);
}

int32_t GTween::to_alpha(Object *p_target, double p_end, double p_duration) {
    CanvasItem *canvas_item = Object::cast_to<CanvasItem>(p_target);
    if (canvas_item == nullptr) {
        return 0;
    }

    Tween tween;
    tween.kind = TWEEN_ALPHA;
    tween.target_id = canvas_item->get_instance_id();
    tween.start_value = canvas_item->get_modulate().a;
    tween.end_value = CLAMP(p_end, 0.0, 1.0);
    tween.duration = MAX(0.0, p_duration);
    return add_tween(tween);
}

int32_t GTween::to_property(Object *p_target, const StringName &p_property, const Variant &p_end, double p_duration) {
    if (p_target == nullptr || p_property == StringName()) {
        return 0;
    }

    Tween tween;
    tween.kind = TWEEN_PROPERTY;
    tween.target_id = p_target->get_instance_id();
    tween.property = p_property;
    tween.start_variant = p_target->get(p_property);
    tween.end_variant = p_end;
    tween.duration = MAX(0.0, p_duration);
    return add_tween(tween);
}

int32_t GTween::to_value(double p_start, double p_end, double p_duration, const Callable &p_update) {
    Tween tween;
    tween.kind = TWEEN_VALUE;
    tween.start_value = p_start;
    tween.end_value = p_end;
    tween.duration = MAX(0.0, p_duration);
    tween.legacy_update_callback = p_update;
    return add_tween(tween);
}

int32_t GTween::to_shake(Object *p_target, double p_amplitude, double p_duration) {
    Control *control = Object::cast_to<Control>(p_target);
    if (control == nullptr) {
        return 0;
    }

    Tween tween;
    tween.kind = TWEEN_SHAKE;
    tween.target_id = control->get_instance_id();
    tween.shake_start_original = control->get_position();
    tween.shake_amplitude = p_amplitude;
    tween.duration = MAX(0.0, p_duration);
    tween.ease = EASE_LINEAR_ENUM;
    return add_tween(tween);
}

int32_t GTween::to_pivot(Object *p_target, const Vector2 &p_end, double p_duration) {
    Control *control = Object::cast_to<Control>(p_target);
    if (control == nullptr) {
        return 0;
    }

    Tween tween;
    tween.kind = TWEEN_PIVOT;
    tween.target_id = control->get_instance_id();
    tween.start_variant = control->get_pivot_offset();
    tween.end_variant = p_end;
    tween.duration = MAX(0.0, p_duration);
    return add_tween(tween);
}

int32_t GTween::delayed_call(double p_delay, const Callable &p_callback) {
    Tween tween;
    tween.kind = TWEEN_DELAY;
    tween.duration = 0.0;
    tween.delay = MAX(0.0, p_delay);
    tween.legacy_complete_callback = p_callback;
    return add_tween(tween);
}

void GTween::set_tween_delay(int32_t p_id, double p_delay) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->delay = MAX(0.0, p_delay);
    }
}

void GTween::set_tween_ease(int32_t p_id, int32_t p_ease_type) {
    Tween *tween = find_tween(p_id);
    if (tween == nullptr) {
        return;
    }
    if (p_ease_type >= 0 && p_ease_type <= EASE_CUSTOM_ENUM) {
        tween->ease = static_cast<EaseType>(p_ease_type);
    }
}

void GTween::set_tween_ease_params(int32_t p_id, double p_amplitude, double p_period) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->ease_amplitude = p_amplitude;
        tween->ease_period = p_period;
    }
}

void GTween::set_tween_repeat(int32_t p_id, int32_t p_repeat, bool p_yoyo) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->repeat = p_repeat;
        tween->yoyo = p_yoyo;
    }
}

void GTween::set_tween_breakpoint(int32_t p_id, double p_breakpoint) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->breakpoint = p_breakpoint;
    }
}

void GTween::set_tween_time_scale(int32_t p_id, double p_time_scale) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->time_scale = MAX(0.0, p_time_scale);
    }
}

void GTween::set_tween_paused(int32_t p_id, bool p_paused) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->paused = p_paused;
    }
}

void GTween::set_tween_snapping(int32_t p_id, bool p_snapping) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->snapping = p_snapping;
    }
}

void GTween::set_tween_target(int32_t p_id, Object *p_target) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr && p_target != nullptr) {
        tween->target_id = p_target->get_instance_id();
    }
}

void GTween::set_tween_user_data(int32_t p_id, const Variant &p_data) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->user_data = p_data;
    }
}

void GTween::seek(int32_t p_id, double p_time) {
    Tween *tween = find_tween(p_id);
    if (tween == nullptr || tween->killed) return;
    tween->elapsed = MAX(0.0, p_time);
    if (tween->elapsed < tween->delay) return;
    if (!tween->started) tween->started = true;
    const double active_time = tween->elapsed - tween->delay;
    double ratio = tween->duration <= 0.0 ? 1.0 : CLAMP(active_time / tween->duration, 0.0, 1.0);
    if (tween->kind != TWEEN_SHAKE) {
        ratio = ease_ratio(tween->ease, ratio, tween->ease_amplitude, tween->ease_period);
    }
    apply_tween(*tween, ratio);
}

void GTween::set_tween_on_start(int32_t p_id, const Callable &p_callback) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->on_start = p_callback;
    }
}

void GTween::set_tween_on_update(int32_t p_id, const Callable &p_callback) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->on_update = p_callback;
    }
}

void GTween::set_tween_on_complete(int32_t p_id, const Callable &p_callback) {
    Tween *tween = find_tween(p_id);
    if (tween != nullptr) {
        tween->on_complete = p_callback;
    }
}

void GTween::kill(int32_t p_id, bool p_complete) {
    Tween *tween = find_tween(p_id);
    if (tween == nullptr) {
        return;
    }
    if (p_complete) {
        if (tween->_ended == 0) {
            if (tween->breakpoint >= 0) {
                tween->elapsed = tween->delay + tween->breakpoint;
            } else if (tween->repeat >= 0) {
                tween->elapsed = tween->delay + tween->duration * (tween->repeat + 1);
            } else {
                tween->elapsed = tween->delay + tween->duration * 2;
            }
            // Process to completion point
            const double active_time = MAX(0.0, tween->elapsed - tween->delay);
            double ratio = tween->duration <= 0.0 ? 1.0 : CLAMP(active_time / tween->duration, 0.0, 1.0);
            const int32_t total_rounds = tween->repeat >= 0 ? (tween->repeat + 1) : 1;
            int32_t round = static_cast<int32_t>(Math::floor(active_time / MAX(0.001, tween->duration)));
            if (round >= total_rounds) round = total_rounds - 1;
            bool reversed = tween->yoyo && (round % 2 == 1);
            if (reversed) ratio = 1.0 - ratio;
            apply_tween(*tween, ease_ratio(tween->ease, ratio, tween->ease_amplitude, tween->ease_period));
        }
        complete_tween(*tween);
    }
    tween->killed = true;
}

void GTween::kill_target(Object *p_target, bool p_complete) {
    if (p_target == nullptr) {
        return;
    }
    const uint64_t target_id = p_target->get_instance_id();
    for (int32_t i = 0; i < tweens.size(); i++) {
        if (tweens[i].target_id == target_id) {
            if (p_complete) {
                apply_tween(tweens.write[i], 1.0);
                complete_tween(tweens.write[i]);
            }
            tweens.write[i].killed = true;
        }
    }
}

bool GTween::is_tweening(Object *p_target) const {
    if (p_target == nullptr) {
        return false;
    }
    const uint64_t target_id = p_target->get_instance_id();
    for (int32_t i = 0; i < tweens.size(); i++) {
        if (!tweens[i].killed && tweens[i].target_id == target_id) {
            return true;
        }
    }
    return false;
}

int32_t GTween::get_active_tween_count() const {
    int32_t count = 0;
    for (int32_t i = 0; i < tweens.size(); i++) {
        if (!tweens[i].killed) {
            count++;
        }
    }
    return count;
}

void GTween::clear() {
    tweens.clear();
}

void GTween::advance(double p_delta) {
    if (p_delta <= 0.0) {
        return;
    }

    const int32_t initial_count = tweens.size();
    Vector<int32_t> completed_ids;

    for (int32_t i = 0; i < initial_count && i < tweens.size(); i++) {
        Tween &tween = tweens.write[i];
        if (tween.killed || tween._ended != 0) {
            continue;
        }
        if (tween.paused) continue;

        double dt = p_delta * tween.time_scale;
        tween.elapsed += dt;

        if (tween.elapsed < tween.delay) {
            continue;
        }

        if (!tween.started) {
            tween.started = true;
            if (tween.killed) continue;
        }

        const double active_time = tween.elapsed - tween.delay;
        double tt = active_time;
        bool reversed = false;

        if (tween.breakpoint >= 0 && tt >= tween.breakpoint) {
            tt = tween.breakpoint;
            tween._ended = 2;
        }

        if (tween.kind != TWEEN_SHAKE && tween.repeat != 0 && tween.duration > 0) {
            int32_t round = static_cast<int32_t>(Math::floor(tt / tween.duration));
            tt -= tween.duration * round;
            if (tween.yoyo) {
                reversed = round % 2 == 1;
            }
            if (tween.repeat > 0 && tween.repeat - round < 0) {
                if (tween.yoyo) {
                    reversed = tween.repeat % 2 == 1;
                }
                tt = tween.duration;
                tween._ended = 1;
            }
        } else if (tt >= tween.duration) {
            tt = tween.duration;
            tween._ended = 1;
        }

        double ratio = tween.duration <= 0.0 ? 1.0 : CLAMP(tt / tween.duration, 0.0, 1.0);
        if (reversed) ratio = 1.0 - ratio;

        if (tween.kind != TWEEN_SHAKE) {
            ratio = ease_ratio(tween.ease, ratio, tween.ease_amplitude, tween.ease_period);
        }

        apply_tween(tween, ratio);

        if (tween._ended != 0 && !tween.killed) {
            completed_ids.push_back(tween.id);
        }
    }

    // Complete tweens after iteration — safe from re-entrancy
    for (int32_t id : completed_ids) {
        Tween *t = find_tween(id);
        if (t != nullptr) {
            complete_tween(*t);
            t->killed = true;
        }
    }

    // Fire deferred callbacks
    for (int32_t i = 0; i < MIN(initial_count, tweens.size()); i++) {
        Tween &tween = tweens.write[i];
        if (tween.killed || tween._ended == 0) continue;
        if (tween.on_start.is_valid() && !tween.started) {
            tween.started = true;
            tween.on_start.call();
        }
        if (tween.on_update.is_valid()) {
            tween.on_update.call();
        }
    }

    for (int32_t i = tweens.size() - 1; i >= 0; i--) {
        if (tweens[i].killed) {
            tweens.remove_at(i);
        }
    }
}

int32_t GTween::add_tween(const Tween &p_tween) {
    Tween tween = p_tween;
    tween.id = next_id++;
    tweens.push_back(tween);
    return tween.id;
}

GTween::Tween *GTween::find_tween(int32_t p_id) {
    for (int32_t i = 0; i < tweens.size(); i++) {
        if (tweens[i].id == p_id && !tweens[i].killed) {
            return &tweens.write[i];
        }
    }
    return nullptr;
}

const GTween::Tween *GTween::find_tween(int32_t p_id) const {
    for (int32_t i = 0; i < tweens.size(); i++) {
        if (tweens[i].id == p_id && !tweens[i].killed) {
            return &tweens[i];
        }
    }
    return nullptr;
}

void GTween::complete_tween(Tween &p_tween) {
    if (p_tween._ended == 0) {
        p_tween._ended = 1;
    }

    if (p_tween.kind == TWEEN_SHAKE) {
        Object *target = ObjectDB::get_instance(p_tween.target_id);
        Control *control = Object::cast_to<Control>(target);
        if (control != nullptr) {
            control->set_position(p_tween.shake_start_original);
        }
    }

    if (p_tween.legacy_complete_callback.is_valid()) {
        p_tween.legacy_complete_callback.call();
    }
    if (p_tween.on_complete.is_valid()) {
        p_tween.on_complete.call();
    }
    emit_signal("fgui_tween_complete", p_tween.id);
}

void GTween::apply_tween(Tween &p_tween, double p_ratio) {
    if (p_tween.kind == TWEEN_DELAY) {
        return;
    }
    if (p_tween.kind == TWEEN_VALUE) {
        double val = p_tween.start_value + (p_tween.end_value - p_tween.start_value) * p_ratio;
        if (p_tween.snapping) val = Math::round(val);
        if (p_tween.legacy_update_callback.is_valid()) {
            p_tween.legacy_update_callback.call(val);
        }
        return;
    }

    if (p_tween.kind == TWEEN_SHAKE) {
        Object *instance = ObjectDB::get_instance(p_tween.target_id);
        Control *control = Object::cast_to<Control>(instance);
        if (control == nullptr) {
            p_tween.killed = true;
            return;
        }
        if (p_tween._ended == 0) {
            double decay = 1.0 - p_ratio;
            double r_x = (UtilityFunctions::randf() * 2.0 - 1.0) * p_tween.shake_amplitude * decay;
            double r_y = (UtilityFunctions::randf() * 2.0 - 1.0) * p_tween.shake_amplitude * decay;
            control->set_position(Vector2(p_tween.shake_start_original.x + r_x, p_tween.shake_start_original.y + r_y));
        } else {
            control->set_position(p_tween.shake_start_original);
        }
        return;
    }

    // Cache ObjectDB::get_instance — called once per apply_tween
    Object *instance = ObjectDB::get_instance(p_tween.target_id);
    if (instance == nullptr) {
        p_tween.killed = true;
        return;
    }

    switch (p_tween.kind) {
        case TWEEN_POSITION: {
            Control *control = Object::cast_to<Control>(instance);
            if (control == nullptr) {
                p_tween.killed = true;
                return;
            }
            Vector2 pos = p_tween.start_position.lerp(p_tween.end_position, p_ratio);
            if (p_tween.snapping) {
                pos.x = Math::round(pos.x);
                pos.y = Math::round(pos.y);
            }
            control->set_position(pos);
            break;
        }
        case TWEEN_ALPHA: {
            CanvasItem *canvas_item = Object::cast_to<CanvasItem>(instance);
            if (canvas_item == nullptr) {
                p_tween.killed = true;
                return;
            }
            Color modulate = canvas_item->get_modulate();
            double alpha = p_tween.start_value + (p_tween.end_value - p_tween.start_value) * p_ratio;
            if (p_tween.snapping) alpha = Math::round(alpha);
            modulate.a = alpha;
            canvas_item->set_modulate(modulate);
            break;
        }
        case TWEEN_PIVOT: {
            Control *ctrl = Object::cast_to<Control>(instance);
            if (ctrl == nullptr) {
                p_tween.killed = true;
                return;
            }
            Vector2 val = p_tween.start_variant.operator Vector2().lerp(p_tween.end_variant.operator Vector2(), p_ratio);
            if (p_tween.snapping) {
                val.x = Math::round(val.x);
                val.y = Math::round(val.y);
            }
            ctrl->set_pivot_offset(val);
            break;
        }
        case TWEEN_PROPERTY: {
            switch (p_tween.start_variant.get_type()) {
                case Variant::FLOAT:
                case Variant::INT: {
                    const double start = static_cast<double>(p_tween.start_variant);
                    const double end = static_cast<double>(p_tween.end_variant);
                    double val = start + (end - start) * p_ratio;
                    if (p_tween.snapping) val = Math::round(val);
                    instance->set(p_tween.property, val);
                    break;
                }
                case Variant::VECTOR2: {
                    const Vector2 start = p_tween.start_variant;
                    const Vector2 end = p_tween.end_variant;
                    Vector2 val = start.lerp(end, p_ratio);
                    if (p_tween.snapping) {
                        val.x = Math::round(val.x);
                        val.y = Math::round(val.y);
                    }
                    instance->set(p_tween.property, val);
                    break;
                }
                case Variant::COLOR: {
                    const Color start = p_tween.start_variant;
                    const Color end = p_tween.end_variant;
                    instance->set(p_tween.property, start.lerp(end, p_ratio));
                    break;
                }
                default:
                    instance->set(p_tween.property, p_tween.end_variant);
                    break;
            }
            break;
        }
        default:
            break;
    }
}


