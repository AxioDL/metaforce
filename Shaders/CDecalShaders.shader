#shader CDecalShaderTexZTest
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;
layout(location=5) in vec4 uvsIn[4];

UBINDING0 uniform DecalUniform
{
    mat4 mvp;
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
    vtf.uv = uvsIn[gl_VertexID].xy;
    gl_Position = mvp * posIn[gl_VertexID];
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
    float4 posIn[4] : POSITION;
    float4 colorIn : COLOR;
    float4 uvsIn[4] : UV;
};

cbuffer DecalUniform : register(b0)
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.uv = v.uvsIn[vertId].xy;
    vtf.position = mul(mvp, v.posIn[vertId]);
    return vtf;
}

#fragment hlsl
SamplerState samp : register(s0);
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
    float4 posIn[4];
    float4 colorIn;
    float4 uvsIn[4];
};

struct DecalUniform
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant DecalUniform& decal [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * decal.moduColor;
    vtf.uv = v.uvsIn[vertId].xy;
    vtf.position = decal.mvp * v.posIn[vertId];
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
                      texture2d<float> tex0 [[ texture(0) ]])
{
    return vtf.color * tex0.sample(samp, vtf.uv);
}


#shader CDecalShaderTexAdditiveZTest : CDecalShaderTexZTest
#srcfac srcalpha
#dstfac one

#shader CDecalShaderTexRedToAlphaZTest : CDecalShaderTexZTest
#srcfac one
#dstfac one
#alphawrite true

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
    colorOut = vtf.color;
    colorOut.a = texture(tex, vtf.uv).r;
}


#fragment hlsl
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return float4(vtf.color.rgb, tex0.Sample(samp, vtf.uv).r);
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
                      texture2d<float> tex0 [[ texture(0) ]])
{
    return float4(vtf.color.rgb, tex0.sample(samp, vtf.uv).r);
}


#shader CDecalShaderNoTexZTest
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite false
#alphawrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;

UBINDING0 uniform DecalUniform
{
    mat4 mvp;
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
    gl_Position = mvp * posIn[gl_VertexID];
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
    float4 posIn[4] : POSITION;
    float4 colorIn : COLOR;
};

cbuffer DecalUniform : register(b0)
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.position = mul(mvp, v.posIn[vertId]);
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
    float4 posIn[4];
    float4 colorIn;
};

struct DecalUniform
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant DecalUniform& decal [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * decal.moduColor;
    vtf.position = decal.mvp * v.posIn[vertId];
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

#shader CDecalShaderNoTexAdditiveZTest : CDecalShaderNoTexZTest
#srcfac srcalpha
#dstfac one
