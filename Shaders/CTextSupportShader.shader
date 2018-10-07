#shader CTextSupportShaderAlpha
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#instattribute color 0
#instattribute color 1
#instattribute color 2
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 uvIn[4];
layout(location=8) in vec4 fontColorIn;
layout(location=9) in vec4 outlineColorIn;
layout(location=10) in vec4 mulColorIn;

UBINDING0 uniform TextSupportUniform
{
    mat4 mtx;
    vec4 color;
};

struct VertToFrag
{
    vec4 fontColor;
    vec4 outlineColor;
    vec4 mulColor;
    vec3 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec3 pos = posIn[gl_VertexID].xyz;
    vtf.uv = uvIn[gl_VertexID].xyz;
    vtf.fontColor = color * fontColorIn;
    vtf.outlineColor = color * outlineColorIn;
    vtf.mulColor = mulColorIn;
    gl_Position = mtx * vec4(pos, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 fontColor;
    vec4 outlineColor;
    vec4 mulColor;
    vec3 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2DArray tex;
void main()
{
    vec4 texel = texture(tex, vtf.uv);
    colorOut = (vtf.fontColor * texel.r + vtf.outlineColor * texel.g) * vtf.mulColor;
}

#vertex hlsl
struct InstData
{
    float4 posIn[4] : POSITION;
    float4 uvIn[4] : UV;
    float4 fontColorIn : COLOR0;
    float4 outlineColorIn : COLOR1;
    float4 mulColorIn : COLOR2;
};

cbuffer TextSupportUniform : register(b0)
{
    float4x4 mtx;
    float4 color;
};

struct VertToFrag
{
    float4 pos : SV_Position;
    float4 fontColor : COLOR0;
    float4 outlineColor : COLOR1;
    float4 mulColor : COLOR2;
    float3 uv : UV;
};

VertToFrag main(in InstData inst, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.fontColor = color * inst.fontColorIn;
    vtf.outlineColor = color * inst.outlineColorIn;
    vtf.mulColor = inst.mulColorIn;
    vtf.uv = inst.uvIn[vertId].xyz;
    vtf.pos = mul(mtx, float4(inst.posIn[vertId].xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
    float4 fontColor : COLOR0;
    float4 outlineColor : COLOR1;
    float4 mulColor : COLOR2;
    float3 uv : UV;
};

Texture2DArray tex : register(t0);
SamplerState samp : register(s3);

float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 texel = tex.Sample(samp, vtf.uv.xyz);
    return (vtf.fontColor * texel.r + vtf.outlineColor * texel.g) * vtf.mulColor;
}

#vertex metal
struct InstData
{
    float4 posIn[4];
    float4 uvIn[4];
    float4 fontColorIn;
    float4 outlineColorIn;
    float4 mulColorIn;
};

struct TextSupportUniform
{
    float4x4 mtx;
    float4 color;
};

struct VertToFrag
{
    float4 pos [[ position ]];
    float4 fontColor;
    float4 outlineColor;
    float4 mulColor;
    float3 uv;
};

vertex VertToFrag vmain(constant InstData* instArr [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant TextSupportUniform& uData [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant InstData& inst = instArr[instId];
    vtf.fontColor = inst.fontColorIn * uData.color;
    vtf.outlineColor = inst.outlineColorIn * uData.color;
    vtf.mulColor = inst.mulColorIn;
    vtf.uv = inst.uvIn[vertId].xyz;
    vtf.pos = uData.mtx * float4(inst.posIn[vertId].xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 fontColor;
    float4 outlineColor;
    float4 mulColor;
    float3 uv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d_array<float> tex [[ texture(0) ]])
{
    float4 texel = tex.sample(clampSamp, vtf.uv.xy, vtf.uv.z);
    return (vtf.fontColor * texel.r + vtf.outlineColor * texel.g) * vtf.mulColor;
}

#shader CTextSupportShaderAdd : CTextSupportShaderAlpha
#srcfac srcalpha
#dstfac one

#shader CTextSupportShaderAddOverdraw : CTextSupportShaderAlpha
#srcfac one
#dstfac one

#shader CTextSupportShaderImageAlpha
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#instattribute color 0
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec3 posIn[4];
layout(location=4) in vec2 uvIn[4];
layout(location=8) in vec4 colorIn;

UBINDING0 uniform TextSupportUniform
{
    mat4 mtx;
    vec4 color;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec3 pos = posIn[gl_VertexID].xyz;
    vtf.uv = uvIn[gl_VertexID];
    vtf.color = color * colorIn;
    gl_Position = mtx * vec4(pos, 1.0);
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
    vec4 texel = texture(tex, vtf.uv);
    colorOut = vtf.color * texel;
}

#vertex hlsl
struct InstData
{
    float4 posIn[4] : POSITION;
    float4 uvIn[4] : UV;
    float4 colorIn : COLOR;
};

cbuffer TextSupportUniform : register(b0)
{
    float4x4 mtx;
    float4 color;
};

struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

VertToFrag main(in InstData inst, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = color * inst.colorIn;
    vtf.uv = inst.uvIn[vertId].xy;
    vtf.pos = mul(mtx, float4(inst.posIn[vertId].xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

Texture2D tex : register(t0);
SamplerState samp : register(s3);

float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 texel = tex.Sample(samp, vtf.uv);
    return vtf.color * texel;
}

#vertex metal
struct InstData
{
    float4 posIn[4];
    float4 uvIn[4];
    float4 colorIn;
};

struct TextSupportUniform
{
    float4x4 mtx;
    float4 color;
};

struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant InstData* instArr [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant TextSupportUniform& uData [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant InstData& inst = instArr[instId];
    vtf.color = uData.color * inst.colorIn;
    vtf.uv = inst.uvIn[vertId].xy;
    vtf.pos = uData.mtx * float4(inst.posIn[vertId].xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    float4 texel = tex.sample(clampSamp, vtf.uv);
    return vtf.color * texel;
}

#shader CTextSupportShaderImageAdd : CTextSupportShaderImageAlpha
#srcfac srcalpha
#dstfac one

#shader CTextSupportShaderImageAddOverdraw : CTextSupportShaderImageAlpha
#srcfac one
#dstfac one
