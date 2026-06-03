#include "byte_buffer.h"

#include <godot_cpp/core/error_macros.hpp>

#include <cstring>

namespace godot::fgui {

ByteBuffer::ByteBuffer(const PackedByteArray &p_data) :
        data(p_data),
        length(p_data.size()) {
}

ByteBuffer::ByteBuffer(const PackedByteArray &p_data, int64_t p_offset, int64_t p_length) :
        data(p_data),
        offset(p_offset),
        length(p_length) {
    ERR_FAIL_COND_MSG(offset < 0 || length < 0 || offset + length > data.size(), "Invalid ByteBuffer slice.");
}

int64_t ByteBuffer::get_position() const {
    return position;
}

void ByteBuffer::set_position(int64_t p_position) {
    ERR_FAIL_COND_MSG(p_position < 0 || p_position > length, "ByteBuffer position out of range.");
    position = p_position;
}

int64_t ByteBuffer::get_length() const {
    return length;
}

bool ByteBuffer::has_available() const {
    return position < length;
}

void ByteBuffer::skip(int64_t p_count) {
    require(p_count);
    position += p_count;
}

uint8_t ByteBuffer::read_byte() {
    require(1);
    return data[offset + position++];
}

bool ByteBuffer::read_bool() {
    return read_byte() == 1;
}

int16_t ByteBuffer::read_short() {
    require(2);
    const int64_t start = offset + position;
    position += 2;
    if (little_endian) {
        return static_cast<int16_t>(data[start] | (data[start + 1] << 8));
    }
    return static_cast<int16_t>((data[start] << 8) | data[start + 1]);
}

uint16_t ByteBuffer::read_ushort() {
    return static_cast<uint16_t>(read_short());
}

int32_t ByteBuffer::read_int() {
    require(4);
    const int64_t start = offset + position;
    position += 4;
    if (little_endian) {
        return static_cast<int32_t>(data[start] | (data[start + 1] << 8) | (data[start + 2] << 16) | (data[start + 3] << 24));
    }
    return static_cast<int32_t>((data[start] << 24) | (data[start + 1] << 16) | (data[start + 2] << 8) | data[start + 3]);
}

uint32_t ByteBuffer::read_uint() {
    return static_cast<uint32_t>(read_int());
}

float ByteBuffer::read_float() {
    uint32_t bits = read_uint();
    float value = 0.0f;
    std::memcpy(&value, &bits, sizeof(float));
    return value;
}

Color ByteBuffer::read_color() {
    require(4);
    const int64_t start = offset + position;
    position += 4;
    return Color(static_cast<float>(data[start]) / 255.0f,
            static_cast<float>(data[start + 1]) / 255.0f,
            static_cast<float>(data[start + 2]) / 255.0f,
            static_cast<float>(data[start + 3]) / 255.0f);
}

String ByteBuffer::read_string() {
    return read_string(read_ushort());
}

String ByteBuffer::read_string(int64_t p_length) {
    require(p_length);
    const char *ptr = reinterpret_cast<const char *>(data.ptr()) + offset + position;
    String result = String::utf8(ptr, p_length);
    position += p_length;
    return result;
}

String ByteBuffer::read_s() {
    const uint16_t index = read_ushort();
    if (index == 65535 || index == 65534) {
        return String();
    }
    if (index == 65533) {
        return String("");
    }
    if (index >= string_table.size()) {
        return String();
    }
    return string_table[index];
}

Vector<String> ByteBuffer::read_s_array(int32_t p_count) {
    Vector<String> result;
    result.resize(p_count);
    for (int32_t i = 0; i < p_count; i++) {
        result.set(i, read_s());
    }
    return result;
}

ByteBuffer ByteBuffer::read_buffer() {
    const int32_t count = read_int();
    ByteBuffer buffer(data, offset + position, count);
    buffer.string_table = string_table;
    buffer.version = version;
    position += count;
    return buffer;
}

bool ByteBuffer::seek(int64_t p_index_table_pos, int32_t p_block_index) {
    const int64_t old_position = position;
    position = p_index_table_pos;
    const int32_t segment_count = data[offset + position++];

    if (p_block_index >= segment_count) {
        position = old_position;
        return false;
    }

    const bool use_short = data[offset + position++] == 1;
    int32_t new_position = 0;
    if (use_short) {
        position += 2 * p_block_index;
        new_position = read_short();
    } else {
        position += 4 * p_block_index;
        new_position = read_int();
    }

    if (new_position > 0 && new_position <= length - p_index_table_pos) {
        position = p_index_table_pos + new_position;
        return true;
    }

    position = old_position;
    return false;
}

void ByteBuffer::require(int64_t p_count) const {
    ERR_FAIL_COND_MSG(p_count < 0 || position + p_count > length, "ByteBuffer read out of range.");
}

} // namespace godot::fgui
