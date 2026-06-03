#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/variant/rect2.hpp>

namespace godot {

class Control;
class ShaderMaterial;

class ClipHelper {
public:
    static Ref<ShaderMaterial> create_clip_material();
    static void apply_rect_clip(Control *p_control, const Rect2 &p_clip_rect, float p_softness = 0.0f);
};

} // namespace godot
