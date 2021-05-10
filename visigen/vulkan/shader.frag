#version 450
#extension GL_ARB_separate_shader_objects : enable

struct VertToFrag
{
    vec4 color;
};

layout(location=0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vtf.color;
}
