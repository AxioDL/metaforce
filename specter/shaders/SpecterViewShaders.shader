#include "SpecterCommon.shader"

#shader SpecterViewShaderSolid
#attribute position4
#attribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec3 posIn;
layout(location=1) in vec4 colorIn;
SPECTER_GLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    vec4 color;
};
SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn * mulColor;
    gl_Position = mv * vec4(posIn, 1.0);
    gl_Position = FLIPFROMGL(gl_Position);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
};
SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vtf.color;
}

#vertex hlsl
struct VertData
{
    float3 posIn : POSITION;
    float4 colorIn : COLOR;
};
SPECTER_HLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};
VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * mulColor;
    vtf.position = mul(mv, float4(v.posIn, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};
float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color;
}

#vertex metal
struct VertData
{
    float3 posIn [[ attribute(0) ]];
    float4 colorIn [[ attribute(1) ]];
};
SPECTER_METAL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};
vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpecterViewBlock& view [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = v.colorIn * view.mulColor;
    vtf.position = view.mv * float4(v.posIn, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]])
{
    return vtf.color;
}

#shader SpecterViewShaderTex
#attribute position4
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec3 posIn;
layout(location=1) in vec2 uvIn;
SPECTER_GLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    vec4 color;
    vec4 uv;
};
SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uv.xy = uvIn;
    vtf.color = mulColor;
    gl_Position = mv * vec4(posIn, 1.0);
    gl_Position = FLIPFROMGL(gl_Position);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec4 uv;
};
SBINDING(0) in VertToFrag vtf;
TBINDING0 uniform sampler2D tex;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = texture(tex, vtf.uv.xy) * vtf.color;
}

#vertex hlsl
struct VertData
{
    float3 posIn : POSITION;
    float2 uvIn : UV;
};
SPECTER_HLSL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};
VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.uv = v.uvIn;
    vtf.color = mulColor;
    vtf.position = mul(mv, float4(v.posIn, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};
Texture2D tex : register(t0);
SamplerState samp : register(s0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    return tex.Sample(samp, vtf.uv) * vtf.color;
}

#vertex metal
struct VertData
{
    float3 posIn [[ attribute(0) ]];
    float2 uvIn [[ attribute(1) ]];
};
SPECTER_METAL_VIEW_VERT_BLOCK
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};
vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpecterViewBlock& view [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.uv = v.uvIn;
    vtf.color = view.mulColor;
    vtf.position = view.mv * float4(v.posIn, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    return tex.sample(samp, vtf.uv) * vtf.color;
}
