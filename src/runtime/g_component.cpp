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

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

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
    ClassDB::bind_method(D_METHOD("set_controller_selected_page_id", "controller_index", "page_id"), &GComponent::set_controller_selected_page_id);
    ClassDB::bind_method(D_METHOD("go_back_controller", "controller_index"), &GComponent::go_back_controller);
    ClassDB::bind_method(D_METHOD("get_controller_previous_index", "controller_index"), &GComponent::get_controller_previous_index);
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
    ClassDB::bind_method(D_METHOD("get_child_by_name", "name"), &GComponent::get_child_by_name);
    ClassDB::bind_method(D_METHOD("get_child_by_path", "path"), &GComponent::get_child_by_path);
    ClassDB::bind_method(D_METHOD("get_visible_child", "name"), &GComponent::get_visible_child);
    ClassDB::bind_method(D_METHOD("swap_children", "index1", "index2"), &GComponent::swap_children);
    ClassDB::bind_method(D_METHOD("swap_children_at", "index1", "index2"), &GComponent::swap_children_at);
    ClassDB::bind_method(D_METHOD("set_child_index_before", "child", "before_index"), &GComponent::set_child_index_before);
    ClassDB::bind_method(D_METHOD("get_controller_name", "controller_index"), &GComponent::get_controller_name);
    ClassDB::bind_method(D_METHOD("get_controller_page_id", "controller_index", "page_index"), &GComponent::get_controller_page_id);
    ClassDB::bind_method(D_METHOD("get_controller_page_name", "controller_index", "page_index"), &GComponent::get_controller_page_name);
    ClassDB::bind_method(D_METHOD("has_controller_page", "controller_index", "page_id"), &GComponent::has_controller_page);
    ClassDB::bind_method(D_METHOD("stop_transition", "name", "set_to_complete"), &GComponent::stop_transition, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("stop_transition_at", "index", "set_to_complete"), &GComponent::stop_transition_at, DEFVAL(true));
    ClassDB::bind_method(D_METHOD("set_transition_paused", "paused"), &GComponent::set_transition_paused);
    ClassDB::bind_method(D_METHOD("is_transition_playing"), &GComponent::is_transition_playing);
    ClassDB::bind_method(D_METHOD("get_transition_time_scale"), &GComponent::get_transition_time_scale);
    ClassDB::bind_method(D_METHOD("set_transition_time_scale", "time_scale"), &GComponent::set_transition_time_scale);

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

bool GComponent::set_controller_selected_page_id(int32_t p_controller_index, const String &p_page_id) {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return false;
    }
    const ControllerData &controller = controllers[p_controller_index];
    for (int32_t i = 0; i < controller.page_ids.size(); i++) {
        if (controller.page_ids[i] == p_page_id) {
            return set_controller_selected_index(p_controller_index, i);
        }
    }
    return false;
}

bool GComponent::go_back_controller(int32_t p_controller_index) {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return false;
    }
    const ControllerData &controller = controllers[p_controller_index];
    if (controller.previous_index < 0 || controller.previous_index >= controller.page_ids.size()) {
        return false;
    }
    return set_controller_selected_index(p_controller_index, controller.previous_index);
}

int32_t GComponent::get_controller_previous_index(int32_t p_controller_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) {
        return -1;
    }
    return controllers[p_controller_index].previous_index;
}

void GComponent::on_parent_controller_changed(int32_t p_parent_controller_index, const String &p_selected_page_id) {
    (void)p_parent_controller_index;
    (void)p_selected_page_id;
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

void GComponent::apply_controller(int32_t p_controller_index) {
    const String selected_page_id = get_controller_selected_page_id(p_controller_index);
    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *child = Object::cast_to<GObject>(get_child(i));
        if (child != nullptr) {
            child->apply_controller_state(p_controller_index, selected_page_id);
        }

        GComponent *child_comp = Object::cast_to<GComponent>(get_child(i));
        if (child_comp != nullptr) {
            child_comp->on_parent_controller_changed(p_controller_index, selected_page_id);
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

GObject *GComponent::get_child_by_name(const String &p_name) const {
    for (int32_t i = 0; i < get_child_count(); i++) {
        Node *child = get_child(i);
        if (child->get_name() == p_name) {
            GObject *obj = Object::cast_to<GObject>(child);
            if (obj != nullptr) return obj;
        }
    }
    return nullptr;
}

GObject *GComponent::get_child_by_path(const String &p_path) const {
    PackedStringArray parts = p_path.split("/");
    const GComponent *current = this;
    for (int32_t i = 0; i < parts.size(); i++) {
        if (parts[i].is_empty()) continue;
        GObject *found = nullptr;
        for (int32_t j = 0; j < current->get_child_count(); j++) {
            Node *child = current->get_child(j);
            if (child->get_name() == parts[i]) {
                GObject *obj = Object::cast_to<GObject>(child);
                if (obj != nullptr) {
                    found = obj;
                    break;
                }
            }
        }
        if (found == nullptr) return nullptr;
        GComponent *comp = Object::cast_to<GComponent>(found);
        if (comp != nullptr) { current = comp; continue; }
        return (i == parts.size() - 1) ? found : nullptr;
    }
    return const_cast<GComponent *>(current);
}

GObject *GComponent::get_visible_child(const String &p_name) const {
    GObject *child = get_child_by_name(p_name);
    return (child != nullptr && child->is_visible()) ? child : nullptr;
}

void GComponent::swap_children(int32_t p_index1, int32_t p_index2) { swap_children_at(p_index1, p_index2); }
void GComponent::swap_children_at(int32_t p_index1, int32_t p_index2) {
    if (p_index1 < 0 || p_index1 >= get_child_count()) return;
    if (p_index2 < 0 || p_index2 >= get_child_count()) return;
    if (p_index1 == p_index2) return;
    Node *child1 = get_child(p_index1);
    Node *child2 = get_child(p_index2);
    if (child1 == nullptr || child2 == nullptr) return;
    move_child(child1, p_index2);
}

void GComponent::set_child_index_before(GObject *p_child, int32_t p_before_index) {
    if (p_child == nullptr || p_child->get_parent() != this) return;
    int32_t current_index = p_child->get_index();
    if (current_index < 0) return;
    if (p_before_index > current_index) p_before_index--;
    move_child(p_child, CLAMP(p_before_index, 0, get_child_count() - 1));
}

String GComponent::get_controller_name(int32_t p_controller_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) return String();
    return controllers[p_controller_index].name;
}

String GComponent::get_controller_page_id(int32_t p_controller_index, int32_t p_page_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) return String();
    const ControllerData &c = controllers[p_controller_index];
    if (p_page_index < 0 || p_page_index >= c.page_ids.size()) return String();
    return c.page_ids[p_page_index];
}

String GComponent::get_controller_page_name(int32_t p_controller_index, int32_t p_page_index) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) return String();
    const ControllerData &c = controllers[p_controller_index];
    if (p_page_index < 0 || p_page_index >= c.page_names.size()) return String();
    return c.page_names[p_page_index];
}

bool GComponent::has_controller_page(int32_t p_controller_index, const String &p_page_id) const {
    if (p_controller_index < 0 || p_controller_index >= controllers.size()) return false;
    const ControllerData &c = controllers[p_controller_index];
    for (int32_t i = 0; i < c.page_ids.size(); i++) {
        if (c.page_ids[i] == p_page_id) return true;
    }
    return false;
}

void GComponent::stop_transition(const String &p_name, bool p_set_to_complete) {
    stop_transition_at(find_transition_index(p_name), p_set_to_complete);
}

void GComponent::stop_transition_at(int32_t p_index, bool p_set_to_complete) {
    if (p_index < 0 || p_index >= transitions.size()) return;
    TransitionData &transition = transitions.write[p_index];
    for (int32_t i = 0; i < transition.items.size(); i++) {
        stop_transition_item(transition.items.write[i], p_set_to_complete);
    }
    transition.active_play_index = -1;
    transition.paused = false;
}

void GComponent::set_transition_paused(bool p_paused) {
    for (int32_t i = 0; i < transitions.size(); i++) {
        transitions.write[i].paused = p_paused;
        for (int32_t j = 0; j < transitions[i].items.size(); j++) {
            TransitionItem &item = transitions.write[i].items.write[j];
            if (item.active_tween_id > 0 && transition_tween != nullptr) {
                transition_tween->set_tween_paused(item.active_tween_id, p_paused);
            }
        }
    }
}

bool GComponent::is_transition_playing() const {
    for (int32_t i = 0; i < transitions.size(); i++) {
        if (transitions[i].active_play_index >= 0 && !transitions[i].paused) return true;
    }
    return false;
}

float GComponent::get_transition_time_scale() const {
    for (int32_t i = 0; i < transitions.size(); i++) {
        if (transitions[i].active_play_index >= 0) return transitions[i].time_scale;
    }
    return 1.0f;
}

void GComponent::set_transition_time_scale(float p_time_scale) {
    for (int32_t i = 0; i < transitions.size(); i++) {
        transitions.write[i].time_scale = p_time_scale;
        for (int32_t j = 0; j < transitions[i].items.size(); j++) {
            TransitionItem &item = transitions.write[i].items.write[j];
            if (item.active_tween_id > 0 && transition_tween != nullptr) {
                transition_tween->set_tween_time_scale(item.active_tween_id, p_time_scale);
            }
        }
    }
}
