#shader test

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

#vertex hlsl
struct VertData
{
    float3 in_pos : POSITION;
    float3 in_norm : NORMAL;
    float2 in_uv : UV;
};
struct VertToFrag
{
    float4 position : SV_Position;
    float2 out_uv : UV;
};
VertToFrag main(in VertData v)
{
    VertToFrag ret;
    ret.position = float4(v.in_pos, 1.0);
    ret.out_uv = v.in_uv;
    return ret;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float2 out_uv : UV;
};
Texture2D texs : register(t0);
SamplerState samp : register(s0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    return texs.Sample(samp, vtf.out_uv);
}

#vertex metal
struct VertData
{
    float3 in_pos [[ attribute(0) ]];
    float3 in_norm [[ attribute(1) ]];
    float2 in_uv [[ attribute(2) ]];
};
struct VertToFrag
{
    float4 position [[ position ]];
    float2 out_uv;
};
vertex VertToFrag vmain(VertData v [[ stage_in ]])
{
    VertToFrag ret;
    ret.position = float4(v.in_pos, 1.0);
    ret.out_uv = v.in_uv;
    return ret;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 out_uv;
};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    return tex.sample(samp, vtf.out_uv);
}
