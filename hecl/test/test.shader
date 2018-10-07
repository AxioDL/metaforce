#culling none
#attribute position3
#attribute normal3
#attribute uv2

#vertex glsl
layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_norm;
layout(location=2) in vec2 in_uv;
SBINDING(0) out vec2 out_uv;
void main()
{
    gl_Position = vec4(in_pos, 1.0);
    out_uv = in_uv;
}

#fragment glsl
precision highp float;
TBINDING0 uniform sampler2D texs[1];
layout(location=0) out vec4 out_frag;
SBINDING(0) in vec2 out_uv;
void main()
{
    out_frag = texture(texs[0], out_uv);
}
