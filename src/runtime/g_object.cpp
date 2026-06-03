#include "g_object.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GObject::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_touchable", "touchable"), &GObject::set_touchable);
    ClassDB::bind_method(D_METHOD("is_touchable"), &GObject::is_touchable);
    ClassDB::bind_method(D_METHOD("set_package_item_id", "id"), &GObject::set_package_item_id);
    ClassDB::bind_method(D_METHOD("get_package_item_id"), &GObject::get_package_item_id);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &GObject::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &GObject::get_id);
    ClassDB::bind_method(D_METHOD("set_data", "data"), &GObject::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &GObject::get_data);
    ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &GObject::set_enabled);
    ClassDB::bind_method(D_METHOD("is_enabled"), &GObject::is_enabled);
    ClassDB::bind_method(D_METHOD("set_grayed", "grayed"), &GObject::set_grayed);
    ClassDB::bind_method(D_METHOD("is_grayed"), &GObject::is_grayed);
    ClassDB::bind_method(D_METHOD("set_tooltips", "tooltips"), &GObject::set_tooltips);
    ClassDB::bind_method(D_METHOD("get_tooltips"), &GObject::get_tooltips);
    ClassDB::bind_method(D_METHOD("local_to_global_pos", "local"), &GObject::local_to_global_pos);
    ClassDB::bind_method(D_METHOD("global_to_local_pos", "global"), &GObject::global_to_local_pos);
    ClassDB::bind_method(D_METHOD("root_to_local_pos", "root"), &GObject::root_to_local_pos);
    ClassDB::bind_method(D_METHOD("local_to_root_pos", "local"), &GObject::local_to_root_pos);
    ClassDB::bind_method(D_METHOD("add_relation", "target", "relation_type", "percent"), &GObject::add_relation, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("remove_relation", "target", "relation_type"), &GObject::remove_relation);
    ClassDB::bind_method(D_METHOD("get_relation_count"), &GObject::get_relation_count);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "touchable"), "set_touchable", "is_touchable");
    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "package_item_id"), "set_package_item_id", "get_package_item_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "set_id", "get_id");
    ADD_PROPERTY(PropertyInfo(Variant::NIL, "data"), "set_data", "get_data");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "grayed"), "set_grayed", "is_grayed");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "tooltips"), "set_tooltips", "get_tooltips");

    ADD_SIGNAL(MethodInfo("fgui_click"));
    ADD_SIGNAL(MethodInfo("fgui_right_click"));
    ADD_SIGNAL(MethodInfo("fgui_roll_over"));
    ADD_SIGNAL(MethodInfo("fgui_roll_out"));
    ADD_SIGNAL(MethodInfo("fgui_key_down"));
    ADD_SIGNAL(MethodInfo("fgui_focus_in"));
    ADD_SIGNAL(MethodInfo("fgui_focus_out"));
    ADD_SIGNAL(MethodInfo("fgui_drag_start"));
    ADD_SIGNAL(MethodInfo("fgui_drag_move"));
    ADD_SIGNAL(MethodInfo("fgui_drag_end"));
}

void GObject::_gui_input(const Ref<InputEvent> &p_event) {
    Ref<InputEventMouseButton> mouse_button = p_event;
    if (mouse_button.is_valid() && mouse_button->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT && !mouse_button->is_pressed()) {
        emit_signal("fgui_click");
        accept_event();
    }
}

void GObject::set_touchable(bool p_touchable) {
    touchable = p_touchable;
    set_mouse_filter(touchable ? MOUSE_FILTER_STOP : MOUSE_FILTER_IGNORE);
}

bool GObject::is_touchable() const {
    return touchable;
}

void GObject::set_package_item_id(const StringName &p_id) {
    package_item_id = p_id;
}

StringName GObject::get_package_item_id() const {
    return package_item_id;
}

void GObject::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    p_buffer.seek(p_begin_pos, 0);
    p_buffer.skip(5);

    set_package_item_id(StringName(p_buffer.read_s()));
    set_name(p_buffer.read_s());
    set_position(Vector2(p_buffer.read_int(), p_buffer.read_int()));

    if (p_buffer.read_bool()) {
        const int32_t width = p_buffer.read_int();
        const int32_t height = p_buffer.read_int();
        set_custom_minimum_size(Vector2(width, height));
        set_size(Vector2(width, height));
    }

    if (p_buffer.read_bool()) {
        p_buffer.skip(16); // min/max width/height
    }

    if (p_buffer.read_bool()) {
        set_scale(Vector2(p_buffer.read_float(), p_buffer.read_float()));
    }

    if (p_buffer.read_bool()) {
        p_buffer.skip(8); // skew
    }

    if (p_buffer.read_bool()) {
        p_buffer.read_float();
        p_buffer.read_float();
        p_buffer.read_bool(); // pivot as anchor
    }

    const float alpha = p_buffer.read_float();
    Color modulate = get_modulate();
    modulate.a = alpha;
    set_modulate(modulate);

    const float rotation_degrees = p_buffer.read_float();
    if (rotation_degrees != 0.0f) {
        set_rotation_degrees(rotation_degrees);
    }

    declared_visible = p_buffer.read_bool();
    set_visible(declared_visible && gear_visible);
    set_touchable(p_buffer.read_bool());

    p_buffer.read_bool(); // grayed
    p_buffer.read_byte(); // blend mode

    const uint8_t filter = p_buffer.read_byte();
    if (filter == 1) {
        p_buffer.skip(16); // brightness/contrast/saturation/hue
    }

    p_buffer.read_s(); // custom data
}

void GObject::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    if (!p_buffer.seek(p_begin_pos, 1)) {
        return;
    }

    p_buffer.read_s(); // tooltip
    p_buffer.read_short(); // group id

    if (!p_buffer.seek(p_begin_pos, 2)) {
        return;
    }

    const int32_t gear_count = p_buffer.read_short();
    for (int32_t i = 0; i < gear_count; i++) {
        const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();
        const uint8_t gear_index = p_buffer.read_byte();
        setup_gear(p_buffer, gear_index);
        p_buffer.set_position(next_pos);
    }
}

void GObject::apply_controller_visibility(int32_t p_controller_index, const String &p_selected_page_id) {
    if (!has_gear_display || gear_display_controller_index != p_controller_index) {
        return;
    }

    gear_visible = gear_display_pages.is_empty();
    for (int32_t i = 0; i < gear_display_pages.size(); i++) {
        if (gear_display_pages[i] == p_selected_page_id) {
            gear_visible = true;
            break;
        }
    }
    set_visible(declared_visible && gear_visible);
}

void GObject::apply_controller_state(int32_t p_controller_index, const String &p_selected_page_id) {
    apply_controller_visibility(p_controller_index, p_selected_page_id);
    if (has_gear_xy && gear_xy_controller_index == p_controller_index) {
        apply_gear_xy(p_selected_page_id);
    }
    if (has_gear_size && gear_size_controller_index == p_controller_index) {
        apply_gear_size(p_selected_page_id);
    }
    if (has_gear_look && gear_look_controller_index == p_controller_index) {
        apply_gear_look(p_selected_page_id);
    }
}

bool GObject::has_display_gear() const {
    return has_gear_display;
}

int32_t GObject::get_common_gear_count() const {
    return (has_gear_display ? 1 : 0) + (has_gear_xy ? 1 : 0) + (has_gear_size ? 1 : 0) + (has_gear_look ? 1 : 0);
}

void GObject::setup_relations(fgui::ByteBuffer &p_buffer, bool p_parent_to_child) {
    const int32_t relation_item_count = p_buffer.read_byte();
    for (int32_t i = 0; i < relation_item_count; i++) {
        const int32_t target_index = p_buffer.read_short();
        const int32_t relation_count = p_buffer.read_byte();
        const bool targets_parent = target_index == -1 || !p_parent_to_child;
        for (int32_t j = 0; j < relation_count; j++) {
            const fgui::RelationType type = static_cast<fgui::RelationType>(p_buffer.read_byte());
            const bool percent = p_buffer.read_bool();
            if (targets_parent) {
                add_relation_def(type, percent);
            }
        }
    }
}

void GObject::apply_parent_relation_size_change(const Vector2 &p_old_parent_size, const Vector2 &p_new_parent_size) {
    if (parent_relations.is_empty()) {
        return;
    }

    const Vector2 delta = p_new_parent_size - p_old_parent_size;
    Vector2 position = get_position();
    Vector2 size = get_size();

    for (int32_t i = 0; i < parent_relations.size(); i++) {
        const RelationDef &relation = parent_relations[i];
        switch (relation.type) {
            case fgui::RelationType::Left_Left:
            case fgui::RelationType::Left_Center:
            case fgui::RelationType::Left_Right:
            case fgui::RelationType::Center_Center:
            case fgui::RelationType::Right_Left:
            case fgui::RelationType::Right_Center:
            case fgui::RelationType::Right_Right:
                position.x += relation.percent && p_old_parent_size.x != 0.0f ? (position.x * p_new_parent_size.x / p_old_parent_size.x - position.x) : delta.x;
                break;
            case fgui::RelationType::Top_Top:
            case fgui::RelationType::Top_Middle:
            case fgui::RelationType::Top_Bottom:
            case fgui::RelationType::Middle_Middle:
            case fgui::RelationType::Bottom_Top:
            case fgui::RelationType::Bottom_Middle:
            case fgui::RelationType::Bottom_Bottom:
                position.y += relation.percent && p_old_parent_size.y != 0.0f ? (position.y * p_new_parent_size.y / p_old_parent_size.y - position.y) : delta.y;
                break;
            case fgui::RelationType::Width:
                size.x = relation.percent && p_old_parent_size.x != 0.0f ? size.x * p_new_parent_size.x / p_old_parent_size.x : size.x + delta.x;
                break;
            case fgui::RelationType::Height:
                size.y = relation.percent && p_old_parent_size.y != 0.0f ? size.y * p_new_parent_size.y / p_old_parent_size.y : size.y + delta.y;
                break;
            case fgui::RelationType::LeftExt_Left:
            case fgui::RelationType::LeftExt_Right:
                position.x += delta.x;
                size.x -= delta.x;
                break;
            case fgui::RelationType::RightExt_Left:
            case fgui::RelationType::RightExt_Right:
                size.x += delta.x;
                break;
            case fgui::RelationType::TopExt_Top:
            case fgui::RelationType::TopExt_Bottom:
                position.y += delta.y;
                size.y -= delta.y;
                break;
            case fgui::RelationType::BottomExt_Top:
            case fgui::RelationType::BottomExt_Bottom:
                size.y += delta.y;
                break;
            case fgui::RelationType::Size:
                break;
        }
    }

    set_position(position);
    set_custom_minimum_size(size);
    set_size(size);
}

int32_t GObject::get_relation_count() const {
    return parent_relations.size();
}

void GObject::setup_gear(fgui::ByteBuffer &p_buffer, uint8_t p_gear_index) {
    const int32_t controller_index = p_buffer.read_short();
    const int32_t page_count = p_buffer.read_short();

    if (p_gear_index == 0) {
        gear_display_controller_index = controller_index;
        gear_display_pages = p_buffer.read_s_array(page_count);
        has_gear_display = true;
        return;
    }

    if (p_gear_index == 1) {
        gear_xy_controller_index = controller_index;
        has_gear_xy = true;
        for (int32_t i = 0; i < page_count; i++) {
            const String page_id = p_buffer.read_s();
            Vector2 pos = Vector2(p_buffer.read_int(), p_buffer.read_int());
            if (!page_id.is_empty()) {
                GearXYValue value;
                value.position = pos;
                gear_xy_values[page_id] = value;
            }
        }
        if (p_buffer.read_bool()) {
            gear_xy_default.position = Vector2(p_buffer.read_int(), p_buffer.read_int());
        } else {
            gear_xy_default.position = get_position();
        }
        if (p_buffer.read_bool()) {
            p_buffer.read_byte();
            p_buffer.read_float();
            p_buffer.read_float();
        }
        if (p_buffer.version >= 2 && p_buffer.read_bool()) {
            gear_xy_positions_in_percent = true;
            for (int32_t i = 0; i < page_count; i++) {
                const String page_id = p_buffer.read_s();
                if (page_id.is_empty() || !gear_xy_values.has(page_id)) {
                    continue;
                }
                GearXYValue value = gear_xy_values[page_id];
                value.percent = Vector2(p_buffer.read_float(), p_buffer.read_float());
                gear_xy_values[page_id] = value;
            }
            if (p_buffer.read_bool()) {
                gear_xy_default.percent = Vector2(p_buffer.read_float(), p_buffer.read_float());
            }
        }
        return;
    }

    if (p_gear_index == 2) {
        gear_size_controller_index = controller_index;
        has_gear_size = true;
        for (int32_t i = 0; i < page_count; i++) {
            const String page_id = p_buffer.read_s();
            Vector2 sz = Vector2(p_buffer.read_int(), p_buffer.read_int());
            Vector2 sc = Vector2(p_buffer.read_float(), p_buffer.read_float());
            if (!page_id.is_empty()) {
                GearSizeValue value;
                value.size = sz;
                value.scale = sc;
                gear_size_values[page_id] = value;
            }
        }
        if (p_buffer.read_bool()) {
            gear_size_default.size = Vector2(p_buffer.read_int(), p_buffer.read_int());
            gear_size_default.scale = Vector2(p_buffer.read_float(), p_buffer.read_float());
        } else {
            gear_size_default.size = get_size();
            gear_size_default.scale = get_scale();
        }
        if (p_buffer.read_bool()) {
            p_buffer.read_byte();
            p_buffer.read_float();
            p_buffer.read_float();
        }
        return;
    }

    if (p_gear_index == 3) {
        gear_look_controller_index = controller_index;
        has_gear_look = true;
        for (int32_t i = 0; i < page_count; i++) {
            const String page_id = p_buffer.read_s();
            float alpha = p_buffer.read_float();
            float rotation = p_buffer.read_float();
            p_buffer.read_bool(); // grayed
            bool touchable = p_buffer.read_bool();
            if (!page_id.is_empty()) {
                GearLookValue value;
                value.alpha = alpha;
                value.rotation = rotation;
                value.touchable = touchable;
                gear_look_values[page_id] = value;
            }
        }
        if (p_buffer.read_bool()) {
            gear_look_default.alpha = p_buffer.read_float();
            gear_look_default.rotation = p_buffer.read_float();
            p_buffer.read_bool(); // grayed
            gear_look_default.touchable = p_buffer.read_bool();
        } else {
            gear_look_default.alpha = get_modulate().a;
            gear_look_default.rotation = get_rotation_degrees();
            gear_look_default.touchable = is_touchable();
        }
        if (p_buffer.read_bool()) {
            p_buffer.read_byte();
            p_buffer.read_float();
            p_buffer.read_float();
        }
    }
}

void GObject::apply_gear_xy(const String &p_selected_page_id) {
    const GearXYValue *found = gear_xy_values.getptr(p_selected_page_id);
    GearXYValue value = found ? *found : gear_xy_default;
    if (gear_xy_positions_in_percent && get_parent() != nullptr) {
        const Control *parent_control = Object::cast_to<Control>(get_parent());
        if (parent_control != nullptr) {
            set_position(Vector2(value.percent.x * parent_control->get_size().x, value.percent.y * parent_control->get_size().y));
            return;
        }
    }
    set_position(value.position);
}

void GObject::apply_gear_size(const String &p_selected_page_id) {
    const GearSizeValue *found = gear_size_values.getptr(p_selected_page_id);
    GearSizeValue value = found ? *found : gear_size_default;
    set_custom_minimum_size(value.size);
    set_size(value.size);
    set_scale(value.scale);
}

void GObject::apply_gear_look(const String &p_selected_page_id) {
    const GearLookValue *found = gear_look_values.getptr(p_selected_page_id);
    GearLookValue value = found ? *found : gear_look_default;
    Color modulate = get_modulate();
    modulate.a = value.alpha;
    set_modulate(modulate);
    set_rotation_degrees(value.rotation);
    set_touchable(value.touchable);
}

void GObject::add_relation_def(fgui::RelationType p_type, bool p_percent) {
    if (p_type == fgui::RelationType::Size) {
        add_relation_def(fgui::RelationType::Width, p_percent);
        add_relation_def(fgui::RelationType::Height, p_percent);
        return;
    }

    RelationDef relation;
    relation.type = p_type;
    relation.percent = p_percent;
    parent_relations.push_back(relation);
}

void GObject::set_id(const String &p_id) { package_item_id = StringName(p_id); }
String GObject::get_id() const { return package_item_id; }

void GObject::set_data(const Variant &p_data) { user_data = p_data; }
Variant GObject::get_data() const { return user_data; }

void GObject::set_enabled(bool p_enabled) {
    set_grayed(!p_enabled);
    set_touchable(p_enabled);
}
bool GObject::is_enabled() const { return !is_grayed() && is_touchable(); }

void GObject::set_grayed(bool p_grayed) { grayed = p_grayed; }
bool GObject::is_grayed() const { return grayed; }

void GObject::set_tooltips(const String &p_tooltips) { tooltips = p_tooltips; }
String GObject::get_tooltips() const { return tooltips; }

Vector2 GObject::local_to_global_pos(const Vector2 &p_local) const {
    if (!is_inside_tree()) return p_local + get_position();
    return get_screen_transform().xform(p_local);
}
Vector2 GObject::global_to_local_pos(const Vector2 &p_global) const {
    if (!is_inside_tree()) return p_global - get_position();
    return get_screen_transform().affine_inverse().xform(p_global);
}
Vector2 GObject::root_to_local_pos(const Vector2 &p_root) const {
    if (!is_inside_tree() || get_tree() == nullptr) return p_root;
    return get_screen_transform().affine_inverse().xform(p_root);
}
Vector2 GObject::local_to_root_pos(const Vector2 &p_local) const {
    if (!is_inside_tree() || get_tree() == nullptr) return p_local;
    return get_screen_transform().xform(p_local);
}

void GObject::add_relation(GObject *p_target, int32_t p_relation_type, bool p_percent) {
    if (p_target == nullptr) return;
    RelationDef def;
    def.type = static_cast<fgui::RelationType>(p_relation_type);
    def.percent = p_percent;
    parent_relations.push_back(def);
}
void GObject::remove_relation(GObject *p_target, int32_t p_relation_type) {
    (void)p_target;
    fgui::RelationType type = static_cast<fgui::RelationType>(p_relation_type);
    for (int32_t i = parent_relations.size() - 1; i >= 0; i--) {
        if (parent_relations[i].type == type) {
            parent_relations.remove_at(i);
        }
    }
}
