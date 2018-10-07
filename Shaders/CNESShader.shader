#shader CNESShader
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
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = color;
    vtf.uv = uvIn.xy;
    gl_Position = mtx * vec4(posIn.xyz, 1.0);
    gl_Position = FLIPFROMGL(gl_Position);
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
    float4 uvIn : UV;
};

cbuffer TexuredQuadUniform : register(b0)
{
    float4x4 mat;
    float4 color;
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
    vtf.color = color;
    vtf.uv = v.uvIn.xy;
    vtf.position = mul(mat, float4(v.posIn.xyz, 1.0));
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
SamplerState samp : register(s4);

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * tex.Sample(samp, vtf.uv);
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
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant TexuredQuadUniform& tqu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = tqu.color;
    vtf.uv = v.uvIn.xy;
    vtf.position = tqu.mat * float4(v.posIn.xyz, 1.0);
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
                      sampler clampSamp [[ sampler(4) ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    return vtf.color * tex.sample(clampSamp, vtf.uv);
}
