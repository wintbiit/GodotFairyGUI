#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {

class GTween : public Node {
    GDCLASS(GTween, Node)

protected:
    static void _bind_methods();

public:
    GTween();

    void _process(double p_delta) override;

    int32_t to_position(Object *p_target, const Vector2 &p_end, double p_duration);
    int32_t to_alpha(Object *p_target, double p_end, double p_duration);
    int32_t to_property(Object *p_target, const StringName &p_property, const Variant &p_end, double p_duration);
    int32_t to_value(double p_start, double p_end, double p_duration, const Callable &p_update);
    int32_t to_shake(Object *p_target, double p_amplitude, double p_duration);
    int32_t delayed_call(double p_delay, const Callable &p_callback);

    void set_tween_delay(int32_t p_id, double p_delay);
    void set_tween_ease(int32_t p_id, int32_t p_ease_type);
    void set_tween_ease_params(int32_t p_id, double p_amplitude, double p_period);
    void set_tween_repeat(int32_t p_id, int32_t p_repeat, bool p_yoyo = false);
    void set_tween_breakpoint(int32_t p_id, double p_breakpoint);
    void set_tween_time_scale(int32_t p_id, double p_time_scale);
    void set_tween_snapping(int32_t p_id, bool p_snapping);
    void set_tween_on_start(int32_t p_id, const Callable &p_callback);
    void set_tween_on_update(int32_t p_id, const Callable &p_callback);
    void set_tween_on_complete(int32_t p_id, const Callable &p_callback);

    void kill(int32_t p_id, bool p_complete = false);
    void kill_target(Object *p_target, bool p_complete = false);
    bool is_tweening(Object *p_target) const;
    int32_t get_active_tween_count() const;
    void clear();
    void advance(double p_delta);

    // EaseType constants exposed to GDScript — must mirror EaseType enum below
    static int32_t EASE_LINEAR;
    static int32_t EASE_SINE_IN;
    static int32_t EASE_SINE_OUT;
    static int32_t EASE_SINE_IN_OUT;
    static int32_t EASE_QUAD_IN;
    static int32_t EASE_QUAD_OUT;
    static int32_t EASE_QUAD_IN_OUT;
    static int32_t EASE_CUBIC_IN;
    static int32_t EASE_CUBIC_OUT;
    static int32_t EASE_CUBIC_IN_OUT;
    static int32_t EASE_QUART_IN;
    static int32_t EASE_QUART_OUT;
    static int32_t EASE_QUART_IN_OUT;
    static int32_t EASE_QUINT_IN;
    static int32_t EASE_QUINT_OUT;
    static int32_t EASE_QUINT_IN_OUT;
    static int32_t EASE_EXPO_IN;
    static int32_t EASE_EXPO_OUT;
    static int32_t EASE_EXPO_IN_OUT;
    static int32_t EASE_CIRC_IN;
    static int32_t EASE_CIRC_OUT;
    static int32_t EASE_CIRC_IN_OUT;
    static int32_t EASE_ELASTIC_IN;
    static int32_t EASE_ELASTIC_OUT;
    static int32_t EASE_ELASTIC_IN_OUT;
    static int32_t EASE_BACK_IN;
    static int32_t EASE_BACK_OUT;
    static int32_t EASE_BACK_IN_OUT;
    static int32_t EASE_BOUNCE_IN;
    static int32_t EASE_BOUNCE_OUT;
    static int32_t EASE_BOUNCE_IN_OUT;
    static int32_t EASE_CUSTOM;

private:
    enum TweenKind {
        TWEEN_POSITION = 0,
        TWEEN_ALPHA = 1,
        TWEEN_VALUE = 2,
        TWEEN_DELAY = 3,
        TWEEN_PROPERTY = 4,
        TWEEN_SHAKE = 5,
    };

    enum EaseType {
        EASE_LINEAR_ENUM = 0,
        EASE_SINE_IN_ENUM,
        EASE_SINE_OUT_ENUM,
        EASE_SINE_IN_OUT_ENUM,
        EASE_QUAD_IN_ENUM,
        EASE_QUAD_OUT_ENUM,
        EASE_QUAD_IN_OUT_ENUM,
        EASE_CUBIC_IN_ENUM,
        EASE_CUBIC_OUT_ENUM,
        EASE_CUBIC_IN_OUT_ENUM,
        EASE_QUART_IN_ENUM,
        EASE_QUART_OUT_ENUM,
        EASE_QUART_IN_OUT_ENUM,
        EASE_QUINT_IN_ENUM,
        EASE_QUINT_OUT_ENUM,
        EASE_QUINT_IN_OUT_ENUM,
        EASE_EXPO_IN_ENUM,
        EASE_EXPO_OUT_ENUM,
        EASE_EXPO_IN_OUT_ENUM,
        EASE_CIRC_IN_ENUM,
        EASE_CIRC_OUT_ENUM,
        EASE_CIRC_IN_OUT_ENUM,
        EASE_ELASTIC_IN_ENUM,
        EASE_ELASTIC_OUT_ENUM,
        EASE_ELASTIC_IN_OUT_ENUM,
        EASE_BACK_IN_ENUM,
        EASE_BACK_OUT_ENUM,
        EASE_BACK_IN_OUT_ENUM,
        EASE_BOUNCE_IN_ENUM,
        EASE_BOUNCE_OUT_ENUM,
        EASE_BOUNCE_IN_OUT_ENUM,
        EASE_CUSTOM_ENUM,
    };

    struct Tween {
        int32_t id = 0;
        TweenKind kind = TWEEN_VALUE;
        uint64_t target_id = 0;
        StringName property;
        Vector2 start_position;
        Vector2 end_position;
        Vector2 shake_start_original;
        double shake_amplitude = 0.0;
        Variant start_variant;
        Variant end_variant;
        double start_value = 0.0;
        double end_value = 0.0;
        double duration = 0.0;
        double delay = 0.0;
        double elapsed = 0.0;
        double time_scale = 1.0;
        bool ignore_engine_time_scale = false;
        EaseType ease = EASE_QUAD_OUT_ENUM;
        double ease_amplitude = 1.70158;
        double ease_period = 0.0;
        int32_t repeat = 0;
        bool yoyo = false;
        bool snapping = false;
        double breakpoint = -1.0;
        Callable on_start;
        Callable on_update;
        Callable on_complete;
        Callable legacy_update_callback;
        Callable legacy_complete_callback;
        bool started = false;
        bool killed = false;
        int32_t _ended = 0;
    };

    Vector<Tween> tweens;
    int32_t next_id = 1;

    int32_t add_tween(const Tween &p_tween);
    Tween *find_tween(int32_t p_id);
    const Tween *find_tween(int32_t p_id) const;
    void complete_tween(Tween &p_tween);
    void apply_tween(Tween &p_tween, double p_ratio);
    double ease_ratio(EaseType p_ease, double p_ratio, double p_amplitude, double p_period) const;
    double bounce_ease_out(double p_ratio) const;
    double bounce_ease_in(double p_ratio) const;
    double bounce_ease_in_out(double p_ratio) const;
};

} // namespace godot
