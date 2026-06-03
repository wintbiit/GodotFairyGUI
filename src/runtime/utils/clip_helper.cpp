#include "clip_helper.h"

#include <godot_cpp/classes/canvas_item.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>

using namespace godot;

namespace {

Ref<Shader> s_clip_shader;

} // namespace

Ref<ShaderMaterial> ClipHelper::create_clip_material() {
    if (s_clip_shader.is_null()) {
        s_clip_shader.instantiate();
    }
    Ref<ShaderMaterial> mat;
    mat.instantiate();
    mat->set_shader(s_clip_shader);
    return mat;
}

void ClipHelper::apply_rect_clip(Control *p_control, const Rect2 &p_clip_rect, float p_softness) {
    if (p_control == nullptr) return;

    if (p_softness <= 0.0f) {
        p_control->set_clip_contents(true);
        return;
    }

    Ref<ShaderMaterial> shader_mat;
    Ref<Material> cur_material = p_control->get_material();
    if (cur_material.is_valid()) {
        shader_mat = Ref<ShaderMaterial>(Object::cast_to<ShaderMaterial>(cur_material.ptr()));
    }
    if (shader_mat.is_null()) {
        shader_mat = create_clip_material();
    }

    shader_mat->set_shader_parameter("clip_rect", Vector4(p_clip_rect.position.x, p_clip_rect.position.y, p_clip_rect.position.x + p_clip_rect.size.x, p_clip_rect.position.y + p_clip_rect.size.y));
    shader_mat->set_shader_parameter("clip_softness", p_softness);
    p_control->set_material(shader_mat);
    p_control->set_clip_contents(true);
}
