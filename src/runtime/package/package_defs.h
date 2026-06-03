#pragma once

#include "byte_buffer.h"

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/rect2.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot::fgui {

enum class PackageItemType : uint8_t {
    Image = 0,
    MovieClip = 1,
    Sound = 2,
    Component = 3,
    Atlas = 4,
    Font = 5,
    Swf = 6,
    Misc = 7,
    Unknown = 8,
    Spine = 9,
    DragonBones = 10,
};

enum class ObjectType : uint8_t {
    Image = 0,
    MovieClip = 1,
    Swf = 2,
    Graph = 3,
    Loader = 4,
    Group = 5,
    Text = 6,
    RichText = 7,
    InputText = 8,
    Component = 9,
    List = 10,
    Label = 11,
    Button = 12,
    ComboBox = 13,
    ProgressBar = 14,
    Slider = 15,
    ScrollBar = 16,
    Tree = 17,
    Loader3D = 18,
};

enum class RelationType : uint8_t {
    Left_Left = 0,
    Left_Center = 1,
    Left_Right = 2,
    Center_Center = 3,
    Right_Left = 4,
    Right_Center = 5,
    Right_Right = 6,
    Top_Top = 7,
    Top_Middle = 8,
    Top_Bottom = 9,
    Middle_Middle = 10,
    Bottom_Top = 11,
    Bottom_Middle = 12,
    Bottom_Bottom = 13,
    Width = 14,
    Height = 15,
    LeftExt_Left = 16,
    LeftExt_Right = 17,
    RightExt_Left = 18,
    RightExt_Right = 19,
    TopExt_Top = 20,
    TopExt_Bottom = 21,
    BottomExt_Top = 22,
    BottomExt_Bottom = 23,
    Size = 24,
};

enum class ListLayoutType : uint8_t {
    SingleColumn = 0,
    SingleRow = 1,
    FlowHorizontal = 2,
    FlowVertical = 3,
    Pagination = 4,
};

enum class ListSelectionMode : uint8_t {
    Single = 0,
    Multiple = 1,
    MultipleSingleClick = 2,
    None = 3,
};

enum class ScrollType : uint8_t {
    Horizontal = 0,
    Vertical = 1,
    Both = 2,
};

enum class ScrollBarDisplayType : uint8_t {
    Default = 0,
    Visible = 1,
    Auto = 2,
    Hidden = 3,
};

enum class ProgressTitleType : uint8_t {
    Percent = 0,
    ValueAndMax = 1,
    Value = 2,
    Max = 3,
};

struct PixelHitTestData {
    int32_t pixel_width = 0;
    float scale = 1.0f;
    PackedByteArray pixels;

    void load(ByteBuffer &p_buffer);
    bool hit_test(const Vector2 &p_local_point, const Rect2 &p_content_rect, int32_t p_offset_x, int32_t p_offset_y, float p_source_width, float p_source_height) const;
};

struct MovieClipFrame {
    Rect2 rect;
    float add_delay = 0.0f;
    String sprite_id;
};

struct BitmapFontGlyph {
    int32_t char_code = 0;
    String image_item_id;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    int32_t advance = 0;
    int32_t line_height = 0;
    int32_t channel = 0;
};

struct BitmapFontData {
    bool loaded = false;
    bool ttf = false;
    bool can_tint = true;
    bool resizable = false;
    bool has_channel = false;
    int32_t size = 0;
    int32_t xadvance = 0;
    int32_t line_height = 0;
    HashMap<int32_t, BitmapFontGlyph> glyphs;
};

struct PackageItem {
    PackageItemType type = PackageItemType::Unknown;
    ObjectType object_type = ObjectType::Image;

    String id;
    String name;
    String file;
    bool exported = false;
    int32_t width = 0;
    int32_t height = 0;

    bool has_scale9_grid = false;
    Rect2 scale9_grid;
    bool scale_by_tile = false;
    int32_t tile_grid_indice = 0;

    ByteBuffer raw_data;
    Vector<String> branches;
    Vector<String> high_resolution;

    bool has_pixel_hit_test_data = false;
    PixelHitTestData pixel_hit_test_data;

    float interval = 0.1f;
    float repeat_delay = 0.0f;
    bool swing = false;
    Vector<MovieClipFrame> movie_clip_frames;

    BitmapFontData bitmap_font;
};

struct AtlasSprite {
    String atlas_item_id;
    Rect2 rect;
    bool rotated = false;
    Vector2 offset;
    Vector2 original_size;
};

struct PackageData {
    String id;
    String name;
    String asset_name_prefix;
    int32_t version = 0;
    Vector<PackageItem> items;
    HashMap<String, int32_t> items_by_id;
    HashMap<String, int32_t> items_by_name;
    HashMap<String, AtlasSprite> sprites;
};

} // namespace godot::fgui
