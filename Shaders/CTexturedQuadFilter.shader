#shader CTexturedQuadFilterAlpha
#attribute position4
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;

UBINDING0 uniform TexuredQuadUniform
{
    mat4 mtx;
    vec4 color;
    float lod;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float lod;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = color;
    vtf.uv = uvIn.xy;
    vtf.lod = lod;
    gl_Position = mtx * vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float lod;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = vtf.color * vec4(texture(tex, vtf.uv, vtf.lod).rgb, 1.0);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer TexuredQuadUniform : register(b0)
{
    float4x4 mat;
    float4 color;
    float lod;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float lod : LOD;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = color;
    vtf.uv = v.uvIn.xy;
    vtf.lod = lod;
    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float lod : LOD;
};

Texture2D tex : register(t0);
SamplerState samp : register(s3);

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * float4(tex.SampleBias(samp, vtf.uv, vtf.lod).rgb, 1.0);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct TexuredQuadUniform
{
    float4x4 mat;
    float4 color;
    float lod;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
    float lod;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = tqu.color;
    vtf.uv = v.uvIn.xy;
    vtf.lod = tqu.lod;
    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
    float lod;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    return vtf.color * float4(tex.sample(clampSamp, vtf.uv, bias(vtf.lod)).rgb, 1.0);
}

#shader CTexturedQuadFilterAlphaGEqual : CTexturedQuadFilterAlpha
#depthtest gequal

#shader CTexturedQuadFilterAlphaLEqual : CTexturedQuadFilterAlpha
#depthtest lequal

#shader CTexturedQuadFilterAdd : CTexturedQuadFilterAlpha
#srcfac srcalpha
#dstfac one
#depthtest none

#shader CTexturedQuadFilterAddGEqual : CTexturedQuadFilterAdd
#depthtest gequal

#shader CTexturedQuadFilterAddLEqual : CTexturedQuadFilterAdd
#depthtest lequal

#shader CTexturedQuadFilterSubtract : CTexturedQuadFilterAlpha
#srcfac srcalpha
#dstfac subtract
#depthtest none

#shader CTexturedQuadFilterSubtractGEqual : CTexturedQuadFilterSubtract
#depthtest gequal

#shader CTexturedQuadFilterSubtractLEqual : CTexturedQuadFilterSubtract
#depthtest lequal

#shader CTexturedQuadFilterMult : CTexturedQuadFilterAlpha
#srcfac zero
#dstfac srccolor
#depthtest none
#overwritealpha true

#shader CTexturedQuadFilterMultGEqual : CTexturedQuadFilterMult
#depthtest gequal

#shader CTexturedQuadFilterMultLEqual : CTexturedQuadFilterMult
#depthtest lequal

#shader CTexturedQuadFilterInvDstMult : CTexturedQuadFilterAlpha
#srcfac zero
#dstfac invsrccolor
#depthtest none
#overwritealpha true

#shader CTexturedQuadFilterInvDstMultGEqual : CTexturedQuadFilterInvDstMult
#depthtest gequal

#shader CTexturedQuadFilterInvDstMultLEqual : CTexturedQuadFilterInvDstMult
#depthtest lequal

#shader CTexturedQuadFilterAlphaTexAlpha : CTexturedQuadFilterAlpha
#attribute position4
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest none
#overwritealpha false

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float lod;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = vtf.color * texture(tex, vtf.uv, vtf.lod);
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float lod : LOD;
};

Texture2D tex : register(t0);
SamplerState samp : register(s3);

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * tex.SampleBias(samp, vtf.uv, vtf.lod);
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
    float lod;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    return vtf.color * tex.sample(clampSamp, vtf.uv, bias(vtf.lod));
}

#shader CTexturedQuadFilterAlphaTexAdd : CTexturedQuadFilterAlphaTexAlpha
#srcfac srcalpha
#dstfac one

#shader CTexturedQuadFilterAlphaTexSubtract : CTexturedQuadFilterAlphaTexAlpha
#srcfac srcalpha
#dstfac subtract

#shader CTexturedQuadFilterAlphaTexMult : CTexturedQuadFilterAlphaTexAlpha
#srcfac zero
#dstfac srccolor
#overwritealpha true

#shader CTexturedQuadFilterAlphaTexInvDstMult : CTexturedQuadFilterAlphaTexAlpha
#srcfac zero
#dstfac invsrccolor
#overwritealpha true

#shader CTexturedQuadFilterAlphaGEqualZWrite : CTexturedQuadFilterAlpha
#depthtest gequal
#depthwrite true

#shader CTexturedQuadFilterAddGEqualZWrite : CTexturedQuadFilterAdd
#depthtest gequal
#depthwrite true

#shader CTexturedQuadFilterSubtractGEqualZWrite : CTexturedQuadFilterSubtract
#depthtest gequal
#depthwrite true

#shader CTexturedQuadFilterMultGEqualZWrite : CTexturedQuadFilterMult
#depthtest gequal
#depthwrite true
