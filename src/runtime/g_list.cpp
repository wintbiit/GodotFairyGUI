#include "g_list.h"

#include "g_button.h"
#include "g_scroll_bar.h"
#include "g_text_field.h"
#include "ui_package.h"

#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

void GList::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_layout"), &GList::get_layout);
    ClassDB::bind_method(D_METHOD("get_selection_mode"), &GList::get_selection_mode);
    ClassDB::bind_method(D_METHOD("get_line_gap"), &GList::get_line_gap);
    ClassDB::bind_method(D_METHOD("get_column_gap"), &GList::get_column_gap);
    ClassDB::bind_method(D_METHOD("get_line_count"), &GList::get_line_count);
    ClassDB::bind_method(D_METHOD("get_column_count"), &GList::get_column_count);
    ClassDB::bind_method(D_METHOD("is_auto_resize_item"), &GList::is_auto_resize_item);
    ClassDB::bind_method(D_METHOD("is_scroll_item_to_view_on_click"), &GList::is_scroll_item_to_view_on_click);
    ClassDB::bind_method(D_METHOD("is_fold_invisible_items"), &GList::is_fold_invisible_items);
    ClassDB::bind_method(D_METHOD("get_scroll_type"), &GList::get_scroll_type);
    ClassDB::bind_method(D_METHOD("get_scroll_bar_display"), &GList::get_scroll_bar_display);
    ClassDB::bind_method(D_METHOD("is_mouse_wheel_enabled"), &GList::is_mouse_wheel_enabled);
    ClassDB::bind_method(D_METHOD("is_page_mode"), &GList::is_page_mode);
    ClassDB::bind_method(D_METHOD("is_snap_to_item"), &GList::is_snap_to_item);
    ClassDB::bind_method(D_METHOD("set_default_item", "url"), &GList::set_default_item);
    ClassDB::bind_method(D_METHOD("get_default_item"), &GList::get_default_item);
    ClassDB::bind_method(D_METHOD("get_declared_item_count"), &GList::get_declared_item_count);
    ClassDB::bind_method(D_METHOD("get_item_count"), &GList::get_item_count);
    ClassDB::bind_method(D_METHOD("set_virtual", "loop"), &GList::set_virtual, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("is_virtual"), &GList::is_virtual);
    ClassDB::bind_method(D_METHOD("is_loop_virtual"), &GList::is_loop_virtual);
    ClassDB::bind_method(D_METHOD("set_num_items", "count"), &GList::set_num_items);
    ClassDB::bind_method(D_METHOD("get_num_items"), &GList::get_num_items);
    ClassDB::bind_method(D_METHOD("set_item_renderer", "renderer"), &GList::set_item_renderer);
    ClassDB::bind_method(D_METHOD("get_item_renderer"), &GList::get_item_renderer);
    ClassDB::bind_method(D_METHOD("refresh_virtual_list"), &GList::refresh_virtual_list);
    ClassDB::bind_method(D_METHOD("get_first_visible_index"), &GList::get_first_visible_index);
    ClassDB::bind_method(D_METHOD("get_virtual_child_count"), &GList::get_virtual_child_count);
    ClassDB::bind_method(D_METHOD("get_reuse_pool_count"), &GList::get_reuse_pool_count);
    ClassDB::bind_method(D_METHOD("add_item_from_url", "url"), &GList::add_item_from_url);
    ClassDB::bind_method(D_METHOD("add_selection", "index"), &GList::add_selection);
    ClassDB::bind_method(D_METHOD("remove_selection", "index"), &GList::remove_selection);
    ClassDB::bind_method(D_METHOD("clear_selection"), &GList::clear_selection);
    ClassDB::bind_method(D_METHOD("is_selected", "index"), &GList::is_selected);
    ClassDB::bind_method(D_METHOD("get_selected_index"), &GList::get_selected_index);
    ClassDB::bind_method(D_METHOD("handle_item_click", "item"), &GList::handle_item_click);
    ClassDB::bind_method(D_METHOD("set_scroll_position", "position"), &GList::set_scroll_position);
    ClassDB::bind_method(D_METHOD("get_scroll_position"), &GList::get_scroll_position);
    ClassDB::bind_method(D_METHOD("get_content_size"), &GList::get_content_size);
    ClassDB::bind_method(D_METHOD("scroll_to_view", "index"), &GList::scroll_to_view);
    ClassDB::bind_method(D_METHOD("set_scroll_step", "step"), &GList::set_scroll_step);
    ClassDB::bind_method(D_METHOD("get_scroll_step"), &GList::get_scroll_step);
    ClassDB::bind_method(D_METHOD("handle_mouse_wheel", "button_index"), &GList::handle_mouse_wheel);

    ADD_SIGNAL(MethodInfo("fgui_click_item", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::OBJECT, "item")));
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scroll_position"), "set_scroll_position", "get_scroll_position");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "scroll_step"), "set_scroll_step", "get_scroll_step");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_item"), "set_default_item", "get_default_item");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "num_items"), "set_num_items", "get_num_items");
}

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
        scroll_velocity = delta; // frame-based velocity
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

void GList::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GComponent::setup_before_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 5)) {
        return;
    }

    layout = static_cast<fgui::ListLayoutType>(p_buffer.read_byte());
    selection_mode = static_cast<fgui::ListSelectionMode>(p_buffer.read_byte());
    p_buffer.read_byte(); // align
    p_buffer.read_byte(); // vertical align
    line_gap = p_buffer.read_short();
    column_gap = p_buffer.read_short();
    line_count = p_buffer.read_short();
    column_count = p_buffer.read_short();
    auto_resize_item = p_buffer.read_bool();
    p_buffer.read_byte(); // children render order
    p_buffer.read_short(); // apex index

    if (p_buffer.read_bool()) {
        p_buffer.skip(16); // margin top/bottom/left/right
    }

    const uint8_t overflow = p_buffer.read_byte();
    set_clip_contents(overflow != 0);
    if (overflow == 2 && p_buffer.seek(p_begin_pos, 7)) {
        setup_scroll(p_buffer);
    }

    if (p_buffer.seek(p_begin_pos, 5)) {
        p_buffer.skip(18);
        if (p_buffer.read_bool()) {
            p_buffer.skip(16);
        }
        p_buffer.read_byte();
        if (p_buffer.read_bool()) {
            p_buffer.skip(8); // clip softness
        }
        if (p_buffer.version >= 2) {
            scroll_item_to_view_on_click = p_buffer.read_bool();
            fold_invisible_items = p_buffer.read_bool();
        }
    }

    if (!p_buffer.seek(p_begin_pos, 8)) {
        declared_item_count = 0;
        default_item = String();
        return;
    }

    default_item = p_buffer.read_s();
    declared_item_count = p_buffer.read_short();
    read_declared_items(p_buffer);
    relayout_items();
}

int32_t GList::get_layout() const {
    return static_cast<int32_t>(layout);
}

int32_t GList::get_selection_mode() const {
    return static_cast<int32_t>(selection_mode);
}

int32_t GList::get_line_gap() const {
    return line_gap;
}

int32_t GList::get_column_gap() const {
    return column_gap;
}

int32_t GList::get_line_count() const {
    return line_count;
}

int32_t GList::get_column_count() const {
    return column_count;
}

bool GList::is_auto_resize_item() const {
    return auto_resize_item;
}

bool GList::is_scroll_item_to_view_on_click() const {
    return scroll_item_to_view_on_click;
}

bool GList::is_fold_invisible_items() const {
    return fold_invisible_items;
}

int32_t GList::get_scroll_type() const {
    return static_cast<int32_t>(scroll_type);
}

int32_t GList::get_scroll_bar_display() const {
    return static_cast<int32_t>(scroll_bar_display);
}

bool GList::is_mouse_wheel_enabled() const {
    return mouse_wheel_enabled;
}

bool GList::is_page_mode() const {
    return page_mode;
}

bool GList::is_snap_to_item() const {
    return snap_to_item;
}

String GList::get_default_item() const {
    return default_item;
}

void GList::set_default_item(const String &p_url) {
    default_item = p_url;
    if (virtual_list) {
        virtual_item_size = Vector2();
        clear_virtual_items();
        for (GObject *item : reuse_pool) {
            if (item->get_parent() == this) remove_child(item);
            memdelete(item);
        }
        reuse_pool.clear();
        relayout_virtual_items();
    }
}

int32_t GList::get_declared_item_count() const {
    return declared_item_count;
}

int32_t GList::get_item_count() const {
    if (virtual_list) {
        return num_items;
    }
    return get_child_count();
}

void GList::set_virtual(bool p_loop) {
    if (virtual_list) {
        virtual_loop = p_loop;
        refresh_virtual_list();
        return;
    }

    virtual_list = true;
    virtual_loop = p_loop;
    clear_selection();
    clear_virtual_items();
    num_items = 0;
    first_visible_index = 0;
    set_clip_contents(true);
    relayout_virtual_items();
}

bool GList::is_virtual() const {
    return virtual_list;
}

bool GList::is_loop_virtual() const {
    return virtual_list && virtual_loop;
}

void GList::set_num_items(int32_t p_count) {
    p_count = MAX(0, p_count);
    if (!virtual_list) {
        return;
    }

    num_items = p_count;
    for (int32_t i = selected_indices.size() - 1; i >= 0; i--) {
        if (selected_indices[i] >= num_items) {
            selected_indices.remove_at(i);
        }
    }
    relayout_virtual_items();
}

int32_t GList::get_num_items() const {
    return virtual_list ? num_items : get_child_count();
}

void GList::set_item_renderer(const Callable &p_renderer) {
    item_renderer = p_renderer;
    if (virtual_list) {
        refresh_virtual_list();
    }
}

Callable GList::get_item_renderer() const {
    return item_renderer;
}

void GList::refresh_virtual_list() {
    if (!virtual_list) {
        return;
    }
    relayout_virtual_items();
}

int32_t GList::get_first_visible_index() const {
    return first_visible_index;
}

int32_t GList::get_virtual_child_count() const {
    return virtual_list ? virtual_items.size() : get_child_count();
}

int32_t GList::get_reuse_pool_count() const {
    return reuse_pool.size();
}

GObject *GList::add_item_from_url(const String &p_url) {
    if (virtual_list) {
        return nullptr;
    }
    if (p_url.is_empty()) {
        return nullptr;
    }

    GObject *item = UIPackage::create_object_from_url(p_url);
    if (item == nullptr) {
        return nullptr;
    }

    add_child(item);
    bind_item(item);
    relayout_items();
    return item;
}

void GList::add_selection(int32_t p_index) {
    if (selection_mode == fgui::ListSelectionMode::None || p_index < 0 || p_index >= get_item_count()) {
        return;
    }

    if (selection_mode == fgui::ListSelectionMode::Single) {
        clear_selection();
    }

    if (virtual_list) {
        if (!is_selected(p_index)) {
            selected_indices.push_back(p_index);
        }
    } else {
        set_item_selected(p_index, true);
    }
    last_selected_index = p_index;
    if (virtual_list) {
        relayout_virtual_items();
    }
}

void GList::remove_selection(int32_t p_index) {
    if (selection_mode == fgui::ListSelectionMode::None || p_index < 0 || p_index >= get_item_count()) {
        return;
    }

    if (virtual_list) {
        for (int32_t i = selected_indices.size() - 1; i >= 0; i--) {
            if (selected_indices[i] == p_index) {
                selected_indices.remove_at(i);
            }
        }
    } else {
        set_item_selected(p_index, false);
    }
    if (last_selected_index == p_index) {
        last_selected_index = get_selected_index();
    }
    if (virtual_list) {
        relayout_virtual_items();
    }
}

void GList::clear_selection() {
    if (virtual_list) {
        selected_indices.clear();
        last_selected_index = -1;
        relayout_virtual_items();
        return;
    }
    for (int32_t i = 0; i < get_child_count(); i++) {
        set_item_selected(i, false);
    }
    last_selected_index = -1;
}

bool GList::is_selected(int32_t p_index) const {
    if (virtual_list) {
        for (int32_t i = 0; i < selected_indices.size(); i++) {
            if (selected_indices[i] == p_index) {
                return true;
            }
        }
        return false;
    }
    const GButton *button = get_button_item(p_index);
    return button != nullptr && button->is_selected();
}

int32_t GList::get_selected_index() const {
    if (virtual_list) {
        return selected_indices.is_empty() ? -1 : selected_indices[0];
    }
    for (int32_t i = 0; i < get_child_count(); i++) {
        if (is_selected(i)) {
            return i;
        }
    }
    return -1;
}

void GList::handle_item_click(GObject *p_item) {
    if (p_item == nullptr) {
        return;
    }

    const int32_t physical_index = p_item->get_index();
    const int32_t index = virtual_list ? get_physical_index_for_virtual_index(physical_index) : physical_index;
    if (index < 0) {
        return;
    }
    GButton *button = Object::cast_to<GButton>(p_item);
    if (button != nullptr && selection_mode != fgui::ListSelectionMode::None) {
        if (selection_mode == fgui::ListSelectionMode::Single) {
            clear_selection();
            if (virtual_list) {
                selected_indices.push_back(index);
            } else {
                button->set_selected(true);
            }
        } else {
            if (virtual_list) {
                if (is_selected(index)) {
                    remove_selection(index);
                } else {
                    selected_indices.push_back(index);
                }
            } else {
                button->set_selected(!button->is_selected());
            }
        }
        last_selected_index = is_selected(index) ? index : get_selected_index();
        if (virtual_list) {
            relayout_virtual_items();
        }
    }

    emit_signal("fgui_click_item", index, p_item);
}

void GList::set_scroll_position(const Vector2 &p_position) {
    scroll_position = clamp_scroll_position(p_position);
    if (virtual_list) {
        relayout_virtual_items();
    } else {
        relayout_items();
    }
}

Vector2 GList::get_scroll_position() const {
    return scroll_position;
}

Vector2 GList::get_content_size() const {
    return content_size;
}

void GList::scroll_to_view(int32_t p_index) {
    if (p_index < 0 || p_index >= get_item_count()) {
        return;
    }

    if (virtual_list) {
        if (layout == fgui::ListLayoutType::SingleRow) {
            set_scroll_position(Vector2(p_index * (virtual_item_size.x + column_gap), scroll_position.y));
        } else {
            set_scroll_position(Vector2(scroll_position.x, p_index * (virtual_item_size.y + line_gap)));
        }
        return;
    }

    const GObject *item = Object::cast_to<GObject>(get_child(p_index));
    if (item == nullptr) {
        return;
    }

    const Vector2 logical_position = item->get_position() + scroll_position;
    const Vector2 item_size = item->get_size();
    Vector2 target = scroll_position;
    const Vector2 viewport = get_size();

    if (logical_position.x < scroll_position.x) {
        target.x = logical_position.x;
    } else if (logical_position.x + item_size.x > scroll_position.x + viewport.x) {
        target.x = logical_position.x + item_size.x - viewport.x;
    }

    if (logical_position.y < scroll_position.y) {
        target.y = logical_position.y;
    } else if (logical_position.y + item_size.y > scroll_position.y + viewport.y) {
        target.y = logical_position.y + item_size.y - viewport.y;
    }

    set_scroll_position(target);
}

void GList::set_scroll_step(float p_step) {
    scroll_step = MAX(0.0f, p_step);
}

float GList::get_scroll_step() const {
    return scroll_step;
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

void GList::read_declared_items(fgui::ByteBuffer &p_buffer) {
    for (int32_t i = 0; i < declared_item_count; i++) {
        const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();

        String item_url = p_buffer.read_s();
        if (item_url.is_empty()) {
            item_url = default_item;
        }

        GObject *item = add_item_from_url(item_url);
        if (item != nullptr) {
            setup_item(p_buffer, item);
        }

        p_buffer.set_position(next_pos);
    }
}

void GList::setup_scroll(fgui::ByteBuffer &p_buffer) {
    scroll_type = static_cast<fgui::ScrollType>(p_buffer.read_byte());
    scroll_bar_display = static_cast<fgui::ScrollBarDisplayType>(p_buffer.read_byte());
    const int32_t flags = p_buffer.read_int();

    if (p_buffer.read_bool()) {
        p_buffer.skip(16); // scroll bar margin top/bottom/left/right
    }

    vt_scroll_bar_res = p_buffer.read_s();
    hz_scroll_bar_res = p_buffer.read_s();
    p_buffer.read_s(); // header resource
    p_buffer.read_s(); // footer resource

    snap_to_item = (flags & 2) != 0;
    page_mode = (flags & 8) != 0;
    scroll_touch_effect = !((flags & 32) != 0);
    if ((flags & 16) != 0) scroll_touch_effect = true;
    scroll_bounce_effect = !((flags & 128) != 0);
    if ((flags & 64) != 0) scroll_bounce_effect = true;
    scroll_inertia_disabled = (flags & 256) != 0;
    mouse_wheel_enabled = scroll_bar_display != fgui::ScrollBarDisplayType::Hidden;
    set_clip_contents(true);
}

void GList::setup_item(fgui::ByteBuffer &p_buffer, GObject *p_item) {
    const String title = p_buffer.read_s();
    if (!title.is_empty()) {
        GTextField *text_field = Object::cast_to<GTextField>(p_item);
        if (text_field != nullptr) {
            text_field->set_text(title);
        }

        GButton *button = Object::cast_to<GButton>(p_item);
        if (button != nullptr) {
            button->set_title(title);
        }
    }

    p_buffer.read_s(); // selected title
    p_buffer.read_s(); // icon
    p_buffer.read_s(); // selected icon
    const String item_name = p_buffer.read_s();
    if (!item_name.is_empty()) {
        p_item->set_name(item_name);
    }

    GComponent *component = Object::cast_to<GComponent>(p_item);
    if (component == nullptr) {
        return;
    }

    int32_t count = p_buffer.read_short();
    for (int32_t i = 0; i < count; i++) {
        p_buffer.read_s(); // controller name
        p_buffer.read_s(); // selected page id
    }

    if (p_buffer.version < 2) {
        return;
    }

    count = p_buffer.read_short();
    for (int32_t i = 0; i < count; i++) {
        p_buffer.read_s(); // target path
        p_buffer.read_short(); // property id
        p_buffer.read_s(); // value
    }
}

void GList::relayout_items() {
    if (virtual_list) {
        relayout_virtual_items();
        return;
    }

    Vector2 cursor;
    real_t row_height = 0.0;
    Vector2 max_extent;
    const real_t view_width = get_size().x;

    for (int32_t i = 0; i < get_child_count(); i++) {
        GObject *item = Object::cast_to<GObject>(get_child(i));
        if (item == nullptr) {
            continue;
        }

        const Vector2 item_size = item->get_size();
        if (layout == fgui::ListLayoutType::SingleRow) {
            item->set_position(cursor - scroll_position);
            max_extent.x = MAX(max_extent.x, cursor.x + item_size.x);
            max_extent.y = MAX(max_extent.y, cursor.y + item_size.y);
            cursor.x += item_size.x + column_gap;
        } else if (layout == fgui::ListLayoutType::FlowHorizontal && view_width > 0.0 && cursor.x > 0.0 && cursor.x + item_size.x > view_width) {
            cursor.x = 0.0;
            cursor.y += row_height + line_gap;
            row_height = 0.0;
            item->set_position(cursor - scroll_position);
            max_extent.x = MAX(max_extent.x, cursor.x + item_size.x);
            max_extent.y = MAX(max_extent.y, cursor.y + item_size.y);
            cursor.x += item_size.x + column_gap;
            row_height = MAX(row_height, item_size.y);
        } else {
            item->set_position(cursor - scroll_position);
            max_extent.x = MAX(max_extent.x, cursor.x + item_size.x);
            max_extent.y = MAX(max_extent.y, cursor.y + item_size.y);
            if (layout == fgui::ListLayoutType::FlowHorizontal) {
                cursor.x += item_size.x + column_gap;
                row_height = MAX(row_height, item_size.y);
            } else {
                cursor.y += item_size.y + line_gap;
            }
        }
    }

    content_size = max_extent;
    for (int32_t iter = 0; iter < 3; iter++) {
        const Vector2 clamped = clamp_scroll_position(scroll_position);
        if (clamped == scroll_position) break;
        scroll_position = clamped;
        // Re-check positions only if clamped; single pass is sufficient here
    }
}

void GList::relayout_virtual_items() {
    if (!virtual_list) {
        return;
    }

    if (num_items <= 0 || default_item.is_empty()) {
        clear_virtual_items();
        first_visible_index = 0;
        content_size = Vector2();
        scroll_position = Vector2();
        return;
    }

    if (virtual_item_size == Vector2()) {
        GObject *sample = get_virtual_item_from_pool();
        if (sample != nullptr) {
            update_virtual_item_size(sample);
            return_virtual_item_to_pool(sample);
        }
    }

    if (virtual_item_size == Vector2()) {
        virtual_item_size = Vector2(1, 1);
    }

    const bool horizontal = layout == fgui::ListLayoutType::SingleRow;
    const real_t item_extent = horizontal ? virtual_item_size.x + column_gap : virtual_item_size.y + line_gap;
    const real_t viewport_extent = horizontal ? get_size().x : get_size().y;
    const real_t scroll_extent = horizontal ? scroll_position.x : scroll_position.y;
    const real_t total_extent = item_extent * num_items - (horizontal ? column_gap : line_gap);
    content_size = horizontal ? Vector2(MAX(total_extent, virtual_item_size.x), virtual_item_size.y) : Vector2(virtual_item_size.x, MAX(total_extent, virtual_item_size.y));
    scroll_position = clamp_scroll_position(scroll_position);

    first_visible_index = item_extent > 0.0 ? CLAMP(static_cast<int32_t>(Math::floor(scroll_extent / item_extent)), 0, num_items - 1) : 0;
    const int32_t visible_capacity = MAX(1, static_cast<int32_t>(Math::ceil(viewport_extent / item_extent)) + 2);
    const int32_t last_visible_index = MIN(num_items - 1, first_visible_index + visible_capacity - 1);

    for (int32_t i = virtual_items.size() - 1; i >= 0; i--) {
        const int32_t logical_index = virtual_item_indices[i];
        if (logical_index < first_visible_index || logical_index > last_visible_index) {
            GObject *item = virtual_items[i];
            virtual_items.remove_at(i);
            virtual_item_indices.remove_at(i);
            return_virtual_item_to_pool(item);
        }
    }

    for (int32_t logical_index = first_visible_index; logical_index <= last_visible_index; logical_index++) {
        bool exists = false;
        for (int32_t i = 0; i < virtual_item_indices.size(); i++) {
            if (virtual_item_indices[i] == logical_index) {
                exists = true;
                break;
            }
        }
        if (exists) {
            continue;
        }

        GObject *item = get_virtual_item_from_pool();
        if (item == nullptr) {
            continue;
        }
        if (item->get_parent() == nullptr) {
            add_child(item);
        }
        bind_item(item);
        virtual_items.push_back(item);
        virtual_item_indices.push_back(logical_index);
    }

    for (int32_t i = 0; i < virtual_items.size(); i++) {
        GObject *item = virtual_items[i];
        const int32_t logical_index = virtual_item_indices[i];
        const Vector2 logical_position = horizontal ? Vector2(logical_index * item_extent, 0) : Vector2(0, logical_index * item_extent);
        item->set_position(logical_position - scroll_position);
        item->set_size(virtual_item_size);
        set_item_selected(i, is_selected(logical_index));
        render_virtual_item(logical_index, item);
    }
}

GObject *GList::get_virtual_item_from_pool() {
    GObject *item = nullptr;
    if (!reuse_pool.is_empty()) {
        item = reuse_pool[reuse_pool.size() - 1];
        reuse_pool.remove_at(reuse_pool.size() - 1);
    } else {
        item = UIPackage::create_object_from_url(default_item);
    }
    if (item != nullptr) {
        item->set_visible(true);
    }
    return item;
}

void GList::return_virtual_item_to_pool(GObject *p_item) {
    if (p_item == nullptr) {
        return;
    }
    if (p_item->get_parent() == this) {
        remove_child(p_item);
    }
    p_item->set_visible(false);
    reuse_pool.push_back(p_item);
}

void GList::clear_virtual_items() {
    for (int32_t i = virtual_items.size() - 1; i >= 0; i--) {
        return_virtual_item_to_pool(virtual_items[i]);
    }
    virtual_items.clear();
    virtual_item_indices.clear();
}

void GList::update_virtual_item_size(GObject *p_item) {
    if (p_item == nullptr) {
        return;
    }
    Vector2 size = p_item->get_size();
    if (size == Vector2()) {
        size = p_item->get_custom_minimum_size();
    }
    if (size == Vector2()) {
        size = Vector2(1, 1);
    }
    virtual_item_size = size;
    if (scroll_step == 40.0f) {
        scroll_step = layout == fgui::ListLayoutType::SingleRow ? virtual_item_size.x + column_gap : virtual_item_size.y + line_gap;
    }
}

void GList::render_virtual_item(int32_t p_index, GObject *p_item) {
    if (!item_renderer.is_valid()) {
        return;
    }
    Array args;
    args.push_back(p_index);
    args.push_back(p_item);
    item_renderer.callv(args);
}

int32_t GList::get_physical_index_for_virtual_index(int32_t p_index) const {
    if (!virtual_list) {
        return p_index;
    }
    for (int32_t i = 0; i < virtual_item_indices.size(); i++) {
        GObject *item = virtual_items[i];
        if (item != nullptr && item->get_index() == p_index) {
            return virtual_item_indices[i];
        }
    }
    return -1;
}

Vector2 GList::clamp_scroll_position(const Vector2 &p_position) const {
    const Vector2 viewport = get_size();
    const real_t max_x = MAX(static_cast<real_t>(0.0), content_size.x - viewport.x);
    const real_t max_y = MAX(static_cast<real_t>(0.0), content_size.y - viewport.y);
    return Vector2(CLAMP(p_position.x, static_cast<real_t>(0.0), max_x), CLAMP(p_position.y, static_cast<real_t>(0.0), max_y));
}

void GList::bind_item(GObject *p_item) {
    if (p_item == nullptr) return;
    Callable bound = Callable(this, "handle_item_click").bind(p_item);
    if (p_item->is_connected("fgui_click", bound)) return;
    p_item->connect("fgui_click", bound);
}

void GList::set_item_selected(int32_t p_index, bool p_selected) {
    GButton *button = get_mutable_button_item(p_index);
    if (button != nullptr) {
        button->set_selected(p_selected);
    }
}

const GButton *GList::get_button_item(int32_t p_index) const {
    if (p_index < 0 || p_index >= get_child_count()) {
        return nullptr;
    }
    return Object::cast_to<GButton>(get_child(p_index));
}

GButton *GList::get_mutable_button_item(int32_t p_index) {
    if (p_index < 0 || p_index >= get_child_count()) {
        return nullptr;
    }
    return Object::cast_to<GButton>(get_child(p_index));
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
        // Bounce if velocity near zero but position differs due to clamping
        if (scroll_velocity.length() < 0.1 && new_pos != scroll_position) {
            scroll_velocity = Vector2();
        }
    }

    if (new_pos != scroll_position || has_motion) {
        scroll_position = new_pos;
        if (virtual_list) {
            relayout_virtual_items();
        } else {
            relayout_items();
        }
    } else {
        set_process(false);
    }
}
