#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    float time;
} pc;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex_coord;
layout (location = 2) in vec4 color;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec2 out_tex_coord;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() {
    out_color = color;
	out_tex_coord = tex_coord;
    gl_Position = pc.mvp * pos;
}
