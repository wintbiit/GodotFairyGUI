#pragma once

#include "g_component.h"

#include <godot_cpp/variant/callable.hpp>

namespace godot {

class GButton;

class GList : public GComponent {
    GDCLASS(GList, GComponent)

protected:
    static void _bind_methods();

public:
    void _gui_input(const Ref<InputEvent> &p_event) override;
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;
    void _process(double p_delta) override;

    int32_t get_layout() const;
    int32_t get_selection_mode() const;
    int32_t get_line_gap() const;
    int32_t get_column_gap() const;
    int32_t get_line_count() const;
    int32_t get_column_count() const;
    bool is_auto_resize_item() const;
    bool is_scroll_item_to_view_on_click() const;
    bool is_fold_invisible_items() const;
    int32_t get_scroll_type() const;
    int32_t get_scroll_bar_display() const;
    bool is_mouse_wheel_enabled() const;
    bool is_page_mode() const;
    bool is_snap_to_item() const;
    void set_default_item(const String &p_url);
    String get_default_item() const;
    int32_t get_declared_item_count() const;
    int32_t get_item_count() const;
    void set_virtual(bool p_loop = false);
    bool is_virtual() const;
    bool is_loop_virtual() const;
    void set_num_items(int32_t p_count);
    int32_t get_num_items() const;
    void set_item_renderer(const Callable &p_renderer);
    Callable get_item_renderer() const;
    void refresh_virtual_list();
    int32_t get_first_visible_index() const;
    int32_t get_virtual_child_count() const;
    int32_t get_reuse_pool_count() const;
    GObject *add_item_from_url(const String &p_url);
    void add_selection(int32_t p_index);
    void remove_selection(int32_t p_index);
    void clear_selection();
    bool is_selected(int32_t p_index) const;
    int32_t get_selected_index() const;
    void handle_item_click(GObject *p_item);
    void set_scroll_position(const Vector2 &p_position);
    Vector2 get_scroll_position() const;
    Vector2 get_content_size() const;
    void scroll_to_view(int32_t p_index);
    void set_scroll_step(float p_step);
    float get_scroll_step() const;
    bool handle_mouse_wheel(int32_t p_button_index);

private:
    fgui::ListLayoutType layout = fgui::ListLayoutType::SingleColumn;
    fgui::ListSelectionMode selection_mode = fgui::ListSelectionMode::Single;
    int32_t line_gap = 0;
    int32_t column_gap = 0;
    int32_t line_count = 0;
    int32_t column_count = 0;
    bool auto_resize_item = true;
    bool scroll_item_to_view_on_click = true;
    bool fold_invisible_items = false;
    fgui::ScrollType scroll_type = fgui::ScrollType::Vertical;
    fgui::ScrollBarDisplayType scroll_bar_display = fgui::ScrollBarDisplayType::Default;
    bool mouse_wheel_enabled = true;
    bool page_mode = false;
    bool snap_to_item = false;
    String default_item;
    int32_t declared_item_count = 0;
    int32_t last_selected_index = -1;
    Vector2 scroll_position;
    Vector2 content_size;
    float scroll_step = 40.0f;
    bool virtual_list = false;
    bool virtual_loop = false;
    int32_t num_items = 0;
    int32_t first_visible_index = 0;
    Vector2 virtual_item_size;
    Callable item_renderer;
    Vector<GObject *> virtual_items;
    Vector<int32_t> virtual_item_indices;
    Vector<GObject *> reuse_pool;
    Vector<int32_t> selected_indices;

    // Scroll physics
    bool scroll_dragging = false;
    Vector2 scroll_drag_last_pos;
    Vector2 scroll_velocity;
    float scroll_deceleration_rate = 0.95f;
    bool scroll_bounce_effect = true;
    bool scroll_touch_effect = true;
    bool scroll_inertia_disabled = false;
    Vector2 scroll_page_size;
    String vt_scroll_bar_res;
    String hz_scroll_bar_res;

    void process_scroll_physics(double p_delta);

    void read_declared_items(fgui::ByteBuffer &p_buffer);
    void setup_scroll(fgui::ByteBuffer &p_buffer);
    void setup_item(fgui::ByteBuffer &p_buffer, GObject *p_item);
    void relayout_items();
    void relayout_virtual_items();
    GObject *get_virtual_item_from_pool();
    void return_virtual_item_to_pool(GObject *p_item);
    void clear_virtual_items();
    void update_virtual_item_size(GObject *p_item);
    void render_virtual_item(int32_t p_index, GObject *p_item);
    int32_t get_physical_index_for_virtual_index(int32_t p_index) const;
    Vector2 clamp_scroll_position(const Vector2 &p_position) const;
    void bind_item(GObject *p_item);
    void set_item_selected(int32_t p_index, bool p_selected);
    const GButton *get_button_item(int32_t p_index) const;
    GButton *get_mutable_button_item(int32_t p_index);
};

} // namespace godot
