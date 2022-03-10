#version 450
#extension GL_ARB_separate_shader_objects : enable

#define lightCount 6

layout(location = 0) in vec3 inPos; 
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main() {
	outColor = inColor;
    gl_Position = vec4(inPos, 1.0f);
}