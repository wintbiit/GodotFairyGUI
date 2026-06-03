shader_type canvas_item;

uniform sampler2D mask_texture : filter_linear, repeat_disable;
uniform vec4 mask_uv = vec4(0.0, 0.0, 1.0, 1.0);

void fragment() {
	vec2 local_uv = mix(mask_uv.xy, mask_uv.xy + mask_uv.zw, UV);
	float mask_alpha = texture(mask_texture, local_uv).a;
	COLOR.a *= mask_alpha;
}
