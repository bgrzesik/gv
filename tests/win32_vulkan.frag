#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstants {
    mat4 mvp;
    float time;
} pc;

layout (set = 0, binding = 0) uniform sampler2D img;

layout (location = 0) in vec4 color;
layout (location = 1) in vec2 tex_coord;

layout (location = 0) out vec4 out_color;

void main() {
   out_color = color * texture(img, tex_coord) * abs(sin(pc.time));
}