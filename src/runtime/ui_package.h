#pragma once

#include "package/package_defs.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GObject;
class GComponent;

class UIPackage : public Object {
    GDCLASS(UIPackage, Object)

protected:
    static void _bind_methods();

public:
    static bool add_package(const String &p_path);
    static bool add_package_bytes(const PackedByteArray &p_bytes, const String &p_asset_name_prefix);
    static bool add_package_with_loader(const String &p_asset_name_prefix, const Callable &p_loader);
    static bool add_package_bytes_with_loader(const PackedByteArray &p_bytes, const String &p_asset_name_prefix, const Callable &p_loader);
    static void clear_custom_loaders();
    static int32_t get_custom_loader_count();
    static void set_asset_path_override(const String &p_source_path, const String &p_mapped_path);
    static bool remove_asset_path_override(const String &p_source_path);
    static void clear_asset_path_overrides();
    static int32_t get_asset_path_override_count();
    static String resolve_asset_path(const String &p_path);
    static Ref<Texture2D> get_resource_texture(const String &p_path);
    static void clear_texture_cache();
    static int32_t get_texture_cache_count();
    static void clear_audio_cache();
    static int32_t get_audio_cache_count();
    static bool has_package(const String &p_package_id_or_name);
    static bool remove_package(const String &p_package_id_or_name);
    static void remove_all_packages();
    static int32_t get_package_count();
    static String get_last_package_id();
    static String get_last_package_name();
    static int32_t get_item_count(const String &p_package_id_or_name);
    static int32_t get_sprite_count(const String &p_package_id_or_name);
    static int32_t get_pixel_hit_test_item_count(const String &p_package_id_or_name);
    static int32_t get_scale9_image_item_count(const String &p_package_id_or_name);
    static int32_t get_bitmap_font_item_count(const String &p_package_id_or_name);
    static String get_first_image_item_name(const String &p_package_id_or_name);
    static String get_first_scale9_image_item_name(const String &p_package_id_or_name);
    static String get_first_bitmap_font_item_name(const String &p_package_id_or_name);
    static String get_first_pixel_hit_test_item_name(const String &p_package_id_or_name);
    static String get_first_component_item_name(const String &p_package_id_or_name);
    static String get_first_button_item_name(const String &p_package_id_or_name);
    static String get_first_label_item_name(const String &p_package_id_or_name);
    static String get_first_list_item_name(const String &p_package_id_or_name);
    static String get_first_progress_bar_item_name(const String &p_package_id_or_name);
    static String get_first_slider_item_name(const String &p_package_id_or_name);
    static String get_first_scroll_bar_item_name(const String &p_package_id_or_name);
    static String get_first_combo_box_item_name(const String &p_package_id_or_name);
    static String get_first_movie_clip_item_name(const String &p_package_id_or_name);
    static String get_first_input_text_component_item_name(const String &p_package_id_or_name);
    static String get_first_sound_item_name(const String &p_package_id_or_name);
    static String get_item_url(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Ref<AudioStream> get_sound_audio_stream(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Ref<AudioStream> get_sound_audio_stream_by_url(const String &p_url);
    static Ref<Texture2D> get_image_texture(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static bool has_image_scale9_grid(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Rect2 get_image_scale9_grid(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static bool has_bitmap_font(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static int32_t get_bitmap_font_glyph_count(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static int32_t get_bitmap_font_size(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static int32_t get_bitmap_font_line_height(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static int32_t get_movie_clip_frame_count(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static double get_movie_clip_interval(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static double get_movie_clip_repeat_delay(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static bool get_movie_clip_swing(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Vector<fgui::MovieClipFrame> get_movie_clip_frames(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Ref<Texture2D> get_movie_clip_frame_texture(const String &p_package_id_or_name, const String &p_item_id_or_name, int32_t p_frame_index);
    static GObject *create_object(const String &p_package_id_or_name, const String &p_item_id_or_name);
    static bool construct_component(GComponent *p_target, const String &p_package_id_or_name, const String &p_item_id_or_name);
    static Ref<Texture2D> get_image_texture_by_url(const String &p_url);
    static GObject *create_object_from_url(const String &p_url);
    static bool find_bitmap_font(const String &p_font_name, fgui::BitmapFontData &r_font_data, String &r_package_id);
    static Ref<Texture2D> get_bitmap_font_glyph_texture(const String &p_package_id_or_name, const fgui::BitmapFontGlyph &p_glyph);

    static void register_font_resource(const String &p_font_name, const String &p_resource_path);
    static bool unregister_font_resource(const String &p_font_name);
    static Ref<Font> get_true_type_font(const String &p_font_name, int32_t p_font_size);
    static String resolve_asset_path_for_image_ubb(const String &p_image_url);

private:
    static bool load_package(fgui::ByteBuffer &p_buffer, const String &p_asset_name_prefix, fgui::PackageData &r_package);
};

} // namespace godot
