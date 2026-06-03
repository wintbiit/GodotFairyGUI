#pragma once

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/string.hpp>

#include "package/package_defs.h"

namespace godot {

namespace fgui_internal {

constexpr uint32_t FGUI_PACKAGE_MAGIC = 0x46475549;

HashMap<String, fgui::PackageData> &get_packages_by_id();
HashMap<String, String> &get_package_id_by_name();
HashMap<String, String> &get_asset_path_overrides();
HashMap<String, Ref<Texture2D>> &get_texture_cache();
HashMap<String, Ref<AudioStream>> &get_audio_cache();
HashMap<String, Callable> &get_custom_loaders_by_prefix();
String &get_last_package_id_storage();

String asset_prefix_to_package_prefix(const String &p_asset_name_prefix);

String resolve_package_id(const String &p_package_id_or_name);
bool parse_item_url(const String &p_url, String &r_package, String &r_item);
const fgui::PackageItem *find_package_item(const fgui::PackageData &p_package, const String &p_item_id_or_name);
bool component_contains_object_type(const fgui::PackageItem &p_item, fgui::ObjectType p_type);
String resolve_asset_path_internal(const String &p_path);
String find_best_loader_prefix(const String &p_path);
Variant call_custom_loader(const String &p_path);
Ref<Texture2D> texture_from_bytes(const String &p_path, const PackedByteArray &p_bytes);
Ref<Texture2D> load_texture_from_custom_loader(const String &p_path);
Ref<AudioStream> audio_from_bytes(const String &p_path, const PackedByteArray &p_bytes);
Ref<AudioStream> load_audio_from_custom_loader(const String &p_path);
Ref<AudioStream> load_audio_from_path(const String &p_path);
PackedByteArray load_bytes_from_custom_loader(const String &p_path);
PackedByteArray load_bytes_from_callable(const Callable &p_loader, const String &p_path);
Ref<Texture2D> load_texture_from_resolved_path(const String &p_resolved_path);

void parse_movie_clip_data(fgui::PackageItem &p_item);
void parse_bitmap_font_data(fgui::PackageData &p_package, fgui::PackageItem &p_item);

} // namespace fgui_internal

} // namespace godot
