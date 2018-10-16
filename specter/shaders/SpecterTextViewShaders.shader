#include "SpecterCommon.shader"

#shader SpecterTextViewShader
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute modelview 0
#instattribute modelview 1
#instattribute modelview 2
#instattribute modelview 3
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#instattribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec3 posIn[4];
layout(location=4) in mat4 mvMtx;
layout(location=8) in vec3 uvIn[4];
layout(location=12) in vec4 colorIn;
SPECTER_GLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    vec3 uv;
    vec4 color;
};
SBINDING(0) out VertToFrag vtf;
void main()
{
    vec3 pos = posIn[gl_VertexID];
    vtf.uv = uvIn[gl_VertexID];
    vtf.color = colorIn * mulColor;
    gl_Position = mv * mvMtx * vec4(pos, 1.0);
    gl_Position = FLIPFROMGL(gl_Position);
}

#fragment glsl
TBINDING0 uniform sampler2DArray fontTex;
struct VertToFrag
{
    vec3 uv;
    vec4 color;
};
SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vtf.color;
    colorOut.a *= texture(fontTex, vtf.uv).r;
}

#vertex hlsl
struct VertData
{
    float3 posIn[4] : POSITION;
    float4x4 mvMtx : MODELVIEW;
    float3 uvIn[4] : UV;
    float4 colorIn : COLOR;
};
SPECTER_HLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position : SV_Position;
    float3 uv : UV;
    float4 color : COLOR;
};
VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.uv = v.uvIn[vertId];
    vtf.color = v.colorIn * mulColor;
    vtf.position = mul(mv, mul(v.mvMtx, float4(v.posIn[vertId], 1.0)));
    return vtf;
}

#fragment hlsl
Texture2DArray fontTex : register(t0);
SamplerState samp : register(s0);
struct VertToFrag
{
    float4 position : SV_Position;
    float3 uv : UV;
    float4 color : COLOR;
};
float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 colorOut = vtf.color;
    colorOut.a *= fontTex.Sample(samp, vtf.uv).r;
    return colorOut;
}

#vertex metal
struct VertData
{
    float3 posIn[4];
    float4x4 mvMtx;
    float3 uvIn[4];
    float4 colorIn;
};
SPECTER_METAL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position [[ position ]];
    float3 uv;
    float4 color;
};
vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant SpecterViewBlock& view [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.uv = v.uvIn[vertId];
    vtf.color = v.colorIn * view.mulColor;
    vtf.position = view.mv * v.mvMtx * float4(v.posIn[vertId], 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float3 uv;
    float4 color;
};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d_array<float> fontTex [[ texture(0) ]])
{
    float4 colorOut = vtf.color;
    colorOut.a *= fontTex.sample(samp, vtf.uv.xy, vtf.uv.z).r;
    return colorOut;
}

#shader SpecterTextViewShaderSubpixel : SpecterTextViewShader
#srcfac srccolor1
#dstfac invsrccolor1

#fragment glsl
TBINDING0 uniform sampler2DArray fontTex;
struct VertToFrag
{
    vec3 uv;
    vec4 color;
};
SBINDING(0) in VertToFrag vtf;
layout(location=0, index=0) out vec4 colorOut;
layout(location=0, index=1) out vec4 blendOut;
void main()
{
    colorOut = vtf.color;
    blendOut = colorOut.a * texture(fontTex, vtf.uv);
}

#fragment hlsl
Texture2DArray fontTex : register(t0);
SamplerState samp : register(s0);
struct VertToFrag
{
    float4 position : SV_Position;
    float3 uv : UV;
    float4 color : COLOR;
};
struct BlendOut
{
    float4 colorOut : SV_Target0;
    float4 blendOut : SV_Target1;
};
BlendOut main(in VertToFrag vtf)
{
    BlendOut ret;
    ret.colorOut = vtf.color;
    ret.blendOut = ret.colorOut.a * fontTex.Sample(samp, vtf.uv);
    return ret;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float3 uv;
    float4 color;
};
struct BlendOut
{
    float4 colorOut [[ color(0), index(0) ]];
    float4 blendOut [[ color(0), index(1) ]];
};
fragment BlendOut fmain(VertToFrag vtf [[ stage_in ]],
                        sampler samp [[ sampler(0) ]],
                        texture2d_array<float> fontTex [[ texture(0) ]])
{
    BlendOut ret;
    ret.colorOut = vtf.color;
    ret.blendOut = ret.colorOut.a * fontTex.sample(samp, vtf.uv.xy, vtf.uv.z);
    return ret;
}
