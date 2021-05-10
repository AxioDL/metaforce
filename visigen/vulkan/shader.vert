#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;

layout(binding=0) uniform UniformBlock
{
    mat4 projectionMatrix;
    mat4 modelViewMatrix;
};

struct VertToFrag
{
    vec4 color;
};

layout(location=0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(posIn.xyz, 1.0);
}
