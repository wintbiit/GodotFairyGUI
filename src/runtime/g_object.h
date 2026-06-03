#pragma once

#include "package/package_defs.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace godot {

class GObject : public Control {
    GDCLASS(GObject, Control)

protected:
    static void _bind_methods();

public:
    GObject() = default;

    void _gui_input(const Ref<InputEvent> &p_event) override;

    void set_touchable(bool p_touchable);
    bool is_touchable() const;

    void set_package_item_id(const StringName &p_id);
    StringName get_package_item_id() const;
    virtual void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos);
    virtual void setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos);
    void apply_controller_visibility(int32_t p_controller_index, const String &p_selected_page_id);
    void apply_controller_state(int32_t p_controller_index, const String &p_selected_page_id);
    bool has_display_gear() const;
    int32_t get_common_gear_count() const;
    void setup_relations(fgui::ByteBuffer &p_buffer, bool p_parent_to_child);
    void apply_parent_relation_size_change(const Vector2 &p_old_parent_size, const Vector2 &p_new_parent_size);
    int32_t get_relation_count() const;

private:
    struct RelationDef {
        fgui::RelationType type = fgui::RelationType::Width;
        bool percent = false;
    };

    struct GearXYValue {
        Vector2 position;
        Vector2 percent;
    };

    struct GearSizeValue {
        Vector2 size;
        Vector2 scale = Vector2(1, 1);
    };

    struct GearLookValue {
        float alpha = 1.0f;
        float rotation = 0.0f;
        bool touchable = true;
    };

    bool touchable = true;
    bool declared_visible = true;
    bool gear_visible = true;
    bool has_gear_display = false;
    int32_t gear_display_controller_index = -1;
    Vector<String> gear_display_pages;
    bool has_gear_xy = false;
    bool gear_xy_positions_in_percent = false;
    int32_t gear_xy_controller_index = -1;
    GearXYValue gear_xy_default;
    HashMap<String, GearXYValue> gear_xy_values;
    bool has_gear_size = false;
    int32_t gear_size_controller_index = -1;
    GearSizeValue gear_size_default;
    HashMap<String, GearSizeValue> gear_size_values;
    bool has_gear_look = false;
    int32_t gear_look_controller_index = -1;
    GearLookValue gear_look_default;
    HashMap<String, GearLookValue> gear_look_values;
    StringName package_item_id;
    Vector<RelationDef> parent_relations;

    void setup_gear(fgui::ByteBuffer &p_buffer, uint8_t p_gear_index);
    void apply_gear_xy(const String &p_selected_page_id);
    void apply_gear_size(const String &p_selected_page_id);
    void apply_gear_look(const String &p_selected_page_id);
    void add_relation_def(fgui::RelationType p_type, bool p_percent);
};

} // namespace godot
