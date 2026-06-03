#include "ui_package.h"

#include "g_button.h"
#include "g_combo_box.h"
#include "g_component.h"
#include "g_image.h"
#include "g_label.h"
#include "g_list.h"
#include "g_movie_clip.h"
#include "g_progress_bar.h"
#include "g_scroll_bar.h"
#include "g_slider.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_mp3.hpp>
#include <godot_cpp/classes/audio_stream_ogg_vorbis.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/atlas_texture.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace {
constexpr uint32_t FGUI_PACKAGE_MAGIC = 0x46475549;

godot::HashMap<godot::String, godot::fgui::PackageData> &get_packages_by_id() {
    static godot::HashMap<godot::String, godot::fgui::PackageData> packages;
    return packages;
}

godot::HashMap<godot::String, godot::String> &get_package_id_by_name() {
    static godot::HashMap<godot::String, godot::String> package_ids;
    return package_ids;
}

godot::HashMap<godot::String, godot::String> &get_asset_path_overrides() {
    static godot::HashMap<godot::String, godot::String> overrides;
    return overrides;
}

godot::HashMap<godot::String, godot::Ref<godot::Texture2D>> &get_texture_cache() {
    static godot::HashMap<godot::String, godot::Ref<godot::Texture2D>> textures;
    return textures;
}

godot::HashMap<godot::String, godot::Ref<godot::AudioStream>> &get_audio_cache() {
    static godot::HashMap<godot::String, godot::Ref<godot::AudioStream>> streams;
    return streams;
}

godot::HashMap<godot::String, godot::Callable> &get_custom_loaders_by_prefix() {
    static godot::HashMap<godot::String, godot::Callable> loaders;
    return loaders;
}

godot::String &get_last_package_id_storage() {
    static godot::String last_package_id;
    return last_package_id;
}

godot::String package_bytes_path_from_prefix(const godot::String &p_asset_name_prefix) {
    if (p_asset_name_prefix.is_empty()) {
        return godot::String("_fui.bytes");
    }
    return p_asset_name_prefix + godot::String("_fui.bytes");
}

godot::String asset_prefix_to_package_prefix(const godot::String &p_asset_name_prefix) {
    if (p_asset_name_prefix.is_empty()) {
        return godot::String();
    }
    return p_asset_name_prefix + godot::String("_");
}

godot::String normalize_asset_name_prefix(const godot::String &p_path) {
    godot::String prefix = p_path.get_basename();
    if (prefix.ends_with("_fui")) {
        prefix = prefix.substr(0, prefix.length() - 4);
    }
    return prefix;
}

godot::String resolve_package_id(const godot::String &p_package_id_or_name) {
    godot::HashMap<godot::String, godot::fgui::PackageData> &packages_by_id = get_packages_by_id();
    godot::HashMap<godot::String, godot::String> &package_id_by_name = get_package_id_by_name();
    if (packages_by_id.has(p_package_id_or_name)) {
        return p_package_id_or_name;
    }
    if (package_id_by_name.has(p_package_id_or_name)) {
        return package_id_by_name[p_package_id_or_name];
    }
    return godot::String();
}

bool parse_item_url(const godot::String &p_url, godot::String &r_package, godot::String &r_item) {
    constexpr const char *URL_PREFIX = "ui://";
    if (!p_url.begins_with(URL_PREFIX)) {
        return false;
    }

    godot::String body = p_url.substr(5);
    const int64_t slash = body.find("/");
    if (slash < 0) {
        return false;
    }

    r_package = body.substr(0, slash);
    r_item = body.substr(slash + 1);
    return !r_package.is_empty() && !r_item.is_empty();
}

const godot::fgui::PackageItem *find_package_item(const godot::fgui::PackageData &p_package, const godot::String &p_item_id_or_name) {
    if (p_package.items_by_id.has(p_item_id_or_name)) {
        return &p_package.items[p_package.items_by_id[p_item_id_or_name]];
    }
    if (p_package.items_by_name.has(p_item_id_or_name)) {
        return &p_package.items[p_package.items_by_name[p_item_id_or_name]];
    }
    return nullptr;
}

bool component_contains_object_type(const godot::fgui::PackageItem &p_item, godot::fgui::ObjectType p_type) {
    if (p_item.type != godot::fgui::PackageItemType::Component) {
        return false;
    }

    godot::fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 2)) {
        return false;
    }

    const int32_t child_count = buffer.read_short();
    for (int32_t i = 0; i < child_count; i++) {
        const int32_t data_len = buffer.read_short();
        const int64_t child_begin_pos = buffer.get_position();
        if (buffer.seek(child_begin_pos, 0)) {
            const godot::fgui::ObjectType child_type = static_cast<godot::fgui::ObjectType>(buffer.read_byte());
            if (child_type == p_type) {
                return true;
            }
        }
        buffer.set_position(child_begin_pos + data_len);
    }
    return false;
}

godot::String resolve_asset_path_internal(const godot::String &p_path) {
    godot::HashMap<godot::String, godot::String> &overrides = get_asset_path_overrides();
    if (overrides.has(p_path)) {
        return overrides[p_path];
    }
    return p_path;
}

godot::String find_best_loader_prefix(const godot::String &p_path) {
    godot::HashMap<godot::String, godot::Callable> &loaders = get_custom_loaders_by_prefix();
    godot::String best_prefix;
    for (const godot::KeyValue<godot::String, godot::Callable> &loader_pair : loaders) {
        const godot::String &prefix = loader_pair.key;
        if ((prefix.is_empty() || p_path.begins_with(prefix)) && prefix.length() >= best_prefix.length()) {
            best_prefix = prefix;
        }
    }
    return best_prefix;
}

godot::Variant call_custom_loader(const godot::String &p_path) {
    godot::HashMap<godot::String, godot::Callable> &loaders = get_custom_loaders_by_prefix();
    const godot::String prefix = find_best_loader_prefix(p_path);
    if (!loaders.has(prefix)) {
        return godot::Variant();
    }

    const godot::Callable &loader = loaders[prefix];
    if (!loader.is_valid()) {
        return godot::Variant();
    }
    return loader.call(p_path);
}

godot::Ref<godot::Texture2D> texture_from_bytes(const godot::String &p_path, const godot::PackedByteArray &p_bytes) {
    if (p_bytes.is_empty()) {
        return godot::Ref<godot::Texture2D>();
    }

    godot::Ref<godot::Image> image;
    image.instantiate();
    const godot::String extension = p_path.get_extension().to_lower();
    godot::Error error = godot::ERR_UNAVAILABLE;
    if (extension == "png") {
        error = image->load_png_from_buffer(p_bytes);
    } else if (extension == "jpg" || extension == "jpeg") {
        error = image->load_jpg_from_buffer(p_bytes);
    } else if (extension == "webp") {
        error = image->load_webp_from_buffer(p_bytes);
    } else if (extension == "bmp") {
        error = image->load_bmp_from_buffer(p_bytes);
    } else if (extension == "tga") {
        error = image->load_tga_from_buffer(p_bytes);
    }

    if (error != godot::OK || image->is_empty()) {
        return godot::Ref<godot::Texture2D>();
    }
    return godot::ImageTexture::create_from_image(image);
}

godot::Ref<godot::Texture2D> load_texture_from_custom_loader(const godot::String &p_path) {
    godot::Variant loaded = call_custom_loader(p_path);
    if (loaded.get_type() == godot::Variant::NIL) {
        return godot::Ref<godot::Texture2D>();
    }

    godot::Ref<godot::Texture2D> texture = loaded;
    if (texture.is_valid()) {
        return texture;
    }

    if (loaded.get_type() == godot::Variant::PACKED_BYTE_ARRAY) {
        return texture_from_bytes(p_path, static_cast<godot::PackedByteArray>(loaded));
    }

    return godot::Ref<godot::Texture2D>();
}

godot::Ref<godot::AudioStream> audio_from_bytes(const godot::String &p_path, const godot::PackedByteArray &p_bytes) {
    if (p_bytes.is_empty()) {
        return godot::Ref<godot::AudioStream>();
    }

    const godot::String extension = p_path.get_extension().to_lower();
    if (extension == "wav") {
        return godot::AudioStreamWAV::load_from_buffer(p_bytes);
    }
    if (extension == "ogg") {
        return godot::AudioStreamOggVorbis::load_from_buffer(p_bytes);
    }
    if (extension == "mp3") {
        return godot::AudioStreamMP3::load_from_buffer(p_bytes);
    }
    return godot::Ref<godot::AudioStream>();
}

godot::Ref<godot::AudioStream> load_audio_from_custom_loader(const godot::String &p_path) {
    godot::Variant loaded = call_custom_loader(p_path);
    if (loaded.get_type() == godot::Variant::NIL) {
        return godot::Ref<godot::AudioStream>();
    }
    if (loaded.get_type() == godot::Variant::OBJECT) {
        godot::Ref<godot::AudioStream> stream = loaded;
        if (stream.is_valid()) {
            return stream;
        }
    }
    if (loaded.get_type() == godot::Variant::PACKED_BYTE_ARRAY) {
        return audio_from_bytes(p_path, static_cast<godot::PackedByteArray>(loaded));
    }
    return godot::Ref<godot::AudioStream>();
}

godot::Ref<godot::AudioStream> load_audio_from_path(const godot::String &p_path) {
    const godot::String resolved_path = resolve_asset_path_internal(p_path);
    godot::Ref<godot::AudioStream> custom_stream = load_audio_from_custom_loader(resolved_path);
    if (custom_stream.is_valid()) {
        return custom_stream;
    }

    if (godot::ResourceLoader::get_singleton()->exists(resolved_path)) {
        godot::Ref<godot::AudioStream> resource_stream = godot::ResourceLoader::get_singleton()->load(resolved_path);
        if (resource_stream.is_valid()) {
            return resource_stream;
        }
    }

    const godot::String extension = resolved_path.get_extension().to_lower();
    if (extension == "wav") {
        return godot::AudioStreamWAV::load_from_file(resolved_path);
    }
    if (extension == "ogg") {
        return godot::AudioStreamOggVorbis::load_from_file(resolved_path);
    }
    if (extension == "mp3") {
        return godot::AudioStreamMP3::load_from_file(resolved_path);
    }

    godot::Ref<godot::FileAccess> file = godot::FileAccess::open(resolved_path, godot::FileAccess::READ);
    if (file.is_valid()) {
        return audio_from_bytes(resolved_path, file->get_buffer(file->get_length()));
    }
    return godot::Ref<godot::AudioStream>();
}

godot::PackedByteArray load_bytes_from_custom_loader(const godot::String &p_path) {
    godot::Variant loaded = call_custom_loader(p_path);
    if (loaded.get_type() == godot::Variant::PACKED_BYTE_ARRAY) {
        return static_cast<godot::PackedByteArray>(loaded);
    }
    return godot::PackedByteArray();
}

godot::PackedByteArray load_bytes_from_callable(const godot::Callable &p_loader, const godot::String &p_path) {
    if (!p_loader.is_valid()) {
        return godot::PackedByteArray();
    }
    godot::Variant loaded = p_loader.call(p_path);
    if (loaded.get_type() == godot::Variant::PACKED_BYTE_ARRAY) {
        return static_cast<godot::PackedByteArray>(loaded);
    }
    return godot::PackedByteArray();
}

godot::Ref<godot::Texture2D> load_texture_from_resolved_path(const godot::String &p_resolved_path) {
    godot::Ref<godot::Texture2D> custom_texture = load_texture_from_custom_loader(p_resolved_path);
    if (custom_texture.is_valid()) {
        return custom_texture;
    }

    if (p_resolved_path.begins_with("res://")) {
        godot::Ref<godot::Resource> resource = godot::ResourceLoader::get_singleton()->load(p_resolved_path);
        godot::Ref<godot::Texture2D> texture = resource;
        if (texture.is_valid()) {
            return texture;
        }
    }

    godot::Ref<godot::Image> image = godot::Image::load_from_file(p_resolved_path);
    if (image.is_null() || image->is_empty()) {
        return godot::Ref<godot::Texture2D>();
    }
    return godot::ImageTexture::create_from_image(image);
}

void parse_movie_clip_data(godot::fgui::PackageItem &p_item) {
    godot::fgui::ByteBuffer buffer = p_item.raw_data;
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
        godot::fgui::MovieClipFrame frame;
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

void parse_bitmap_font_data(godot::fgui::PackageData &p_package, godot::fgui::PackageItem &p_item) {
    godot::fgui::ByteBuffer buffer = p_item.raw_data;
    if (!buffer.seek(0, 0)) {
        return;
    }

    p_item.bitmap_font = godot::fgui::BitmapFontData();
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

        godot::fgui::BitmapFontGlyph glyph;
        glyph.char_code = buffer.read_ushort();
        glyph.image_item_id = buffer.read_s();
        buffer.read_int(); // bx, only needed by Unity's packed TTF atlas UV path.
        buffer.read_int(); // by, only needed by Unity's packed TTF atlas UV path.
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
            const godot::fgui::PackageItem *char_item = find_package_item(p_package, glyph.image_item_id);
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
} // namespace

using namespace godot;

void UIPackage::_bind_methods() {
    ClassDB::bind_static_method("UIPackage", D_METHOD("add_package", "path"), &UIPackage::add_package);
    ClassDB::bind_static_method("UIPackage", D_METHOD("add_package_bytes", "bytes", "asset_name_prefix"), &UIPackage::add_package_bytes);
    ClassDB::bind_static_method("UIPackage", D_METHOD("add_package_with_loader", "asset_name_prefix", "loader"), &UIPackage::add_package_with_loader);
    ClassDB::bind_static_method("UIPackage", D_METHOD("add_package_bytes_with_loader", "bytes", "asset_name_prefix", "loader"), &UIPackage::add_package_bytes_with_loader);
    ClassDB::bind_static_method("UIPackage", D_METHOD("clear_custom_loaders"), &UIPackage::clear_custom_loaders);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_custom_loader_count"), &UIPackage::get_custom_loader_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("set_asset_path_override", "source_path", "mapped_path"), &UIPackage::set_asset_path_override);
    ClassDB::bind_static_method("UIPackage", D_METHOD("remove_asset_path_override", "source_path"), &UIPackage::remove_asset_path_override);
    ClassDB::bind_static_method("UIPackage", D_METHOD("clear_asset_path_overrides"), &UIPackage::clear_asset_path_overrides);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_asset_path_override_count"), &UIPackage::get_asset_path_override_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("resolve_asset_path", "path"), &UIPackage::resolve_asset_path);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_resource_texture", "path"), &UIPackage::get_resource_texture);
    ClassDB::bind_static_method("UIPackage", D_METHOD("clear_texture_cache"), &UIPackage::clear_texture_cache);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_texture_cache_count"), &UIPackage::get_texture_cache_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("clear_audio_cache"), &UIPackage::clear_audio_cache);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_audio_cache_count"), &UIPackage::get_audio_cache_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("has_package", "package_id_or_name"), &UIPackage::has_package);
    ClassDB::bind_static_method("UIPackage", D_METHOD("remove_package", "package_id_or_name"), &UIPackage::remove_package);
    ClassDB::bind_static_method("UIPackage", D_METHOD("remove_all_packages"), &UIPackage::remove_all_packages);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_package_count"), &UIPackage::get_package_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_last_package_id"), &UIPackage::get_last_package_id);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_last_package_name"), &UIPackage::get_last_package_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_item_count", "package_id_or_name"), &UIPackage::get_item_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_sprite_count", "package_id_or_name"), &UIPackage::get_sprite_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_pixel_hit_test_item_count", "package_id_or_name"), &UIPackage::get_pixel_hit_test_item_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_scale9_image_item_count", "package_id_or_name"), &UIPackage::get_scale9_image_item_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_bitmap_font_item_count", "package_id_or_name"), &UIPackage::get_bitmap_font_item_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_image_item_name", "package_id_or_name"), &UIPackage::get_first_image_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_scale9_image_item_name", "package_id_or_name"), &UIPackage::get_first_scale9_image_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_bitmap_font_item_name", "package_id_or_name"), &UIPackage::get_first_bitmap_font_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_pixel_hit_test_item_name", "package_id_or_name"), &UIPackage::get_first_pixel_hit_test_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_component_item_name", "package_id_or_name"), &UIPackage::get_first_component_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_button_item_name", "package_id_or_name"), &UIPackage::get_first_button_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_label_item_name", "package_id_or_name"), &UIPackage::get_first_label_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_list_item_name", "package_id_or_name"), &UIPackage::get_first_list_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_progress_bar_item_name", "package_id_or_name"), &UIPackage::get_first_progress_bar_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_slider_item_name", "package_id_or_name"), &UIPackage::get_first_slider_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_scroll_bar_item_name", "package_id_or_name"), &UIPackage::get_first_scroll_bar_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_combo_box_item_name", "package_id_or_name"), &UIPackage::get_first_combo_box_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_movie_clip_item_name", "package_id_or_name"), &UIPackage::get_first_movie_clip_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_input_text_component_item_name", "package_id_or_name"), &UIPackage::get_first_input_text_component_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_first_sound_item_name", "package_id_or_name"), &UIPackage::get_first_sound_item_name);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_item_url", "package_id_or_name", "item_id_or_name"), &UIPackage::get_item_url);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_sound_audio_stream", "package_id_or_name", "item_id_or_name"), &UIPackage::get_sound_audio_stream);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_sound_audio_stream_by_url", "url"), &UIPackage::get_sound_audio_stream_by_url);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_image_texture", "package_id_or_name", "item_id_or_name"), &UIPackage::get_image_texture);
    ClassDB::bind_static_method("UIPackage", D_METHOD("has_image_scale9_grid", "package_id_or_name", "item_id_or_name"), &UIPackage::has_image_scale9_grid);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_image_scale9_grid", "package_id_or_name", "item_id_or_name"), &UIPackage::get_image_scale9_grid);
    ClassDB::bind_static_method("UIPackage", D_METHOD("has_bitmap_font", "package_id_or_name", "item_id_or_name"), &UIPackage::has_bitmap_font);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_bitmap_font_glyph_count", "package_id_or_name", "item_id_or_name"), &UIPackage::get_bitmap_font_glyph_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_bitmap_font_size", "package_id_or_name", "item_id_or_name"), &UIPackage::get_bitmap_font_size);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_bitmap_font_line_height", "package_id_or_name", "item_id_or_name"), &UIPackage::get_bitmap_font_line_height);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_movie_clip_frame_count", "package_id_or_name", "item_id_or_name"), &UIPackage::get_movie_clip_frame_count);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_movie_clip_interval", "package_id_or_name", "item_id_or_name"), &UIPackage::get_movie_clip_interval);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_movie_clip_repeat_delay", "package_id_or_name", "item_id_or_name"), &UIPackage::get_movie_clip_repeat_delay);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_movie_clip_swing", "package_id_or_name", "item_id_or_name"), &UIPackage::get_movie_clip_swing);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_movie_clip_frame_texture", "package_id_or_name", "item_id_or_name", "frame_index"), &UIPackage::get_movie_clip_frame_texture);
    ClassDB::bind_static_method("UIPackage", D_METHOD("create_object", "package_id_or_name", "item_id_or_name"), &UIPackage::create_object);
    ClassDB::bind_static_method("UIPackage", D_METHOD("construct_component", "target", "package_id_or_name", "item_id_or_name"), &UIPackage::construct_component);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_image_texture_by_url", "url"), &UIPackage::get_image_texture_by_url);
    ClassDB::bind_static_method("UIPackage", D_METHOD("create_object_from_url", "url"), &UIPackage::create_object_from_url);
}

bool UIPackage::add_package(const String &p_path) {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    if (file.is_null()) {
        return false;
    }

    PackedByteArray bytes = file->get_buffer(file->get_length());
    return add_package_bytes(bytes, normalize_asset_name_prefix(p_path));
}

bool UIPackage::add_package_bytes(const PackedByteArray &p_bytes, const String &p_asset_name_prefix) {
    ERR_FAIL_COND_V_MSG(p_bytes.is_empty(), false, "Cannot add an empty FairyGUI package.");

    fgui::ByteBuffer buffer(p_bytes);
    fgui::PackageData package;
    if (!load_package(buffer, p_asset_name_prefix, package)) {
        return false;
    }

    get_packages_by_id()[package.id] = package;
    get_package_id_by_name()[package.name] = package.id;
    get_last_package_id_storage() = package.id;
    return true;
}

bool UIPackage::add_package_with_loader(const String &p_asset_name_prefix, const Callable &p_loader) {
    ERR_FAIL_COND_V_MSG(!p_loader.is_valid(), false, "Cannot add a FairyGUI package with an invalid loader.");
    const PackedByteArray bytes = load_bytes_from_callable(p_loader, package_bytes_path_from_prefix(p_asset_name_prefix));
    ERR_FAIL_COND_V_MSG(bytes.is_empty(), false, "Custom FairyGUI package loader did not return package bytes.");
    return add_package_bytes_with_loader(bytes, p_asset_name_prefix, p_loader);
}

bool UIPackage::add_package_bytes_with_loader(const PackedByteArray &p_bytes, const String &p_asset_name_prefix, const Callable &p_loader) {
    ERR_FAIL_COND_V_MSG(!p_loader.is_valid(), false, "Cannot add a FairyGUI package with an invalid loader.");
    ERR_FAIL_COND_V_MSG(p_bytes.is_empty(), false, "Cannot add an empty FairyGUI package.");

    fgui::ByteBuffer buffer(p_bytes);
    fgui::PackageData package;
    if (!load_package(buffer, p_asset_name_prefix, package)) {
        return false;
    }

    get_packages_by_id()[package.id] = package;
    get_package_id_by_name()[package.name] = package.id;
    get_custom_loaders_by_prefix()[asset_prefix_to_package_prefix(p_asset_name_prefix)] = p_loader;
    get_last_package_id_storage() = package.id;
    return true;
}

void UIPackage::clear_custom_loaders() {
    get_custom_loaders_by_prefix().clear();
}

int32_t UIPackage::get_custom_loader_count() {
    return get_custom_loaders_by_prefix().size();
}

void UIPackage::set_asset_path_override(const String &p_source_path, const String &p_mapped_path) {
    if (p_source_path.is_empty()) {
        return;
    }
    if (p_mapped_path.is_empty()) {
        get_asset_path_overrides().erase(p_source_path);
        return;
    }
    get_asset_path_overrides()[p_source_path] = p_mapped_path;
}

bool UIPackage::remove_asset_path_override(const String &p_source_path) {
    HashMap<String, String> &overrides = get_asset_path_overrides();
    if (!overrides.has(p_source_path)) {
        return false;
    }
    overrides.erase(p_source_path);
    return true;
}

void UIPackage::clear_asset_path_overrides() {
    get_asset_path_overrides().clear();
}

int32_t UIPackage::get_asset_path_override_count() {
    return get_asset_path_overrides().size();
}

String UIPackage::resolve_asset_path(const String &p_path) {
    return resolve_asset_path_internal(p_path);
}

Ref<Texture2D> UIPackage::get_resource_texture(const String &p_path) {
    const String resolved_path = resolve_asset_path_internal(p_path);
    if (resolved_path.is_empty()) {
        return Ref<Texture2D>();
    }

    HashMap<String, Ref<Texture2D>> &texture_cache = get_texture_cache();
    if (texture_cache.has(resolved_path)) {
        return texture_cache[resolved_path];
    }

    Ref<Texture2D> texture = load_texture_from_resolved_path(resolved_path);
    if (texture.is_valid()) {
        texture_cache[resolved_path] = texture;
    }
    return texture;
}

void UIPackage::clear_texture_cache() {
    get_texture_cache().clear();
}

int32_t UIPackage::get_texture_cache_count() {
    return get_texture_cache().size();
}

void UIPackage::clear_audio_cache() {
    get_audio_cache().clear();
}

int32_t UIPackage::get_audio_cache_count() {
    return get_audio_cache().size();
}

bool UIPackage::has_package(const String &p_package_id_or_name) {
    return !resolve_package_id(p_package_id_or_name).is_empty();
}

bool UIPackage::remove_package(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    HashMap<String, String> &package_id_by_name = get_package_id_by_name();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return false;
    }

    const String package_name = packages_by_id[package_id].name;
    const String loader_prefix = asset_prefix_to_package_prefix(packages_by_id[package_id].asset_name_prefix);
    packages_by_id.erase(package_id);
    if (!package_name.is_empty()) {
        package_id_by_name.erase(package_name);
    }
    get_custom_loaders_by_prefix().erase(loader_prefix);
    if (get_last_package_id_storage() == package_id) {
        get_last_package_id_storage() = String();
    }
    return true;
}

void UIPackage::remove_all_packages() {
    get_packages_by_id().clear();
    get_package_id_by_name().clear();
    get_texture_cache().clear();
    get_audio_cache().clear();
    get_custom_loaders_by_prefix().clear();
    get_last_package_id_storage() = String();
}

int32_t UIPackage::get_package_count() {
    return get_packages_by_id().size();
}

String UIPackage::get_last_package_id() {
    return get_last_package_id_storage();
}

String UIPackage::get_last_package_name() {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String &last_package_id = get_last_package_id_storage();
    if (last_package_id.is_empty() || !packages_by_id.has(last_package_id)) {
        return String();
    }
    return packages_by_id[last_package_id].name;
}

int32_t UIPackage::get_item_count(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }
    return packages_by_id[package_id].items.size();
}

int32_t UIPackage::get_sprite_count(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }
    return packages_by_id[package_id].sprites.size();
}

int32_t UIPackage::get_pixel_hit_test_item_count(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    int32_t count = 0;
    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        if (package.items[i].has_pixel_hit_test_data) {
            count++;
        }
    }
    return count;
}

int32_t UIPackage::get_scale9_image_item_count(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    int32_t count = 0;
    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        if (package.items[i].type == fgui::PackageItemType::Image && package.items[i].has_scale9_grid) {
            count++;
        }
    }
    return count;
}

int32_t UIPackage::get_bitmap_font_item_count(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    int32_t count = 0;
    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        if (package.items[i].type == fgui::PackageItemType::Font && package.items[i].bitmap_font.loaded) {
            count++;
        }
    }
    return count;
}

String UIPackage::get_first_image_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Image && package.sprites.has(item.id)) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_scale9_image_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Image && item.has_scale9_grid && package.sprites.has(item.id)) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_bitmap_font_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Font && item.bitmap_font.loaded) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_pixel_hit_test_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Image && item.has_pixel_hit_test_data && package.sprites.has(item.id)) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_component_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_button_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::Button) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_label_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::Label) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_list_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::List) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_progress_bar_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::ProgressBar) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_slider_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::Slider) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_scroll_bar_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::ScrollBar) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_combo_box_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Component && item.object_type == fgui::ObjectType::ComboBox) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_movie_clip_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::MovieClip) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_input_text_component_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (component_contains_object_type(item, fgui::ObjectType::InputText)) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_first_sound_item_name(const String &p_package_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    for (int32_t i = 0; i < package.items.size(); i++) {
        const fgui::PackageItem &item = package.items[i];
        if (item.type == fgui::PackageItemType::Sound) {
            return !item.name.is_empty() ? item.name : item.id;
        }
    }
    return String();
}

String UIPackage::get_item_url(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return String();
    }
    return String("ui://") + package_id + String("/") + p_item_id_or_name;
}

Ref<AudioStream> UIPackage::get_sound_audio_stream(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return Ref<AudioStream>();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    const fgui::PackageItem *item = find_package_item(package, p_item_id_or_name);
    if (item == nullptr || item->type != fgui::PackageItemType::Sound || item->file.is_empty()) {
        return Ref<AudioStream>();
    }

    const String resolved_path = resolve_asset_path_internal(item->file);
    HashMap<String, Ref<AudioStream>> &audio_cache = get_audio_cache();
    if (audio_cache.has(resolved_path)) {
        return audio_cache[resolved_path];
    }

    Ref<AudioStream> stream = load_audio_from_path(item->file);
    if (stream.is_valid()) {
        audio_cache[resolved_path] = stream;
    }
    return stream;
}

Ref<AudioStream> UIPackage::get_sound_audio_stream_by_url(const String &p_url) {
    String package_id_or_name;
    String item_id_or_name;
    if (!parse_item_url(p_url, package_id_or_name, item_id_or_name)) {
        return Ref<AudioStream>();
    }
    return get_sound_audio_stream(package_id_or_name, item_id_or_name);
}

Ref<Texture2D> UIPackage::get_image_texture(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return Ref<Texture2D>();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    const fgui::PackageItem *item = find_package_item(package, p_item_id_or_name);
    if (item == nullptr || item->type != fgui::PackageItemType::Image || !package.sprites.has(item->id)) {
        return Ref<Texture2D>();
    }

    const fgui::AtlasSprite &sprite = package.sprites[item->id];
    if (!package.items_by_id.has(sprite.atlas_item_id)) {
        return Ref<Texture2D>();
    }

    const fgui::PackageItem &atlas_item = package.items[package.items_by_id[sprite.atlas_item_id]];
    Ref<Texture2D> atlas_texture = get_resource_texture(atlas_item.file);
    if (atlas_texture.is_null()) {
        return Ref<Texture2D>();
    }

    Ref<AtlasTexture> atlas_region;
    atlas_region.instantiate();
    atlas_region->set_atlas(atlas_texture);
    atlas_region->set_region(sprite.rect);
    atlas_region->set_filter_clip(true);
    return atlas_region;
}

Ref<Texture2D> UIPackage::get_image_texture_by_url(const String &p_url) {
    String package_id_or_name;
    String item_id_or_name;
    if (!parse_item_url(p_url, package_id_or_name, item_id_or_name)) {
        return Ref<Texture2D>();
    }
    return get_image_texture(package_id_or_name, item_id_or_name);
}

bool UIPackage::find_bitmap_font(const String &p_font_name, fgui::BitmapFontData &r_font_data, String &r_package_id) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    String package_id_or_name;
    String item_id_or_name;
    if (parse_item_url(p_font_name, package_id_or_name, item_id_or_name)) {
        const String package_id = resolve_package_id(package_id_or_name);
        if (package_id.is_empty()) {
            return false;
        }
        const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], item_id_or_name);
        if (item == nullptr || item->type != fgui::PackageItemType::Font || !item->bitmap_font.loaded) {
            return false;
        }
        r_font_data = item->bitmap_font;
        r_package_id = package_id;
        return true;
    }

    for (const KeyValue<String, fgui::PackageData> &package_pair : packages_by_id) {
        const fgui::PackageData &package = package_pair.value;
        const fgui::PackageItem *item = find_package_item(package, p_font_name);
        if (item != nullptr && item->type == fgui::PackageItemType::Font && item->bitmap_font.loaded) {
            r_font_data = item->bitmap_font;
            r_package_id = package_pair.key;
            return true;
        }
    }
    return false;
}

Ref<Texture2D> UIPackage::get_bitmap_font_glyph_texture(const String &p_package_id_or_name, const fgui::BitmapFontGlyph &p_glyph) {
    if (p_glyph.image_item_id.is_empty()) {
        return Ref<Texture2D>();
    }
    return get_image_texture(p_package_id_or_name, p_glyph.image_item_id);
}

bool UIPackage::has_image_scale9_grid(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return false;
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Image && item->has_scale9_grid;
}

Rect2 UIPackage::get_image_scale9_grid(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return Rect2();
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Image && item->has_scale9_grid ? item->scale9_grid : Rect2();
}

bool UIPackage::has_bitmap_font(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return false;
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Font && item->bitmap_font.loaded;
}

int32_t UIPackage::get_bitmap_font_glyph_count(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Font ? item->bitmap_font.glyphs.size() : 0;
}

int32_t UIPackage::get_bitmap_font_size(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Font ? item->bitmap_font.size : 0;
}

int32_t UIPackage::get_bitmap_font_line_height(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }

    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::Font ? item->bitmap_font.line_height : 0;
}

int32_t UIPackage::get_movie_clip_frame_count(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0;
    }
    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::MovieClip ? item->movie_clip_frames.size() : 0;
}

double UIPackage::get_movie_clip_interval(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0.0;
    }
    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::MovieClip ? item->interval : 0.0;
}

double UIPackage::get_movie_clip_repeat_delay(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return 0.0;
    }
    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::MovieClip ? item->repeat_delay : 0.0;
}

bool UIPackage::get_movie_clip_swing(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return false;
    }
    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::MovieClip && item->swing;
}

Vector<fgui::MovieClipFrame> UIPackage::get_movie_clip_frames(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return Vector<fgui::MovieClipFrame>();
    }
    const fgui::PackageItem *item = find_package_item(packages_by_id[package_id], p_item_id_or_name);
    return item != nullptr && item->type == fgui::PackageItemType::MovieClip ? item->movie_clip_frames : Vector<fgui::MovieClipFrame>();
}

Ref<Texture2D> UIPackage::get_movie_clip_frame_texture(const String &p_package_id_or_name, const String &p_item_id_or_name, int32_t p_frame_index) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return Ref<Texture2D>();
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    const fgui::PackageItem *item = find_package_item(package, p_item_id_or_name);
    if (item == nullptr || item->type != fgui::PackageItemType::MovieClip || p_frame_index < 0 || p_frame_index >= item->movie_clip_frames.size()) {
        return Ref<Texture2D>();
    }

    const String sprite_id = item->movie_clip_frames[p_frame_index].sprite_id;
    if (sprite_id.is_empty() || !package.sprites.has(sprite_id)) {
        return Ref<Texture2D>();
    }

    const fgui::AtlasSprite &sprite = package.sprites[sprite_id];
    if (!package.items_by_id.has(sprite.atlas_item_id)) {
        return Ref<Texture2D>();
    }

    const fgui::PackageItem &atlas_item = package.items[package.items_by_id[sprite.atlas_item_id]];
    Ref<Texture2D> atlas_texture = get_resource_texture(atlas_item.file);
    if (atlas_texture.is_null()) {
        return Ref<Texture2D>();
    }

    Ref<AtlasTexture> atlas_region;
    atlas_region.instantiate();
    atlas_region->set_atlas(atlas_texture);
    atlas_region->set_region(sprite.rect);
    atlas_region->set_filter_clip(true);
    return atlas_region;
}

GObject *UIPackage::create_object(const String &p_package_id_or_name, const String &p_item_id_or_name) {
    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return nullptr;
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    const fgui::PackageItem *item = find_package_item(package, p_item_id_or_name);
    if (item == nullptr) {
        return nullptr;
    }

    switch (item->type) {
        case fgui::PackageItemType::Image: {
            GImage *image = memnew(GImage);
            image->set_package_item_id(item->id);
            image->set_package_image(package_id, !item->name.is_empty() ? item->name : item->id);
            if (item->has_pixel_hit_test_data) {
                image->set_pixel_hit_test_data(item->pixel_hit_test_data, item->width, item->height);
            }
            image->set_custom_minimum_size(Vector2(item->width, item->height));
            image->set_size(Vector2(item->width, item->height));
            return image;
        }
        case fgui::PackageItemType::MovieClip: {
            GMovieClip *movie_clip = memnew(GMovieClip);
            movie_clip->set_package_item_id(item->id);
            movie_clip->set_custom_minimum_size(Vector2(item->width, item->height));
            movie_clip->set_size(Vector2(item->width, item->height));
            movie_clip->set_package_movie_clip(package_id, !item->name.is_empty() ? item->name : item->id);
            return movie_clip;
        }
        case fgui::PackageItemType::Component: {
            GComponent *component = nullptr;
            if (item->object_type == fgui::ObjectType::Button) {
                component = static_cast<GComponent *>(memnew(GButton));
            } else if (item->object_type == fgui::ObjectType::Label) {
                component = static_cast<GComponent *>(memnew(GLabel));
            } else if (item->object_type == fgui::ObjectType::List) {
                component = static_cast<GComponent *>(memnew(GList));
            } else if (item->object_type == fgui::ObjectType::ProgressBar) {
                component = static_cast<GComponent *>(memnew(GProgressBar));
            } else if (item->object_type == fgui::ObjectType::Slider) {
                component = static_cast<GComponent *>(memnew(GSlider));
            } else if (item->object_type == fgui::ObjectType::ScrollBar) {
                component = static_cast<GComponent *>(memnew(GScrollBar));
            } else if (item->object_type == fgui::ObjectType::ComboBox) {
                component = static_cast<GComponent *>(memnew(GComboBox));
            } else {
                component = memnew(GComponent);
            }
            component->set_package_item_id(item->id);
            component->set_package_name(package.name);
            component->set_component_name(!item->name.is_empty() ? item->name : item->id);
            component->set_custom_minimum_size(Vector2(item->width, item->height));
            component->set_size(Vector2(item->width, item->height));
            component->construct_from_package(package_id, *item);
            return component;
        }
        default:
            return nullptr;
    }
}

bool UIPackage::construct_component(GComponent *p_target, const String &p_package_id_or_name, const String &p_item_id_or_name) {
    ERR_FAIL_NULL_V(p_target, false);

    HashMap<String, fgui::PackageData> &packages_by_id = get_packages_by_id();
    const String package_id = resolve_package_id(p_package_id_or_name);
    if (package_id.is_empty()) {
        return false;
    }

    const fgui::PackageData &package = packages_by_id[package_id];
    const fgui::PackageItem *item = find_package_item(package, p_item_id_or_name);
    if (item == nullptr || item->type != fgui::PackageItemType::Component) {
        return false;
    }

    p_target->set_package_item_id(item->id);
    return p_target->construct_from_package(package_id, *item);
}

GObject *UIPackage::create_object_from_url(const String &p_url) {
    String package_id_or_name;
    String item_id_or_name;
    if (!parse_item_url(p_url, package_id_or_name, item_id_or_name)) {
        return nullptr;
    }
    return create_object(package_id_or_name, item_id_or_name);
}

bool UIPackage::load_package(fgui::ByteBuffer &p_buffer, const String &p_asset_name_prefix, fgui::PackageData &r_package) {
    ERR_FAIL_COND_V_MSG(p_buffer.read_uint() != FGUI_PACKAGE_MAGIC, false, "Invalid or unsupported FairyGUI package magic.");

    p_buffer.version = p_buffer.read_int();
    const bool version_2_or_later = p_buffer.version >= 2;
    p_buffer.read_bool(); // compressed
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
            p_buffer.read_s(); // dependency id
            p_buffer.read_s(); // dependency name
        }

        if (version_2_or_later) {
            count = p_buffer.read_short();
            if (count > 0) {
                p_buffer.read_s_array(count); // branches
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
            p_buffer.read_s(); // path
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
                    p_buffer.read_bool(); // smoothing
                    break;
                }
                case fgui::PackageItemType::MovieClip: {
                    p_buffer.read_bool(); // smoothing
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
                    p_buffer.read_float(); // skeleton anchor x
                    p_buffer.read_float(); // skeleton anchor y
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

    return true;
}
