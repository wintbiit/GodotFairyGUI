#include "g_text_input.h"

#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

void GTextInput::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_text", "text"), &GTextInput::set_text);
    ClassDB::bind_method(D_METHOD("get_text"), &GTextInput::get_text);
    ClassDB::bind_method(D_METHOD("set_editable", "editable"), &GTextInput::set_editable);
    ClassDB::bind_method(D_METHOD("is_editable"), &GTextInput::is_editable);
    ClassDB::bind_method(D_METHOD("set_prompt_text", "prompt_text"), &GTextInput::set_prompt_text);
    ClassDB::bind_method(D_METHOD("get_prompt_text"), &GTextInput::get_prompt_text);
    ClassDB::bind_method(D_METHOD("set_restrict", "restrict"), &GTextInput::set_restrict);
    ClassDB::bind_method(D_METHOD("get_restrict"), &GTextInput::get_restrict);
    ClassDB::bind_method(D_METHOD("set_max_length", "max_length"), &GTextInput::set_max_length);
    ClassDB::bind_method(D_METHOD("get_max_length"), &GTextInput::get_max_length);
    ClassDB::bind_method(D_METHOD("set_keyboard_type", "keyboard_type"), &GTextInput::set_keyboard_type);
    ClassDB::bind_method(D_METHOD("get_keyboard_type"), &GTextInput::get_keyboard_type);
    ClassDB::bind_method(D_METHOD("set_display_as_password", "display_as_password"), &GTextInput::set_display_as_password);
    ClassDB::bind_method(D_METHOD("is_display_as_password"), &GTextInput::is_display_as_password);
    ClassDB::bind_method(D_METHOD("set_multiline", "multiline"), &GTextInput::set_multiline);
    ClassDB::bind_method(D_METHOD("is_multiline"), &GTextInput::is_multiline);
    ClassDB::bind_method(D_METHOD("set_hide_input", "hide_input"), &GTextInput::set_hide_input);
    ClassDB::bind_method(D_METHOD("is_hide_input"), &GTextInput::is_hide_input);
    ClassDB::bind_method(D_METHOD("set_keyboard_input", "keyboard_input"), &GTextInput::set_keyboard_input);
    ClassDB::bind_method(D_METHOD("is_keyboard_input"), &GTextInput::is_keyboard_input);
    ClassDB::bind_method(D_METHOD("set_disable_ime", "disable_ime"), &GTextInput::set_disable_ime);
    ClassDB::bind_method(D_METHOD("is_disable_ime"), &GTextInput::is_disable_ime);
    ClassDB::bind_method(D_METHOD("set_mouse_wheel_enabled", "mouse_wheel_enabled"), &GTextInput::set_mouse_wheel_enabled);
    ClassDB::bind_method(D_METHOD("is_mouse_wheel_enabled"), &GTextInput::is_mouse_wheel_enabled);
    ClassDB::bind_method(D_METHOD("set_caret_position", "position"), &GTextInput::set_caret_position);
    ClassDB::bind_method(D_METHOD("get_caret_position"), &GTextInput::get_caret_position);
    ClassDB::bind_method(D_METHOD("set_selection", "start", "length"), &GTextInput::set_selection);
    ClassDB::bind_method(D_METHOD("replace_selection", "value"), &GTextInput::replace_selection);
    ClassDB::bind_method(D_METHOD("has_native_input"), &GTextInput::has_native_input);
    ClassDB::bind_method(D_METHOD("get_native_input_owner_instance_id"), &GTextInput::get_native_input_owner_instance_id);
    ClassDB::bind_method(D_METHOD("_handle_line_text_changed", "value"), &GTextInput::_handle_line_text_changed);
    ClassDB::bind_method(D_METHOD("_handle_text_edit_changed"), &GTextInput::_handle_text_edit_changed);
    ClassDB::bind_method(D_METHOD("_handle_line_text_submitted", "value"), &GTextInput::_handle_line_text_submitted);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editable"), "set_editable", "is_editable");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "prompt_text"), "set_prompt_text", "get_prompt_text");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "restrict"), "set_restrict", "get_restrict");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_length"), "set_max_length", "get_max_length");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "keyboard_type"), "set_keyboard_type", "get_keyboard_type");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display_as_password"), "set_display_as_password", "is_display_as_password");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "multiline"), "set_multiline", "is_multiline");
    ADD_SIGNAL(MethodInfo("fgui_changed", PropertyInfo(Variant::STRING, "text")));
    ADD_SIGNAL(MethodInfo("fgui_submit", PropertyInfo(Variant::STRING, "text")));
}

void GTextInput::_draw() {
}

void GTextInput::_notification(int p_what) {
    GTextField::_notification(p_what);
    if (p_what == NOTIFICATION_ENTER_TREE || p_what == NOTIFICATION_READY || p_what == NOTIFICATION_RESIZED) {
        update_native_input();
    }
}

void GTextInput::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GTextField::setup_before_add(p_buffer, p_begin_pos);
    set_multiline(!single_line);

    if (!p_buffer.seek(p_begin_pos, 4)) {
        update_native_input();
        return;
    }

    const String read_prompt = p_buffer.read_s();
    if (!read_prompt.is_empty()) {
        set_prompt_text(read_prompt);
    }

    const String read_restrict = p_buffer.read_s();
    if (!read_restrict.is_empty()) {
        set_restrict(read_restrict);
    }

    const int32_t read_max_length = p_buffer.read_int();
    if (read_max_length != 0) {
        set_max_length(read_max_length);
    }

    const int32_t read_keyboard_type = p_buffer.read_int();
    if (read_keyboard_type != 0) {
        set_keyboard_type(read_keyboard_type);
    }

    if (p_buffer.read_bool()) {
        set_display_as_password(true);
    }
    update_native_input();
}

void GTextInput::set_text(const String &p_text) {
    const String filtered = apply_restrict_and_length(p_text);
    GTextField::set_text(filtered);
    sync_native_text();
}

String GTextInput::get_text() const {
    if (line_edit != nullptr) {
        return line_edit->get_text();
    }
    if (text_edit != nullptr) {
        return text_edit->get_text();
    }
    return GTextField::get_text();
}

void GTextInput::set_text_color(const Color &p_color) {
    GTextField::set_text_color(p_color);
    update_native_input();
}

void GTextInput::set_font_size(int32_t p_size) {
    GTextField::set_font_size(p_size);
    update_native_input();
}

void GTextInput::set_editable(bool p_editable) {
    editable = p_editable;
    update_native_input();
}

bool GTextInput::is_editable() const {
    return editable;
}

void GTextInput::set_prompt_text(const String &p_prompt_text) {
    prompt_text = p_prompt_text;
    update_native_input();
}

String GTextInput::get_prompt_text() const {
    return prompt_text;
}

void GTextInput::set_restrict(const String &p_restrict) {
    restrict = p_restrict;
    set_text(get_text());
}

String GTextInput::get_restrict() const {
    return restrict;
}

void GTextInput::set_max_length(int32_t p_max_length) {
    max_length = MAX(0, p_max_length);
    set_text(get_text());
    update_native_input();
}

int32_t GTextInput::get_max_length() const {
    return max_length;
}

void GTextInput::set_keyboard_type(int32_t p_keyboard_type) {
    keyboard_type = CLAMP(p_keyboard_type, 0, 7);
    update_native_input();
}

int32_t GTextInput::get_keyboard_type() const {
    return keyboard_type;
}

void GTextInput::set_display_as_password(bool p_display_as_password) {
    display_as_password = p_display_as_password;
    update_native_input();
}

bool GTextInput::is_display_as_password() const {
    return display_as_password;
}

void GTextInput::set_multiline(bool p_multiline) {
    if (multiline == p_multiline) {
        return;
    }
    multiline = p_multiline;
    clear_native_input();
    update_native_input();
}

bool GTextInput::is_multiline() const {
    return multiline;
}

void GTextInput::set_hide_input(bool p_hide_input) { hide_input = p_hide_input; }
bool GTextInput::is_hide_input() const { return hide_input; }

void GTextInput::set_keyboard_input(bool p_keyboard_input) { keyboard_input = p_keyboard_input; }
bool GTextInput::is_keyboard_input() const { return keyboard_input; }

void GTextInput::set_disable_ime(bool p_disable_ime) { disable_ime = p_disable_ime; }
bool GTextInput::is_disable_ime() const { return disable_ime; }

void GTextInput::set_mouse_wheel_enabled(bool p_mouse_wheel_enabled) { text_input_mouse_wheel = p_mouse_wheel_enabled; }
bool GTextInput::is_mouse_wheel_enabled() const { return text_input_mouse_wheel; }

void GTextInput::set_caret_position(int32_t p_position) {
    update_native_input();
    if (line_edit != nullptr) {
        line_edit->set_caret_column(CLAMP(p_position, 0, line_edit->get_text().length()));
    } else if (text_edit != nullptr) {
        text_edit->set_caret_column(MAX(0, p_position));
    }
}

int32_t GTextInput::get_caret_position() const {
    if (line_edit != nullptr) {
        return line_edit->get_caret_column();
    }
    if (text_edit != nullptr) {
        return text_edit->get_caret_column();
    }
    return 0;
}

void GTextInput::set_selection(int32_t p_start, int32_t p_length) {
    selection_start = MAX(0, p_start);
    selection_length = MAX(0, p_length);
    update_native_input();
    if (line_edit != nullptr) {
        line_edit->select(selection_start, selection_start + selection_length);
    }
}

void GTextInput::replace_selection(const String &p_value) {
    update_native_input();
    if (selection_length > 0) {
        const String current = get_text();
        const int32_t from = CLAMP(selection_start, 0, current.length());
        const int32_t to = CLAMP(selection_start + selection_length, from, current.length());
        set_text(current.substr(0, from) + p_value + current.substr(to));
        set_caret_position(from + p_value.length());
        selection_start = 0;
        selection_length = 0;
        emit_signal("fgui_changed", get_text());
        return;
    }

    if (line_edit != nullptr) {
        line_edit->insert_text_at_caret(p_value);
    } else if (text_edit != nullptr) {
        text_edit->insert_text_at_caret(p_value);
    }
}

bool GTextInput::has_native_input() {
    update_native_input();
    return line_edit != nullptr || text_edit != nullptr;
}

int64_t GTextInput::get_native_input_owner_instance_id() {
    update_native_input();
    Control *native = get_native_control();
    return native != nullptr && native->get_owner() != nullptr ? native->get_owner()->get_instance_id() : 0;
}

void GTextInput::update_native_input() {
    if (multiline) {
        if (text_edit == nullptr) {
            clear_native_input();
            text_edit = memnew(TextEdit);
            text_edit->set_name("_TextEdit");
            text_edit->set_context_menu_enabled(true);
            add_child(text_edit);
            const Callable callback(this, StringName("_handle_text_edit_changed"));
            if (!text_edit->is_connected(StringName("text_changed"), callback)) {
                text_edit->connect(StringName("text_changed"), callback);
            }
        }
        text_edit->set_position(Vector2());
        text_edit->set_size(get_size());
        text_edit->set_editable(editable);
        text_edit->set_placeholder(prompt_text);
        text_edit->set_virtual_keyboard_enabled(true);
        text_edit->set_virtual_keyboard_show_on_focus(true);
        if (text_edit->get_text() != text) {
            text_edit->set_text(text);
        }
    } else {
        if (line_edit == nullptr) {
            clear_native_input();
            line_edit = memnew(LineEdit);
            line_edit->set_name("_LineEdit");
            line_edit->set_flat(true);
            line_edit->set_context_menu_enabled(true);
            add_child(line_edit);
            const Callable changed_callback(this, StringName("_handle_line_text_changed"));
            if (!line_edit->is_connected(StringName("text_changed"), changed_callback)) {
                line_edit->connect(StringName("text_changed"), changed_callback);
            }
            const Callable submitted_callback(this, StringName("_handle_line_text_submitted"));
            if (!line_edit->is_connected(StringName("text_submitted"), submitted_callback)) {
                line_edit->connect(StringName("text_submitted"), submitted_callback);
            }
        }
        line_edit->set_position(Vector2());
        line_edit->set_size(get_size());
        line_edit->set_editable(editable);
        line_edit->set_placeholder(prompt_text);
        line_edit->set_max_length(max_length);
        line_edit->set_secret(display_as_password);
        line_edit->set_virtual_keyboard_enabled(true);
        line_edit->set_virtual_keyboard_show_on_focus(true);
        line_edit->set_virtual_keyboard_type(static_cast<LineEdit::VirtualKeyboardType>(keyboard_type));
        line_edit->set_horizontal_alignment(horizontal_alignment);
        if (line_edit->get_text() != text) {
            line_edit->set_text(text);
        }
    }
}

void GTextInput::clear_native_input() {
    if (line_edit != nullptr) {
        remove_child(line_edit);
        memdelete(line_edit);
        line_edit = nullptr;
    }
    if (text_edit != nullptr) {
        remove_child(text_edit);
        memdelete(text_edit);
        text_edit = nullptr;
    }
}

Control *GTextInput::get_native_control() const {
    if (line_edit != nullptr) {
        return line_edit;
    }
    return text_edit;
}

String GTextInput::apply_restrict_and_length(const String &p_value) const {
    String result;
    for (int64_t i = 0; i < p_value.length(); i++) {
        const char32_t c = p_value[i];
        if (is_restrict_allowed(c)) {
            result += String::chr(c);
        }
        if (max_length > 0 && result.length() >= max_length) {
            break;
        }
    }
    return result;
}

bool GTextInput::is_restrict_allowed(char32_t p_char) const {
    if (restrict.is_empty()) {
        return true;
    }

    bool negate = false;
    int64_t begin = 0;
    if (restrict[0] == '^') {
        negate = true;
        begin = 1;
    }

    bool matched = false;
    for (int64_t i = begin; i < restrict.length(); i++) {
        if (i + 2 < restrict.length() && restrict[i + 1] == '-') {
            const char32_t from = restrict[i];
            const char32_t to = restrict[i + 2];
            if (p_char >= MIN(from, to) && p_char <= MAX(from, to)) {
                matched = true;
                break;
            }
            i += 2;
            continue;
        }
        if (restrict[i] == p_char) {
            matched = true;
            break;
        }
    }
    return negate ? !matched : matched;
}

void GTextInput::sync_native_text() {
    update_native_input();
    syncing_native = true;
    if (line_edit != nullptr && line_edit->get_text() != text) {
        line_edit->set_text(text);
    }
    if (text_edit != nullptr && text_edit->get_text() != text) {
        text_edit->set_text(text);
    }
    syncing_native = false;
}

void GTextInput::_handle_line_text_changed(const String &p_value) {
    if (syncing_native) {
        return;
    }
    const String filtered = apply_restrict_and_length(p_value);
    if (filtered != p_value && line_edit != nullptr) {
        syncing_native = true;
        line_edit->set_text(filtered);
        line_edit->set_caret_column(MIN(line_edit->get_caret_column(), filtered.length()));
        syncing_native = false;
    }
    GTextField::set_text(filtered);
    emit_signal("fgui_changed", filtered);
}

void GTextInput::_handle_text_edit_changed() {
    if (syncing_native || text_edit == nullptr) {
        return;
    }
    const String filtered = apply_restrict_and_length(text_edit->get_text());
    if (filtered != text_edit->get_text()) {
        syncing_native = true;
        text_edit->set_text(filtered);
        syncing_native = false;
    }
    GTextField::set_text(filtered);
    emit_signal("fgui_changed", filtered);
}

void GTextInput::_handle_line_text_submitted(const String &p_value) {
    emit_signal("fgui_submit", p_value);
}
