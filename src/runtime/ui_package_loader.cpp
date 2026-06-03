#include "ui_package.h"
#include "package/ui_package_internal.h"

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/core/error_macros.hpp>

using namespace godot;
using namespace godot::fgui_internal;

namespace godot {
namespace fgui_internal {

bool component_contains_object_type(const fgui::PackageItem &p_item, fgui::ObjectType p_type) {
    if (p_item.type != fgui::PackageItemType::Component) {
        return false;
    }

    fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 2)) {
        return false;
    }

    const int32_t child_count = buffer.read_short();
    for (int32_t i = 0; i < child_count; i++) {
        const int32_t data_len = buffer.read_short();
        const int64_t child_begin_pos = buffer.get_position();
        if (buffer.seek(child_begin_pos, 0)) {
            const fgui::ObjectType child_type = static_cast<fgui::ObjectType>(buffer.read_byte());
            if (child_type == p_type) {
                return true;
            }
        }
        buffer.set_position(child_begin_pos + data_len);
    }
    return false;
}

void parse_movie_clip_data(fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 0)) {
        return;
    }

    p_item.interval = static_cast<float>(buffer.read_int()) / 1000.0f;
    p_item.swing = buffer.read_bool();
    p_item.repeat_delay = static_cast<float>(buffer.read_int()) / 1000.0f;

    if (!buffer.seek(0, 1)) {
        return;
    }

    const int32_t frame_count = buffer.read_short();
    p_item.movie_clip_frames.resize(frame_count);
    for (int32_t i = 0; i < frame_count; i++) {
        const int32_t next_pos = buffer.read_ushort() + buffer.get_position();
        fgui::MovieClipFrame frame;
        frame.rect.position.x = buffer.read_int();
        frame.rect.position.y = buffer.read_int();
        frame.rect.size.x = buffer.read_int();
        frame.rect.size.y = buffer.read_int();
        frame.add_delay = static_cast<float>(buffer.read_int()) / 1000.0f;
        frame.sprite_id = buffer.read_s();
        p_item.movie_clip_frames.set(i, frame);
        buffer.set_position(next_pos);
    }
}

void parse_bitmap_font_data(fgui::PackageData &p_package, fgui::PackageItem &p_item) {
    fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 0)) {
        return;
    }

    p_item.bitmap_font = fgui::BitmapFontData();
    p_item.bitmap_font.loaded = true;
    p_item.bitmap_font.ttf = buffer.read_bool();
    p_item.bitmap_font.can_tint = buffer.read_bool();
    p_item.bitmap_font.resizable = buffer.read_bool();
    p_item.bitmap_font.has_channel = buffer.read_bool();
    p_item.bitmap_font.size = buffer.read_int();
    p_item.bitmap_font.xadvance = buffer.read_int();
    p_item.bitmap_font.line_height = buffer.read_int();

    if (!buffer.seek(0, 1)) {
        return;
    }

    const int32_t glyph_count = buffer.read_int();
    for (int32_t i = 0; i < glyph_count; i++) {
        const int32_t next_pos = buffer.read_ushort() + buffer.get_position();

        fgui::BitmapFontGlyph glyph;
        glyph.char_code = buffer.read_ushort();
        glyph.image_item_id = buffer.read_s();
        buffer.read_int();
        buffer.read_int();
        const int32_t bg_x = buffer.read_int();
        const int32_t bg_y = buffer.read_int();
        int32_t bg_width = buffer.read_int();
        int32_t bg_height = buffer.read_int();
        glyph.advance = buffer.read_int();
        glyph.channel = buffer.read_byte();

        if (glyph.channel == 1) {
            glyph.channel = 2;
        } else if (glyph.channel == 2) {
            glyph.channel = 1;
        } else if (glyph.channel == 4) {
            glyph.channel = 0;
        } else if (glyph.channel == 8) {
            glyph.channel = 3;
        }

        if (!p_item.bitmap_font.ttf) {
            const fgui::PackageItem *char_item = find_package_item(p_package, glyph.image_item_id);
            if (char_item != nullptr) {
                if (bg_width == 0) {
                    bg_width = char_item->width;
                }
                if (bg_height == 0) {
                    bg_height = char_item->height;
                }
            }

            if (p_item.bitmap_font.size == 0) {
                p_item.bitmap_font.size = bg_height;
            }
            if (glyph.advance == 0) {
                glyph.advance = p_item.bitmap_font.xadvance == 0 ? bg_x + bg_width : p_item.bitmap_font.xadvance;
            }
            glyph.line_height = bg_y < 0 ? bg_height : bg_y + bg_height;
            if (glyph.line_height < p_item.bitmap_font.size) {
                glyph.line_height = p_item.bitmap_font.size;
            }
            glyph.x = bg_x;
            glyph.y = bg_y;
            glyph.width = bg_width;
            glyph.height = bg_height;
        } else {
            glyph.x = bg_x;
            glyph.y = bg_y;
            glyph.width = bg_width;
            glyph.height = bg_height;
            glyph.line_height = p_item.bitmap_font.line_height;
            glyph.image_item_id = p_item.id;
        }

        if (p_item.bitmap_font.line_height == 0) {
            p_item.bitmap_font.line_height = glyph.line_height;
        }
        p_item.bitmap_font.glyphs[glyph.char_code] = glyph;
        buffer.set_position(next_pos);
    }
}

} // namespace fgui_internal
} // namespace godot

bool UIPackage::load_package(fgui::ByteBuffer &p_buffer, const String &p_asset_name_prefix, fgui::PackageData &r_package) {
    ERR_FAIL_COND_V_MSG(p_buffer.read_uint() != FGUI_PACKAGE_MAGIC, false, "Invalid or unsupported FairyGUI package magic.");

    p_buffer.version = p_buffer.read_int();
    const bool version_2_or_later = p_buffer.version >= 2;
    p_buffer.read_bool();
    r_package.version = p_buffer.version;
    r_package.id = p_buffer.read_string();
    r_package.name = p_buffer.read_string();
    r_package.asset_name_prefix = p_asset_name_prefix;

    p_buffer.skip(20);
    const int64_t index_table_pos = p_buffer.get_position();

    if (!p_buffer.seek(index_table_pos, 4)) {
        ERR_FAIL_V_MSG(false, "FairyGUI package is missing string table block.");
    }

    int32_t count = p_buffer.read_int();
    Vector<String> string_table;
    string_table.resize(count);
    for (int32_t i = 0; i < count; i++) {
        string_table.set(i, p_buffer.read_string());
    }
    p_buffer.string_table = string_table;

    if (p_buffer.seek(index_table_pos, 5)) {
        count = p_buffer.read_int();
        for (int32_t i = 0; i < count; i++) {
            const int32_t index = p_buffer.read_ushort();
            const int32_t length = p_buffer.read_int();
            ERR_FAIL_COND_V_MSG(index < 0 || index >= p_buffer.string_table.size(), false, "String table patch index out of range.");
            p_buffer.string_table.set(index, p_buffer.read_string(length));
        }
    }

    if (p_buffer.seek(index_table_pos, 0)) {
        count = p_buffer.read_short();
        for (int32_t i = 0; i < count; i++) {
            p_buffer.read_s();
            p_buffer.read_s();
        }

        if (version_2_or_later) {
            count = p_buffer.read_short();
            if (count > 0) {
                p_buffer.read_s_array(count);
            }
        }
    }

    const String asset_name_prefix = asset_prefix_to_package_prefix(p_asset_name_prefix);

    if (p_buffer.seek(index_table_pos, 1)) {
        count = p_buffer.read_short();
        r_package.items.resize(count);
        for (int32_t i = 0; i < count; i++) {
            const int32_t next_pos = p_buffer.read_int() + p_buffer.get_position();

            fgui::PackageItem item;
            item.type = static_cast<fgui::PackageItemType>(p_buffer.read_byte());
            item.id = p_buffer.read_s();
            item.name = p_buffer.read_s();
            p_buffer.read_s();
            item.file = p_buffer.read_s();
            item.exported = p_buffer.read_bool();
            item.width = p_buffer.read_int();
            item.height = p_buffer.read_int();

            switch (item.type) {
                case fgui::PackageItemType::Image: {
                    item.object_type = fgui::ObjectType::Image;
                    const int32_t scale_option = p_buffer.read_byte();
                    if (scale_option == 1) {
                        item.has_scale9_grid = true;
                        item.scale9_grid.position.x = p_buffer.read_int();
                        item.scale9_grid.position.y = p_buffer.read_int();
                        item.scale9_grid.size.x = p_buffer.read_int();
                        item.scale9_grid.size.y = p_buffer.read_int();
                        item.tile_grid_indice = p_buffer.read_int();
                    } else if (scale_option == 2) {
                        item.scale_by_tile = true;
                    }
                    p_buffer.read_bool();
                    break;
                }
                case fgui::PackageItemType::MovieClip: {
                    p_buffer.read_bool();
                    item.object_type = fgui::ObjectType::MovieClip;
                    item.raw_data = p_buffer.read_buffer();
                    parse_movie_clip_data(item);
                    break;
                }
                case fgui::PackageItemType::Font: {
                    item.raw_data = p_buffer.read_buffer();
                    break;
                }
                case fgui::PackageItemType::Component: {
                    const int32_t extension = p_buffer.read_byte();
                    item.object_type = extension > 0 ? static_cast<fgui::ObjectType>(extension) : fgui::ObjectType::Component;
                    item.raw_data = p_buffer.read_buffer();
                    break;
                }
                case fgui::PackageItemType::Atlas:
                case fgui::PackageItemType::Sound:
                case fgui::PackageItemType::Misc: {
                    item.file = asset_name_prefix + item.file;
                    break;
                }
                case fgui::PackageItemType::Spine:
                case fgui::PackageItemType::DragonBones: {
                    p_buffer.read_float();
                    p_buffer.read_float();
                    break;
                }
                default:
                    break;
            }

            if (version_2_or_later) {
                const String branch = p_buffer.read_s();
                if (!branch.is_empty()) {
                    item.name = branch + String("/") + item.name;
                }

                const int32_t branch_count = p_buffer.read_byte();
                if (branch_count > 0) {
                    item.branches = p_buffer.read_s_array(branch_count);
                }

                const int32_t high_res_count = p_buffer.read_byte();
                if (high_res_count > 0) {
                    item.high_resolution = p_buffer.read_s_array(high_res_count);
                }
            }

            r_package.items.set(i, item);
            r_package.items_by_id[item.id] = i;
            if (!item.name.is_empty()) {
                r_package.items_by_name[item.name] = i;
            }

            p_buffer.set_position(next_pos);
        }
    }

    if (p_buffer.seek(index_table_pos, 2)) {
        count = p_buffer.read_short();
        for (int32_t i = 0; i < count; i++) {
            const int32_t next_pos = p_buffer.read_ushort() + p_buffer.get_position();

            const String item_id = p_buffer.read_s();
            const String atlas_item_id = p_buffer.read_s();

            fgui::AtlasSprite sprite;
            sprite.atlas_item_id = atlas_item_id;
            sprite.rect.position.x = p_buffer.read_int();
            sprite.rect.position.y = p_buffer.read_int();
            sprite.rect.size.x = p_buffer.read_int();
            sprite.rect.size.y = p_buffer.read_int();
            sprite.rotated = p_buffer.read_bool();
            if (version_2_or_later && p_buffer.read_bool()) {
                sprite.offset.x = p_buffer.read_int();
                sprite.offset.y = p_buffer.read_int();
                sprite.original_size.x = p_buffer.read_int();
                sprite.original_size.y = p_buffer.read_int();
            } else if (sprite.rotated) {
                sprite.original_size.x = sprite.rect.size.y;
                sprite.original_size.y = sprite.rect.size.x;
            } else {
                sprite.original_size = sprite.rect.size;
            }

            r_package.sprites[item_id] = sprite;
            p_buffer.set_position(next_pos);
        }
    }

    for (int32_t i = 0; i < r_package.items.size(); i++) {
        fgui::PackageItem item = r_package.items[i];
        if (item.type == fgui::PackageItemType::Font) {
            parse_bitmap_font_data(r_package, item);
            r_package.items.set(i, item);
        }
    }

    if (p_buffer.seek(index_table_pos, 3)) {
        count = p_buffer.read_short();
        for (int32_t i = 0; i < count; i++) {
            const int32_t next_pos = p_buffer.read_int() + p_buffer.get_position();
            const String item_id = p_buffer.read_s();
            if (r_package.items_by_id.has(item_id)) {
                const int32_t item_index = r_package.items_by_id[item_id];
                fgui::PackageItem item = r_package.items[item_index];
                if (item.type == fgui::PackageItemType::Image) {
                    item.has_pixel_hit_test_data = true;
                    item.pixel_hit_test_data.load(p_buffer);
                    r_package.items.set(item_index, item);
                }
            }
            p_buffer.set_position(next_pos);
        }
    }

    for (int32_t i = 0; i < r_package.items.size(); i++) {
        const fgui::PackageItem &item = r_package.items[i];
        if (item.type == fgui::PackageItemType::Font && item.bitmap_font.loaded && item.bitmap_font.glyphs.is_empty()) {
            UIPackage::get_true_type_font(item.name, item.bitmap_font.size > 0 ? item.bitmap_font.size : 14);
        }
    }

    return true;
}
