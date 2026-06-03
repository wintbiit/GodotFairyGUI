#include "package_defs.h"

#include <godot_cpp/core/math.hpp>

namespace godot::fgui {

void PixelHitTestData::load(ByteBuffer &p_buffer) {
    p_buffer.read_int();
    pixel_width = p_buffer.read_int();
    const uint8_t denominator = p_buffer.read_byte();
    scale = denominator == 0 ? 1.0f : 1.0f / static_cast<float>(denominator);
    const int32_t pixels_length = p_buffer.read_int();
    pixels.resize(pixels_length);
    for (int32_t i = 0; i < pixels_length; i++) {
        pixels.set(i, p_buffer.read_byte());
    }
}

bool PixelHitTestData::hit_test(const Vector2 &p_local_point, const Rect2 &p_content_rect, int32_t p_offset_x, int32_t p_offset_y, float p_source_width, float p_source_height) const {
    if (!p_content_rect.has_point(p_local_point) || pixel_width <= 0 || pixels.is_empty()) {
        return false;
    }

    const int32_t x = Math::floor((p_local_point.x * p_source_width / p_content_rect.size.x - p_offset_x) * scale);
    const int32_t y = Math::floor((p_local_point.y * p_source_height / p_content_rect.size.y - p_offset_y) * scale);
    if (x < 0 || y < 0 || x >= pixel_width) {
        return false;
    }

    const int32_t pos = y * pixel_width + x;
    const int32_t byte_index = pos / 8;
    const int32_t bit_index = pos % 8;
    if (byte_index < 0 || byte_index >= pixels.size()) {
        return false;
    }

    return ((pixels[byte_index] >> bit_index) & 0x1) > 0;
}

} // namespace godot::fgui
