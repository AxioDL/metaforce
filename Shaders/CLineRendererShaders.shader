#shader CLineRendererShaderTexAlpha
#attribute position4
#attribute color
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;
layout(location=2) in vec4 uvIn;

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn * moduColor;
    vtf.uv = uvIn.xy;
    gl_Position = posIn;
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = vtf.color * texture(tex, vtf.uv);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 colorIn : COLOR;
    float4 uvIn : UV;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.uv = v.uvIn.xy;
    vtf.position = v.posIn;
    return vtf;
}

#fragment hlsl
SamplerState samp : register(s3);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * tex0.Sample(samp, vtf.uv);
}

#vertex metal
struct VertData
{
    float4 posIn;
    float4 colorIn;
    float4 uvIn;
};

struct LineUniform
{
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],
                        uint vertId [[ vertex_id ]],
                        constant LineUniform& line [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[vertId];
    vtf.color = v.colorIn * line.moduColor;
    vtf.uv = v.uvIn.xy;
    vtf.position = v.posIn;
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
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> tex0 [[ texture(0) ]])
{
    return vtf.color * tex0.sample(samp, vtf.uv);
}

#shader CLineRendererShaderTexAdditive : CLineRendererShaderTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest none

#shader CLineRendererShaderTexAlphaZ : CLineRendererShaderTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal

#shader CLineRendererShaderTexAdditiveZ : CLineRendererShaderTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest lequal

#shader CLineRendererShaderNoTexAlpha
#attribute position4
#attribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 color;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn * moduColor;
    gl_Position = posIn;
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
    float4 posIn : POSITION;
    float4 colorIn : COLOR;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.position = v.posIn;
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
    float4 posIn;
    float4 colorIn;
};

struct LineUniform
{
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],
                        uint vertId [[ vertex_id ]],
                        constant LineUniform& line [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[vertId];
    vtf.color = v.colorIn * line.moduColor;
    vtf.position = v.posIn;
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

#shader CLineRendererShaderNoTexAdditive : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest none

#shader CLineRendererShaderNoTexAlphaZ : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal

#shader CLineRendererShaderNoTexAdditiveZ : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest lequal

#shader CLineRendererShaderNoTexAlphaZGEqual : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest gequal
