#include "g_component.h"

#include "g_button.h"
#include "g_combo_box.h"
#include "g_graph.h"
#include "g_image.h"
#include "g_label.h"
#include "g_list.h"
#include "g_loader.h"
#include "g_movie_clip.h"
#include "g_progress_bar.h"
#include "g_scroll_bar.h"
#include "g_slider.h"
#include "g_text_field.h"
#include "g_text_input.h"
#include "g_tween.h"
#include "ui_package.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/memory.hpp>

using namespace godot;

void GComponent::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_package_name", "name"), &GComponent::set_package_name);
    ClassDB::bind_method(D_METHOD("get_package_name"), &GComponent::get_package_name);
    ClassDB::bind_method(D_METHOD("set_component_name", "name"), &GComponent::set_component_name);
    ClassDB::bind_method(D_METHOD("get_component_name"), &GComponent::get_component_name);
    ClassDB::bind_method(D_METHOD("construct_from_resource"), &GComponent::construct_from_resource);
    ClassDB::bind_method(D_METHOD("count_descendants_by_class", "class_name"), &GComponent::count_descendants_by_class);
    ClassDB::bind_method(D_METHOD("get_controller_count"), &GComponent::get_controller_count);
    ClassDB::bind_method(D_METHOD("get_controller_page_count", "controller_index"), &GComponent::get_controller_page_count);
    ClassDB::bind_method(D_METHOD("get_controller_selected_page_id", "controller_index"), &GComponent::get_controller_selected_page_id);
    ClassDB::bind_method(D_METHOD("set_controller_selected_index", "controller_index", "page_index"), &GComponent::set_controller_selected_index);
    ClassDB::bind_method(D_METHOD("count_hidden_descendants"), &GComponent::count_hidden_descendants);
    ClassDB::bind_method(D_METHOD("count_display_gear_descendants"), &GComponent::count_display_gear_descendants);
    ClassDB::bind_method(D_METHOD("count_common_gear_descendants"), &GComponent::count_common_gear_descendants);
    ClassDB::bind_method(D_METHOD("count_relation_descendants"), &GComponent::count_relation_descendants);
    ClassDB::bind_method(D_METHOD("get_transition_count"), &GComponent::get_transition_count);
    ClassDB::bind_method(D_METHOD("get_transition_name", "index"), &GComponent::get_transition_name);
    ClassDB::bind_method(D_METHOD("get_transition_item_count", "index"), &GComponent::get_transition_item_count);
    ClassDB::bind_method(D_METHOD("play_transition", "name"), &GComponent::play_transition);
    ClassDB::bind_method(D_METHOD("play_transition_at", "index"), &GComponent::play_transition_at);
    ClassDB::bind_method(D_METHOD("advance_transitions", "delta"), &GComponent::advance_transitions);
    ClassDB::bind_method(D_METHOD("add_transition_xy", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_xy, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_size", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_size, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_scale", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_scale, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_alpha", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_alpha, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_rotation", "name", "target", "end_degrees", "duration", "delay"), &GComponent::add_transition_rotation, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_color", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_color, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_visible", "name", "target", "visible", "delay"), &GComponent::add_transition_visible, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_text", "name", "target", "text", "delay"), &GComponent::add_transition_text, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_icon", "name", "target", "icon", "delay"), &GComponent::add_transition_icon, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_pivot", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_pivot, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_shake", "name", "target", "amplitude", "duration", "delay"), &GComponent::add_transition_shake, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_color_filter", "name", "target", "end", "duration", "delay"), &GComponent::add_transition_color_filter, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("add_transition_sound", "name", "target", "sound_url", "volume", "delay"), &GComponent::add_transition_sound, DEFVAL(1.0), DEFVAL(0.0));

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "package_name"), "set_package_name", "get_package_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "component_name"), "set_component_name", "get_component_name");
}

void GComponent::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        request_construct_from_resource();
        check_auto_play_transitions();
        return;
    }

    if (p_what != NOTIFICATION_RESIZED) {
        return;
    }

    const Vector2 current_size = get_size();
    if (constructing || previous_size == Vector2()) {
        previous_size = current_size;
        return;
    }

    if (current_size != previous_size) {
        apply_child_relations(previous_size, current_size);
        previous_size = current_size;
    }
}

void GComponent::set_package_name(const String &p_name) {
    if (package_name == p_name) {
        return;
    }
    package_name = p_name;
    resource_constructed = false;
    request_construct_from_resource();
}

String GComponent::get_package_name() const {
    return package_name;
}

void GComponent::set_component_name(const String &p_name) {
    if (component_name == p_name) {
        return;
    }
    component_name = p_name;
    resource_constructed = false;
    request_construct_from_resource();
}

String GComponent::get_component_name() const {
    return component_name;
}

bool GComponent::construct_from_resource() {
    if (package_name.is_empty() || component_name.is_empty()) {
        return false;
    }

    construct_requested = false;
    const bool constructed = UIPackage::construct_component(this, package_name, component_name);
    resource_constructed = constructed;
    return constructed;
}

bool GComponent::construct_from_package(const String &p_package_id, const fgui::PackageItem &p_item) {
    clear_constructed_children();
    constructing = true;
    package_name = p_package_id;
    component_name = !p_item.name.is_empty() ? p_item.name : p_item.id;
    transition_tween = nullptr;
    controllers.clear();
    transitions.clear();

    fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 0)) {
        constructing = false;
        return false;
    }

    const int32_t source_width = buffer.read_int();
    const int32_t source_height = buffer.read_int();
    set_custom_minimum_size(Vector2(source_width, source_height));
    set_size(Vector2(source_width, source_height));

    if (buffer.read_bool()) {
        buffer.skip(16); // min/max width/height
    }
    if (buffer.read_bool()) {
        buffer.read_float();
        buffer.read_float();
        buffer.read_bool(); // pivot
    }
    if (buffer.read_bool()) {
        buffer.skip(16); // margin
    }

    const uint8_t overflow = buffer.read_byte();
    set_clip_contents(overflow != 0);

    if (buffer.read_bool()) {
        buffer.skip(8); // clip softness
    }

    if (!buffer.seek(0, 2)) {
        constructing = false;
        return false;
    }

    setup_controllers(buffer);

    if (!buffer.seek(0, 2)) {
        constructing = false;
        return false;
    }

    const int32_t child_count = buffer.read_short();
    Vector<GObject *> children;
    children.resize(child_count);
    for (int32_t i = 0; i < child_count; i++) {
        const int32_t data_len = buffer.read_short();
        const int64_t child_begin_pos = buffer.get_position();

        buffer.seek(child_begin_pos, 0);
        const fgui::ObjectType type = static_cast<fgui::ObjectType>(buffer.read_byte());
        const String src = buffer.read_s();
        const String pkg_id = buffer.read_s();

        GObject *child = nullptr;
        const String child_package_id = !pkg_id.is_empty() ? pkg_id : p_package_id;
        if (!src.is_empty()) {
            child = UIPackage::create_object(child_package_id, src);
        }

        if (child == nullptr) {
            switch (type) {
                case fgui::ObjectType::Image:
                    child = memnew(GImage);
                    break;
                case fgui::ObjectType::MovieClip:
                    child = memnew(GMovieClip);
                    break;
                case fgui::ObjectType::Graph:
                    child = memnew(GGraph);
                    break;
                case fgui::ObjectType::Loader:
                case fgui::ObjectType::Loader3D:
                    child = memnew(GLoader);
                    break;
                case fgui::ObjectType::Text:
                case fgui::ObjectType::RichText:
                    child = memnew(GTextField);
                    break;
                case fgui::ObjectType::InputText:
                    child = memnew(GTextInput);
                    break;
                case fgui::ObjectType::Component:
                    child = memnew(GComponent);
                    break;
                case fgui::ObjectType::List:
                    child = memnew(GList);
                    break;
                case fgui::ObjectType::Button:
                    child = memnew(GButton);
                    break;
                case fgui::ObjectType::ComboBox:
                    child = memnew(GComboBox);
                    break;
                case fgui::ObjectType::Label:
                    child = memnew(GLabel);
                    break;
                case fgui::ObjectType::ProgressBar:
                    child = memnew(GProgressBar);
                    break;
                case fgui::ObjectType::Slider:
                    child = memnew(GSlider);
                    break;
                case fgui::ObjectType::ScrollBar:
                    child = memnew(GScrollBar);
                    break;
                default:
                    child = memnew(GObject);
                    break;
            }
        }

        child->setup_before_add(buffer, child_begin_pos);
        add_child(child);
        children.set(i, child);
        buffer.set_position(child_begin_pos + data_len);
    }

    if (buffer.seek(0, 2)) {
        buffer.skip(2);
        for (int32_t i = 0; i < child_count; i++) {
            const int32_t next_pos = buffer.read_ushort() + buffer.get_position();
            const int64_t child_begin_pos = buffer.get_position();
            GObject *child = children[i];
            if (child != nullptr) {
                if (buffer.seek(child_begin_pos, 3)) {
                    child->setup_relations(buffer, false);
                }
            }
            buffer.set_position(next_pos);
        }
    }

    if (buffer.seek(0, 2)) {
        buffer.skip(2);
        for (int32_t i = 0; i < child_count; i++) {
            const int32_t next_pos = buffer.read_ushort() + buffer.get_position();
            const int64_t child_begin_pos = buffer.get_position();
            GObject *child = children[i];
            if (child != nullptr) {
                child->setup_after_add(buffer, child_begin_pos);
            }
            buffer.set_position(next_pos);
        }
    }

    if (buffer.seek(0, 3)) {
        setup_relations(buffer, true);
    }

    setup_transitions(buffer, children);
    construct_extension_from_package(p_item);

    for (int32_t i = 0; i < controllers.size(); i++) {
        apply_controller(i);
    }

    previous_size = get_size();
    constructing = false;
    construct_requested = false;
    resource_constructed = true;
    return true;
}

void GComponent::construct_extension_from_package(const fgui::PackageItem &p_item) {
    (void)p_item;
}

int32_t GComponent::count_descendants_by_class(const String &p_class_name) const {
    int32_t count = 0;
    for (int32_t i = 0; i < get_child_count(); i++) {
        const Node *child = get_child(i);
        if (child->is_class(p_class_name)) {
            count++;
        }

        const GComponent *child_component = Object::cast_to<GComponent>(child);
        if (child_component != nullptr) {
            count += child_component->count_descendants_by_class(p_class_name);
        }
    }
    return count;
}

int32_t GComponent::get_controller_count() const {
    return controllers.size();
}

int32_t GComponent::get_controller_page_count(int32_t p_controller_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return 0;
    }
    return controllers[p_controller_index].page_ids.size();
}

String GComponent::get_controller_selected_page_id(int32_t p_controller_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return String();
    }

    const ControllerData &controller = controllers[p_controller_index];
    if (controller.selected_index < 0 || controller.selected_index >= controller.page_ids.size()) {
        return String();
    }
    return controller.page_ids[controller.selected_index];
}

bool GComponent::set_controller_selected_index(int32_t p_controller_index, int32_t p_page_index) {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return false;
    }

    ControllerData &controller = controllers.write[p_controller_index];
    if (p_page_index < 0 || p_page_index >= controller.page_ids.size()) {
        return false;
    }

    if (controller.selected_index == p_page_index) {
        return true;
    }

    controller.previous_index = controller.selected_index;
    controller.selected_index = p_page_index;
    apply_controller(p_controller_index);
    return true;
}

int32_t GComponent::count_hidden_descendants() const {
    int32_t count = 0;
    for (int32_t i = 0; i < get_child_count(); i++) {
        const Node *child = get_child(i);
        const CanvasItem *canvas_item = Object::cast_to<CanvasItem>(child);
        if (canvas_item != nullptr && !canvas_item->is_visible()) {
            count++;
        }

        const GComponent *child_component = Object::cast_to<GComponent>(child);
        if (child_component != nullptr) {
            count += child_component->count_hidden_descendants();
        }
    }
    return count;
}

int32_t GComponent::count_display_gear_descendants() const {
    int32_t count = 0;
    for (int32_t i = 0; i < get_child_count(); i++) {
        const GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr && child->has_display_gear()) {
            count++;
        }

        const GComponent *child_component = Object::cast_to<GComponent>(get_child(i));
        if (child_component != nullptr) {
            count += child_component->count_display_gear_descendants();
        }
    }
    return count;
}

int32_t GComponent::count_common_gear_descendants() const {
    int32_t count = 0;
    for (int32_t i = 0; i < get_child_count(); i++) {
        const GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr) {
            count += child->get_common_gear_count();
        }

        const GComponent *child_component = Object::cast_to<GComponent>(get_child(i));
        if (child_component != nullptr) {
            count += child_component->count_common_gear_descendants();
        }
    }
    return count;
}

int32_t GComponent::count_relation_descendants() const {
    int32_t count = 0;
    for (int32_t i = 0; i < get_child_count(); i++) {
        const GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr) {
            count += child->get_relation_count();
        }

        const GComponent *child_component = Object::cast_to<GComponent>(get_child(i));
        if (child_component != nullptr) {
            count += child_component->count_relation_descendants();
        }
    }
    return count;
}

int32_t GComponent::get_transition_count() const {
    return transitions.size();
}

String GComponent::get_transition_name(int32_t p_index) const {
    if (p_index < 0 || p_index >= transitions.size()) {
        return String();
    }
    return transitions[p_index].name;
}

int32_t GComponent::get_transition_item_count(int32_t p_index) const {
    if (p_index < 0 || p_index >= transitions.size()) {
        return 0;
    }
    return transitions[p_index].items.size();
}

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
                tween_id = tween->to_property(control, StringName("pivot_offset"), Vector2(item.end_value.f1, item.end_value.f2), item.has_tween ? item.duration : 0.0);
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
                        AudioStreamPlayer *player = memnew(AudioStreamPlayer);
                        player->set_stream(stream);
                        player->set_max_polyphony(4);
                        if (item.sound_volume > 0.0f && item.sound_volume != 1.0f) {
                            double db = 20.0 * Math::log(static_cast<double>(MAX(0.0f, item.sound_volume))) / Math::log(10.0);
                            player->set_volume_db(static_cast<float>(db));
                        }
                        add_child(player);
                        player->play();
                        player->connect("finished", Callable(player, "queue_free"), Object::CONNECT_ONE_SHOT);
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
                // Skew not supported on Godot Control — no-op
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

void GComponent::setup_controllers(fgui::ByteBuffer &p_buffer) {
    controllers.clear();
    if (!p_buffer.seek(0, 1)) {
        return;
    }

    const int32_t controller_count = p_buffer.read_short();
    controllers.resize(controller_count);
    for (int32_t i = 0; i < controller_count; i++) {
        const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();
        const int64_t begin_pos = p_buffer.get_position();

        ControllerData controller;
        if (p_buffer.seek(begin_pos, 0)) {
            controller.name = p_buffer.read_s();
            p_buffer.read_bool(); // autoRadioGroupDepth
        }

        if (p_buffer.seek(begin_pos, 1)) {
            const int32_t page_count = p_buffer.read_short();
            controller.page_ids.resize(page_count);
            controller.page_names.resize(page_count);
            for (int32_t page = 0; page < page_count; page++) {
                controller.page_ids.set(page, p_buffer.read_s());
                controller.page_names.set(page, p_buffer.read_s());
            }

            int32_t home_page_index = 0;
            if (p_buffer.version >= 2) {
                const uint8_t home_page_type = p_buffer.read_byte();
                if (home_page_type == 1) {
                    home_page_index = p_buffer.read_short();
                } else if (home_page_type == 3) {
                    p_buffer.read_s();
                }
            }
            controller.selected_index = page_count > 0 ? CLAMP(home_page_index, 0, page_count - 1) : -1;
        }

        controllers.set(i, controller);
        p_buffer.set_position(next_pos);
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
        p_buffer.read_int(); // options
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
                // Parse non-tween type-specific data
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

void GComponent::apply_controller(int32_t p_controller_index) {
    const String selected_page_id = get_controller_selected_page_id(p_controller_index);
    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr) {
            child->apply_controller_state(p_controller_index, selected_page_id);
        }
    }
}

void GComponent::apply_child_relations(const Vector2 &p_old_size, const Vector2 &p_new_size) {
    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr) {
            child->apply_parent_relation_size_change(p_old_size, p_new_size);
        }
    }
}

void GComponent::clear_constructed_children() {
    while (get_child_count() > 0) {
        Node *child = get_child(get_child_count() - 1);
        remove_child(child);
        memdelete(child);
    }
    transition_tween = nullptr;
}

void GComponent::request_construct_from_resource() {
    if (constructing || resource_constructed || package_name.is_empty() || component_name.is_empty()) {
        return;
    }
    if (!is_inside_tree()) {
        construct_requested = true;
        return;
    }
    construct_from_resource();
}
