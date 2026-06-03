#pragma once

#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot::fgui {

class ByteBuffer {
public:
    ByteBuffer() = default;
    explicit ByteBuffer(const PackedByteArray &p_data);
    ByteBuffer(const PackedByteArray &p_data, int64_t p_offset, int64_t p_length);

    int32_t version = 0;
    bool little_endian = false;
    Vector<String> string_table;

    int64_t get_position() const;
    void set_position(int64_t p_position);
    int64_t get_length() const;
    bool has_available() const;

    void skip(int64_t p_count);
    uint8_t read_byte();
    bool read_bool();
    int16_t read_short();
    uint16_t read_ushort();
    int32_t read_int();
    uint32_t read_uint();
    float read_float();
    Color read_color();
    String read_string();
    String read_string(int64_t p_length);
    String read_s();
    Vector<String> read_s_array(int32_t p_count);
    ByteBuffer read_buffer();
    bool seek(int64_t p_index_table_pos, int32_t p_block_index);

private:
    PackedByteArray data;
    int64_t offset = 0;
    int64_t length = 0;
    int64_t position = 0;

    void require(int64_t p_count) const;
};

} // namespace godot::fgui
