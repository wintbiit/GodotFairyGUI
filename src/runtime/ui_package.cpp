#include "ui_package.h"
#include "package/ui_package_internal.h"

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
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/system_font.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {
namespace fgui_internal {

// All public static methods on UIPackage are designed for main-thread use.
// Godot's scene tree / ClassDB / ObjectDB are not thread-safe — do NOT
// call UIPackage methods from background threads.  For async asset loading,
// dispatch back to the main thread via call_deferred().

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

} // namespace fgui_internal
} // namespace godot

using namespace godot;
using namespace godot::fgui_internal;

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
    ClassDB::bind_static_method("UIPackage", D_METHOD("register_font_resource", "font_name", "resource_path"), &UIPackage::register_font_resource);
    ClassDB::bind_static_method("UIPackage", D_METHOD("unregister_font_resource", "font_name"), &UIPackage::unregister_font_resource);
    ClassDB::bind_static_method("UIPackage", D_METHOD("get_true_type_font", "font_name", "font_size"), &UIPackage::get_true_type_font);
    ClassDB::bind_static_method("UIPackage", D_METHOD("resolve_asset_path_for_image_ubb", "image_url"), &UIPackage::resolve_asset_path_for_image_ubb);
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
    // Texture/audio caches are intentionally NOT cleared here:
    // individual cache entries may be shared across packages via the same asset path.
    // Use clear_texture_cache() / clear_audio_cache() to free resources explicitly.
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

// ── TrueType font resource mapping ──────────────────────────────────────────

namespace {
HashMap<String, String> &_font_resource_map() {
    static HashMap<String, String> map;
    return map;
}

HashMap<String, Ref<Font>> &_font_cache() {
    static HashMap<String, Ref<Font>> cache;
    return cache;
}
} // namespace

void UIPackage::register_font_resource(const String &p_font_name, const String &p_resource_path) {
    _font_resource_map()[p_font_name] = p_resource_path;
    _font_cache().erase(p_font_name);
}

bool UIPackage::unregister_font_resource(const String &p_font_name) {
    _font_cache().erase(p_font_name);
    return _font_resource_map().erase(p_font_name);
}

Ref<Font> UIPackage::get_true_type_font(const String &p_font_name, int32_t p_font_size) {
    const String cache_key = p_font_name + String(":") + itos(p_font_size);
    HashMap<String, Ref<Font>> &font_cache = _font_cache();
    {
        HashMap<String, Ref<Font>>::Iterator C = font_cache.find(cache_key);
        if (C) {
            return C->value;
        }
    }

    {
        HashMap<String, String> &font_map = _font_resource_map();
        HashMap<String, String>::Iterator F = font_map.find(p_font_name);
        if (F) {
            Ref<FontFile> font_file = ResourceLoader::get_singleton()->load(F->value);
            if (font_file.is_valid()) {
                font_file->set_fixed_size(p_font_size > 0 ? p_font_size : 14);
                font_cache[cache_key] = font_file;
                return font_file;
            }
            WARN_PRINT(vformat("UIPackage: TrueType font '%s' failed to load from '%s'", p_font_name, F->value));
        }
    }

    {
        Ref<SystemFont> sys_font;
        sys_font.instantiate();
        PackedStringArray names;
        names.push_back(p_font_name);
        sys_font->set_font_names(names);
        font_cache[cache_key] = sys_font;
        return sys_font;
    }

    return Ref<Font>();
}

String UIPackage::resolve_asset_path_for_image_ubb(const String &p_image_url) {
    String package_id;
    String item_id;
    if (!parse_item_url(p_image_url, package_id, item_id)) {
        return String();
    }
    Ref<Texture2D> texture = get_image_texture(package_id, item_id);
    if (texture.is_valid() && texture->get_path().begins_with("res://")) {
        return texture->get_path();
    }
    return String();
}
