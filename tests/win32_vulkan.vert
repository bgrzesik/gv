#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform bufferVals {
    mat4 mvp;
} myBufferVals;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec2 outTexCoord;

out gl_PerVertex { 
    vec4 gl_Position;
};

void main() {
    outColor = color;
	outTexCoord = texCoord;
    gl_Position = myBufferVals.mvp * pos;
}
