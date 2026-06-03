#pragma once

#include "g_object.h"

namespace godot {

class RichTextLabel;

class GTextField : public GObject {
    GDCLASS(GTextField, GObject)

protected:
    static void _bind_methods();

public:
    void _draw() override;
    void _notification(int p_what);
    void setup_before_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;
    void setup_after_add(fgui::ByteBuffer &p_buffer, int64_t p_begin_pos) override;

    virtual void set_text(const String &p_text);
    virtual String get_text() const;
    String get_display_text() const;
    virtual void set_text_color(const Color &p_color);
    virtual Color get_text_color() const;
    virtual void set_font_size(int32_t p_size);
    virtual int32_t get_font_size() const;
    virtual void set_font_name(const String &p_font_name);
    String get_font_name() const;

    void set_align(int32_t p_align);
    int32_t get_align() const;
    void set_vertical_align(int32_t p_vertical_align);
    int32_t get_vertical_align() const;
    void set_single_line(bool p_single_line);
    bool is_single_line() const;

    void set_auto_size(int32_t p_auto_size);
    int32_t get_auto_size() const;

    float get_text_width() const;
    float get_text_height() const;

    void set_ubb_enabled(bool p_enabled);
    bool is_ubb_enabled() const;
    int32_t get_ubb_image_count() const;
    int32_t get_ubb_link_count() const;
    void set_rich_text_enabled(bool p_enabled);
    bool is_rich_text_enabled() const;
    String get_rich_text_bbcode() const;
    bool has_rich_text_label() const;
    int32_t get_rich_text_owner_instance_id() const;

protected:
    String text;
    String display_text;
    String font_name;
    int32_t font_size = 0;
    Color text_color = Color(1, 1, 1, 1);
    HorizontalAlignment horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    int32_t vertical_alignment = 0;
    int32_t auto_size = 0;
    bool single_line = false;
    bool ubb_enabled = false;
    bool rich_text_enabled = false;
    int32_t ubb_image_count = 0;
    int32_t ubb_link_count = 0;
    String rich_text_bbcode;
    RichTextLabel *rich_text_label = nullptr;

    void refresh_display_text();
    void _handle_rich_text_meta_clicked(const Variant &p_meta);

private:
    struct ApplyRichTextFont {
        bool applied = false;
        int32_t size = 0;
    };

    bool draw_bitmap_font();
    bool draw_true_type_font();
    ApplyRichTextFont apply_rich_text_font_override(RichTextLabel *p_label);
    void update_rich_text_label();
    RichTextLabel *ensure_rich_text_label();
    void clear_rich_text_label();
};

} // namespace godot
