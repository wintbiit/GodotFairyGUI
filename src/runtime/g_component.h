#pragma once

#include "g_object.h"
#include "package/package_defs.h"

namespace godot {

class GTween;
class AudioStreamPlayer;

class GComponent : public GObject {
    GDCLASS(GComponent, GObject)

protected:
    static void _bind_methods();

public:
    void _notification(int p_what);

    void set_package_name(const String &p_name);
    String get_package_name() const;

    void set_component_name(const String &p_name);
    String get_component_name() const;
    bool construct_from_resource();
    bool construct_from_package(const String &p_package_id, const fgui::PackageItem &p_item);
    int32_t count_descendants_by_class(const String &p_class_name) const;
    int32_t get_controller_count() const;
    int32_t get_controller_page_count(int32_t p_controller_index) const;
    String get_controller_selected_page_id(int32_t p_controller_index) const;
    bool set_controller_selected_index(int32_t p_controller_index, int32_t p_page_index);
    bool set_controller_selected_page_id(int32_t p_controller_index, const String &p_page_id);
    bool go_back_controller(int32_t p_controller_index);
    int32_t get_controller_previous_index(int32_t p_controller_index) const;
    int32_t count_hidden_descendants() const;
    int32_t count_display_gear_descendants() const;
    int32_t count_common_gear_descendants() const;
    int32_t count_relation_descendants() const;
    int32_t get_transition_count() const;
    String get_transition_name(int32_t p_index) const;
    int32_t get_transition_item_count(int32_t p_index) const;
    bool play_transition(const String &p_name);
    bool play_transition_at(int32_t p_index);
    void advance_transitions(double p_delta);
    void add_transition_xy(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_size(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_scale(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_alpha(const String &p_name, Object *p_target, double p_end, double p_duration, double p_delay = 0.0);
    void add_transition_rotation(const String &p_name, Object *p_target, double p_end_degrees, double p_duration, double p_delay = 0.0);
    void add_transition_color(const String &p_name, Object *p_target, const Color &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_visible(const String &p_name, Object *p_target, bool p_visible, double p_delay = 0.0);
    void add_transition_text(const String &p_name, Object *p_target, const String &p_text, double p_delay = 0.0);
    void add_transition_icon(const String &p_name, Object *p_target, const String &p_icon, double p_delay = 0.0);
    void add_transition_pivot(const String &p_name, Object *p_target, const Vector2 &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_shake(const String &p_name, Object *p_target, float p_amplitude, float p_duration, double p_delay = 0.0);
    void add_transition_color_filter(const String &p_name, Object *p_target, const Vector4 &p_end, double p_duration, double p_delay = 0.0);
    void add_transition_sound(const String &p_name, Object *p_target, const String &p_sound_url, float p_volume = 1.0f, double p_delay = 0.0);

private:
    struct ControllerData {
        String name;
        Vector<String> page_ids;
        Vector<String> page_names;
        int32_t selected_index = -1;
        int32_t previous_index = -1;
    };

    enum TransitionActionType {
        TRANSITION_XY = 0,
        TRANSITION_SIZE = 1,
        TRANSITION_SCALE = 2,
        TRANSITION_PIVOT = 3,
        TRANSITION_ALPHA = 4,
        TRANSITION_ROTATION = 5,
        TRANSITION_COLOR = 6,
        TRANSITION_ANIMATION = 7,
        TRANSITION_VISIBLE = 8,
        TRANSITION_SOUND = 9,
        TRANSITION_TRANSITION = 10,
        TRANSITION_SHAKE = 11,
        TRANSITION_COLOR_FILTER = 12,
        TRANSITION_SKEW = 13,
        TRANSITION_TEXT = 14,
        TRANSITION_ICON = 15,
    };

    struct TransitionValue {
        bool b1 = false;
        bool b2 = false;
        bool b3 = false;
        bool visible = true;
        float f1 = 0.0f;
        float f2 = 0.0f;
        float f3 = 0.0f;
        float f4 = 0.0f;
        Color color = Color(1, 1, 1, 1);
        String text;
    };

    struct TransitionItem {
        TransitionActionType type = TRANSITION_XY;
        StringName target_item_id;
        uint64_t target_instance_id = 0;
        float time = 0.0f;
        float duration = 0.0f;
        int32_t ease_type = 0;
        int32_t repeat = 0;
        bool yoyo = false;
        String label;
        String end_label;
        TransitionValue start_value;
        TransitionValue end_value;
        bool has_tween = false;

        int32_t anim_frame = -1;
        bool anim_playing = false;
        String anim_animation_name;
        String anim_skin_name;
        String sound_url;
        float sound_volume = 1.0f;
        String trans_target_name;
        int32_t trans_play_times = 1;
        float shake_amplitude = 0.0f;
        float shake_duration = 0.0f;
        Vector2 shake_last_offset;
        int32_t active_tween_id = 0;
        uint64_t display_lock_token = 0;
    };

    struct TransitionData {
        String name;
        bool auto_play = false;
        int32_t auto_play_times = 1;
        float auto_play_delay = 0.0f;
        float total_duration = 0.0f;
        Vector<TransitionItem> items;
    };

    String package_name;
    String component_name;
    Vector<ControllerData> controllers;
    Vector<TransitionData> transitions;
    GTween *transition_tween = nullptr;
    AudioStreamPlayer *transition_sound_player = nullptr;
    Vector2 previous_size;
    bool constructing = false;
    bool construct_requested = false;
    bool resource_constructed = false;

protected:
    virtual void construct_extension_from_package(const fgui::PackageItem &p_item);
    virtual void on_parent_controller_changed(int32_t p_parent_controller_index, const String &p_selected_page_id);

private:
    void setup_controllers(fgui::ByteBuffer &p_buffer);
    void setup_transitions(fgui::ByteBuffer &p_buffer, const Vector<GObject *> &p_children);
    void decode_transition_value(fgui::ByteBuffer &p_buffer, TransitionActionType p_type, TransitionValue &r_value);
    GTween *get_transition_tween();
    Object *resolve_transition_target(const TransitionItem &p_item);
    int32_t find_transition_index(const String &p_name) const;
    int32_t get_or_create_transition_index(const String &p_name);
    void apply_controller(int32_t p_controller_index);
    void apply_child_relations(const Vector2 &p_old_size, const Vector2 &p_new_size);
    void clear_constructed_children();
    void request_construct_from_resource();
    void apply_transition_value(Object *p_target, const TransitionItem &p_item);
    void stop_transition_item(TransitionItem &p_item, bool p_set_to_complete);
    void check_auto_play_transitions();
};

} // namespace godot
