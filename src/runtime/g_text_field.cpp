#include "g_text_field.h"

#include "ui_package.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

namespace {
String strip_edges_ascii(const String &p_value) {
    int64_t begin = 0;
    int64_t end = p_value.length();
    while (begin < end) {
        const char32_t c = p_value[begin];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        begin++;
    }
    while (end > begin) {
        const char32_t c = p_value[end - 1];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        end--;
    }
    return p_value.substr(begin, end - begin);
}

bool is_basic_ubb_tag(const String &p_tag) {
    return p_tag == "b" || p_tag == "i" || p_tag == "u" || p_tag == "strike" || p_tag == "sup" || p_tag == "sub" ||
            p_tag == "color" || p_tag == "font" || p_tag == "size" || p_tag == "align" || p_tag == "url";
}

String parse_basic_ubb(const String &p_text, int32_t &r_image_count, int32_t &r_link_count) {
    r_image_count = 0;
    r_link_count = 0;

    String result;
    int64_t pos = 0;
    while (pos < p_text.length()) {
        const int64_t open = p_text.find("[", pos);
        if (open < 0) {
            result += p_text.substr(pos);
            break;
        }

        if (open > 0 && p_text[open - 1] == '\\') {
            result += p_text.substr(pos, open - pos - 1);
            result += "[";
            pos = open + 1;
            continue;
        }

        result += p_text.substr(pos, open - pos);
        const int64_t close = p_text.find("]", open + 1);
        if (close < 0) {
            result += p_text.substr(open);
            break;
        }
        if (close == open + 1) {
            result += "[]";
            pos = close + 1;
            continue;
        }

        String tag_body = p_text.substr(open + 1, close - open - 1);
        const bool end_tag = tag_body.begins_with("/");
        if (end_tag) {
            tag_body = tag_body.substr(1);
        }

        const int64_t equal = tag_body.find("=");
        String tag = equal >= 0 ? tag_body.substr(0, equal) : tag_body;
        tag = tag.to_lower();

        if (tag == "img" && !end_tag) {
            const int64_t end_open = p_text.find("[/img]", close + 1);
            if (end_open >= 0) {
                const String src = strip_edges_ascii(p_text.substr(close + 1, end_open - close - 1));
                if (!src.is_empty()) {
                    result += src;
                    r_image_count++;
                }
                pos = end_open + 6;
                continue;
            }
        }

        if (tag == "url") {
            if (!end_tag) {
                r_link_count++;
            }
            pos = close + 1;
            continue;
        }

        if (is_basic_ubb_tag(tag)) {
            pos = close + 1;
            continue;
        }

        result += p_text.substr(open, close - open + 1);
        pos = close + 1;
    }

    return result;
}

String escape_bbcode_text(const String &p_text) {
    String result;
    for (int64_t i = 0; i < p_text.length(); i++) {
        const char32_t c = p_text[i];
        if (c == '[') {
            result += "[lb]";
        } else if (c == ']') {
            result += "[rb]";
        } else {
            result += String::chr(c);
        }
    }
    return result;
}

String convert_ubb_to_bbcode(const String &p_text, int32_t &r_image_count, int32_t &r_link_count) {
    r_image_count = 0;
    r_link_count = 0;

    String result;
    int64_t pos = 0;
    while (pos < p_text.length()) {
        const int64_t open = p_text.find("[", pos);
        if (open < 0) {
            result += escape_bbcode_text(p_text.substr(pos));
            break;
        }

        if (open > pos) {
            result += escape_bbcode_text(p_text.substr(pos, open - pos));
        }

        const int64_t close = p_text.find("]", open + 1);
        if (close < 0) {
            result += escape_bbcode_text(p_text.substr(open));
            break;
        }

        String tag_body = p_text.substr(open + 1, close - open - 1);
        const bool end_tag = tag_body.begins_with("/");
        if (end_tag) {
            tag_body = tag_body.substr(1);
        }

        const int64_t equal = tag_body.find("=");
        String tag = equal >= 0 ? tag_body.substr(0, equal) : tag_body;
        const String value = equal >= 0 ? tag_body.substr(equal + 1) : String();
        tag = tag.to_lower();

        if (tag == "img" && !end_tag) {
            const int64_t end_open = p_text.find("[/img]", close + 1);
            if (end_open >= 0) {
                const String src = strip_edges_ascii(p_text.substr(close + 1, end_open - close - 1));
                if (!src.is_empty()) {
                    if (src.begins_with("ui://")) {
                        const String resolved = UIPackage::resolve_asset_path_for_image_ubb(src);
                        if (!resolved.is_empty()) {
                            result += "[img]" + resolved + "[/img]";
                        }
                    } else {
                        result += "[img]" + src + "[/img]";
                    }
                    r_image_count++;
                }
                pos = end_open + 6;
                continue;
            }
        }

        if (tag == "strike") {
            result += end_tag ? "[/s]" : "[s]";
            pos = close + 1;
            continue;
        }
        if (tag == "url") {
            if (!end_tag) {
                r_link_count++;
            }
            result += end_tag ? "[/url]" : (value.is_empty() ? "[url]" : "[url=" + value + "]");
            pos = close + 1;
            continue;
        }
        if (tag == "b" || tag == "i" || tag == "u" || tag == "color" || tag == "size" || tag == "font" || tag == "align") {
            if (end_tag) {
                result += "[/" + tag + "]";
            } else {
                result += value.is_empty() ? "[" + tag + "]" : "[" + tag + "=" + value + "]";
            }
            pos = close + 1;
            continue;
        }

        result += escape_bbcode_text(p_text.substr(open, close - open + 1));
        pos = close + 1;
    }

    return result;
}
} // namespace

void GTextField::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_text", "text"), &GTextField::set_text);
    ClassDB::bind_method(D_METHOD("get_text"), &GTextField::get_text);
    ClassDB::bind_method(D_METHOD("get_display_text"), &GTextField::get_display_text);
    ClassDB::bind_method(D_METHOD("set_text_color", "color"), &GTextField::set_text_color);
    ClassDB::bind_method(D_METHOD("get_text_color"), &GTextField::get_text_color);
    ClassDB::bind_method(D_METHOD("set_font_size", "size"), &GTextField::set_font_size);
    ClassDB::bind_method(D_METHOD("get_font_size"), &GTextField::get_font_size);
    ClassDB::bind_method(D_METHOD("set_font_name", "font_name"), &GTextField::set_font_name);
    ClassDB::bind_method(D_METHOD("get_font_name"), &GTextField::get_font_name);
    ClassDB::bind_method(D_METHOD("set_ubb_enabled", "enabled"), &GTextField::set_ubb_enabled);
    ClassDB::bind_method(D_METHOD("is_ubb_enabled"), &GTextField::is_ubb_enabled);
    ClassDB::bind_method(D_METHOD("get_ubb_image_count"), &GTextField::get_ubb_image_count);
    ClassDB::bind_method(D_METHOD("get_ubb_link_count"), &GTextField::get_ubb_link_count);
    ClassDB::bind_method(D_METHOD("set_rich_text_enabled", "enabled"), &GTextField::set_rich_text_enabled);
    ClassDB::bind_method(D_METHOD("is_rich_text_enabled"), &GTextField::is_rich_text_enabled);
    ClassDB::bind_method(D_METHOD("get_rich_text_bbcode"), &GTextField::get_rich_text_bbcode);
    ClassDB::bind_method(D_METHOD("has_rich_text_label"), &GTextField::has_rich_text_label);
    ClassDB::bind_method(D_METHOD("get_rich_text_owner_instance_id"), &GTextField::get_rich_text_owner_instance_id);
    ClassDB::bind_method(D_METHOD("_handle_rich_text_meta_clicked", "meta"), &GTextField::_handle_rich_text_meta_clicked);
    ClassDB::bind_method(D_METHOD("set_align", "align"), &GTextField::set_align);
    ClassDB::bind_method(D_METHOD("get_align"), &GTextField::get_align);
    ClassDB::bind_method(D_METHOD("set_vertical_align", "vertical_align"), &GTextField::set_vertical_align);
    ClassDB::bind_method(D_METHOD("get_vertical_align"), &GTextField::get_vertical_align);
    ClassDB::bind_method(D_METHOD("set_single_line", "single_line"), &GTextField::set_single_line);
    ClassDB::bind_method(D_METHOD("is_single_line"), &GTextField::is_single_line);
    ClassDB::bind_method(D_METHOD("set_auto_size", "auto_size"), &GTextField::set_auto_size);
    ClassDB::bind_method(D_METHOD("get_auto_size"), &GTextField::get_auto_size);
    ClassDB::bind_method(D_METHOD("get_text_width"), &GTextField::get_text_width);
    ClassDB::bind_method(D_METHOD("get_text_height"), &GTextField::get_text_height);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "text_color"), "set_text_color", "get_text_color");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size"), "set_font_size", "get_font_size");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "font_name"), "set_font_name", "get_font_name");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ubb_enabled"), "set_ubb_enabled", "is_ubb_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rich_text_enabled"), "set_rich_text_enabled", "is_rich_text_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "align"), "set_align", "get_align");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_align"), "set_vertical_align", "get_vertical_align");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "single_line"), "set_single_line", "is_single_line");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "auto_size"), "set_auto_size", "get_auto_size");
    ADD_SIGNAL(MethodInfo("fgui_link_click", PropertyInfo(Variant::STRING, "href")));
}

void GTextField::_draw() {
    if (rich_text_enabled) {
        return;
    }

    if (draw_bitmap_font()) {
        return;
    }

    if (draw_true_type_font()) {
        return;
    }

    Ref<Font> font = get_theme_default_font();
    if (font.is_null() || display_text.is_empty()) {
        return;
    }

    const int resolved_font_size = font_size > 0 ? font_size : get_theme_default_font_size();
    font->draw_string(get_canvas_item(), Vector2(0, resolved_font_size), display_text, horizontal_alignment, get_size().x, resolved_font_size, text_color);
}

void GTextField::_notification(int p_what) {
    if (p_what == NOTIFICATION_RESIZED) {
        update_rich_text_label();
    }
}

void GTextField::setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GObject::setup_before_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 5)) {
        return;
    }

    font_name = p_buffer.read_s();
    font_size = p_buffer.read_short();
    text_color = p_buffer.read_color();
    const uint8_t align = p_buffer.read_byte();
    horizontal_alignment = align == 1 ? HORIZONTAL_ALIGNMENT_CENTER : (align == 2 ? HORIZONTAL_ALIGNMENT_RIGHT : HORIZONTAL_ALIGNMENT_LEFT);
    p_buffer.read_byte(); // vertical align
    p_buffer.read_short(); // line spacing
    p_buffer.read_short(); // letter spacing
    ubb_enabled = p_buffer.read_bool();
    p_buffer.read_byte(); // auto size
    p_buffer.read_bool(); // underline
    p_buffer.read_bool(); // italic
    p_buffer.read_bool(); // bold
    single_line = p_buffer.read_bool();
    if (p_buffer.read_bool()) {
        p_buffer.read_color();
        p_buffer.read_float();
    }
    if (p_buffer.read_bool()) {
        p_buffer.read_color();
        p_buffer.read_float();
        p_buffer.read_float();
    }
    p_buffer.read_bool(); // template vars
    if (p_buffer.version >= 3) {
        p_buffer.read_bool(); // strikethrough
        p_buffer.skip(12);
    }

    refresh_display_text();
}

void GTextField::setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) {
    GObject::setup_after_add(p_buffer, p_begin_pos);

    if (!p_buffer.seek(p_begin_pos, 6)) {
        return;
    }

    const String value = p_buffer.read_s();
    if (!value.is_empty()) {
        set_text(value);
    }
}

void GTextField::set_text(const String &p_text) {
    text = p_text;
    refresh_display_text();
}

String GTextField::get_text() const {
    return text;
}

String GTextField::get_display_text() const {
    return display_text;
}

void GTextField::set_text_color(const Color &p_color) {
    text_color = p_color;
    update_rich_text_label();
    queue_redraw();
}

Color GTextField::get_text_color() const {
    return text_color;
}

void GTextField::set_font_size(int32_t p_size) {
    font_size = p_size;
    update_rich_text_label();
    queue_redraw();
}

int32_t GTextField::get_font_size() const {
    return font_size;
}

void GTextField::set_font_name(const String &p_font_name) {
    font_name = p_font_name;
    update_rich_text_label();
    queue_redraw();
}

String GTextField::get_font_name() const {
    return font_name;
}

void GTextField::set_ubb_enabled(bool p_enabled) {
    if (ubb_enabled == p_enabled) {
        return;
    }
    ubb_enabled = p_enabled;
    refresh_display_text();
}

bool GTextField::is_ubb_enabled() const {
    return ubb_enabled;
}

int32_t GTextField::get_ubb_image_count() const {
    return ubb_image_count;
}

int32_t GTextField::get_ubb_link_count() const {
    return ubb_link_count;
}

void GTextField::set_rich_text_enabled(bool p_enabled) {
    if (rich_text_enabled == p_enabled) {
        return;
    }
    rich_text_enabled = p_enabled;
    if (!rich_text_enabled) {
        clear_rich_text_label();
    }
    refresh_display_text();
}

bool GTextField::is_rich_text_enabled() const {
    return rich_text_enabled;
}

String GTextField::get_rich_text_bbcode() const {
    return rich_text_bbcode;
}

bool GTextField::has_rich_text_label() const {
    return rich_text_label != nullptr;
}

int32_t GTextField::get_rich_text_owner_instance_id() const {
    return rich_text_label != nullptr && rich_text_label->get_owner() != nullptr ? static_cast<int32_t>(rich_text_label->get_owner()->get_instance_id()) : 0;
}

void GTextField::refresh_display_text() {
    if (rich_text_enabled) {
        rich_text_bbcode = ubb_enabled ? convert_ubb_to_bbcode(text, ubb_image_count, ubb_link_count) : escape_bbcode_text(text);
        display_text = parse_basic_ubb(text, ubb_image_count, ubb_link_count);
        update_rich_text_label();
    } else if (ubb_enabled) {
        display_text = parse_basic_ubb(text, ubb_image_count, ubb_link_count);
        rich_text_bbcode = String();
    } else {
        display_text = text;
        ubb_image_count = 0;
        ubb_link_count = 0;
        rich_text_bbcode = String();
    }
    queue_redraw();
}

bool GTextField::draw_bitmap_font() {
    if (font_name.is_empty() || display_text.is_empty()) {
        return false;
    }

    fgui::BitmapFontData bitmap_font;
    String package_id;
    if (!UIPackage::find_bitmap_font(font_name, bitmap_font, package_id) || !bitmap_font.loaded) {
        return false;
    }

    const int32_t base_size = bitmap_font.size > 0 ? bitmap_font.size : (bitmap_font.line_height > 0 ? bitmap_font.line_height : get_theme_default_font_size());
    const int32_t resolved_font_size = font_size > 0 ? font_size : base_size;
    const float scale = bitmap_font.resizable && base_size > 0 ? static_cast<float>(resolved_font_size) / static_cast<float>(base_size) : 1.0f;
    const float line_height = static_cast<float>(bitmap_font.line_height > 0 ? bitmap_font.line_height : base_size) * scale;

    float x = 0.0f;
    float y = 0.0f;
    for (int64_t i = 0; i < display_text.length(); i++) {
        const char32_t ch = display_text[i];
        if (ch == '\r') {
            continue;
        }
        if (ch == '\n') {
            x = 0.0f;
            y += line_height;
            continue;
        }
        if (ch == ' ') {
            x += static_cast<float>(base_size) * scale * 0.5f;
            continue;
        }
        if (!bitmap_font.glyphs.has(static_cast<int32_t>(ch))) {
            continue;
        }

        const fgui::BitmapFontGlyph glyph = bitmap_font.glyphs[static_cast<int32_t>(ch)];
        Ref<Texture2D> texture = UIPackage::get_bitmap_font_glyph_texture(package_id, glyph);
        if (texture.is_valid()) {
            const Rect2 rect(Vector2(x + glyph.x * scale, y + glyph.y * scale), Vector2(glyph.width * scale, glyph.height * scale));
            draw_texture_rect(texture, rect, false, bitmap_font.can_tint ? text_color : Color(1, 1, 1, 1));
        }
        x += static_cast<float>(glyph.advance) * scale;
    }
    return true;
}

bool GTextField::draw_true_type_font() {
    if (font_name.is_empty() || display_text.is_empty()) {
        return false;
    }

    Ref<Font> font = UIPackage::get_true_type_font(font_name, font_size);
    if (font.is_null()) {
        return false;
    }

    const int resolved_size = font_size > 0 ? font_size : 14;
    font->draw_string(get_canvas_item(), Vector2(0, resolved_size), display_text, horizontal_alignment, get_size().x, resolved_size, text_color);
    return true;
}

void GTextField::update_rich_text_label() {
    if (!rich_text_enabled) {
        return;
    }

    RichTextLabel *label = ensure_rich_text_label();
    label->set_position(Vector2());
    label->set_size(get_size());
    label->set_modulate(text_color);
    label->clear();

    ApplyRichTextFont override = apply_rich_text_font_override(label);

    label->set_use_bbcode(true);
    label->parse_bbcode(rich_text_bbcode);
}

RichTextLabel *GTextField::ensure_rich_text_label() {
    if (rich_text_label != nullptr) {
        return rich_text_label;
    }

    rich_text_label = memnew(RichTextLabel);
    rich_text_label->set_name("_RichTextLabel");
    rich_text_label->set_use_bbcode(true);
    rich_text_label->set_scroll_active(false);
    rich_text_label->set_fit_content(false);
    rich_text_label->set_mouse_filter(Control::MOUSE_FILTER_PASS);
    add_child(rich_text_label);

    Callable callback(this, StringName("_handle_rich_text_meta_clicked"));
    if (!rich_text_label->is_connected(StringName("meta_clicked"), callback)) {
        rich_text_label->connect(StringName("meta_clicked"), callback);
    }
    return rich_text_label;
}

GTextField::ApplyRichTextFont GTextField::apply_rich_text_font_override(RichTextLabel *p_label) {
    if (font_name.is_empty()) {
        return ApplyRichTextFont();
    }

    Ref<Font> font = UIPackage::get_true_type_font(font_name, font_size);
    int32_t resolved = font_size;

    if (font.is_null()) {
        font = UIPackage::get_true_type_font(font_name, get_theme_default_font_size());
        resolved = get_theme_default_font_size();
    }

    if (font.is_null()) {
        return ApplyRichTextFont();
    }

    const ApplyRichTextFont result{true, resolved};
    p_label->add_theme_font_override("normal_font", font);
    if (resolved > 0) {
        p_label->add_theme_font_size_override("normal_font_size", resolved);
    }
    return result;
}

void GTextField::clear_rich_text_label() {
    if (rich_text_label == nullptr) {
        return;
    }
    remove_child(rich_text_label);
    memdelete(rich_text_label);
    rich_text_label = nullptr;
}

void GTextField::_handle_rich_text_meta_clicked(const Variant &p_meta) {
    const String href = p_meta.get_type() == Variant::STRING ? static_cast<String>(p_meta) : p_meta.stringify();
    emit_signal("fgui_link_click", href);
}

void GTextField::set_align(int32_t p_align) {
    horizontal_alignment = p_align == 1 ? HORIZONTAL_ALIGNMENT_CENTER : (p_align == 2 ? HORIZONTAL_ALIGNMENT_RIGHT : HORIZONTAL_ALIGNMENT_LEFT);
    update_rich_text_label();
    queue_redraw();
}

int32_t GTextField::get_align() const {
    if (horizontal_alignment == HORIZONTAL_ALIGNMENT_CENTER) return 1;
    if (horizontal_alignment == HORIZONTAL_ALIGNMENT_RIGHT) return 2;
    return 0;
}

void GTextField::set_vertical_align(int32_t p_vertical_align) {
    vertical_alignment = p_vertical_align;
    queue_redraw();
}

int32_t GTextField::get_vertical_align() const { return vertical_alignment; }

void GTextField::set_single_line(bool p_single_line) { single_line = p_single_line; }
bool GTextField::is_single_line() const { return single_line; }

void GTextField::set_auto_size(int32_t p_auto_size) { auto_size = p_auto_size; }
int32_t GTextField::get_auto_size() const { return auto_size; }

float GTextField::get_text_width() const {
    Ref<Font> font = get_theme_default_font();
    if (font.is_valid()) {
        return font->get_string_size(display_text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size > 0 ? font_size : get_theme_default_font_size()).x;
    }
    return 0.0f;
}

float GTextField::get_text_height() const {
    Ref<Font> font = get_theme_default_font();
    if (font.is_valid()) {
        return font->get_height(font_size > 0 ? font_size : get_theme_default_font_size());
    }
    return 0.0f;
}
