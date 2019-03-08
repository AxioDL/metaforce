#shader CSpaceWarpFilter
#attribute position4
#attribute uv4
#srcfac one
#dstfac zero
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;

UBINDING0 uniform SpaceWarpUniform
{
    mat4 mainMtx;
    mat4 indMtx;
    vec4 strength;
};

struct VertToFrag
{
    vec2 sceneUv;
    vec2 indUv;
    vec2 strength;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    gl_Position = mainMtx * vec4(posIn.xy, 0.0, 1.0);
    vtf.sceneUv = gl_Position.xy * vec2(0.5) + vec2(0.5);
    vtf.indUv = (mat3(indMtx) * vec3(uvIn.xy, 1.0)).xy;
    vtf.strength = strength.xy;
}

#fragment glsl
struct VertToFrag
{
    vec2 sceneUv;
    vec2 indUv;
    vec2 strength;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
TBINDING1 uniform sampler2D indTex;
void main()
{
    vec2 indUv = texture(indTex, vtf.indUv).ab * vec2(2.0) - vec2(1.0 - 1.0 / 256.0);
    colorOut = vec4(texture(sceneTex, vtf.sceneUv + indUv * vtf.strength.xy).rgb, 1.0);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};
cbuffer SpaceWarpUniform : register(b0)
{
    float4x4 mainMtx;
    float4x4 indMtx;
    float4 strength;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float2 sceneUv : SCENEUV;
    float2 indUv : INDV;
    float2 strength : STRENGTH;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.position = mul(mainMtx, float4(v.posIn.xy, 0.0, 1.0));
    vtf.sceneUv = vtf.position.xy * float2(0.5, 0.5) + float2(0.5, 0.5);
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.indUv = mul(float3x3(indMtx[0].xyz, indMtx[1].xyz, indMtx[2].xyz), float3(v.uvIn.xy, 1.0)).xy;
    vtf.indUv.y = 1.0 - vtf.indUv.y;
    vtf.strength = strength.xy;
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
Texture2D indTex : register(t1);
SamplerState samp : register(s0);
struct VertToFrag
{
    float4 position : SV_Position;
    float2 sceneUv : SCENEUV;
    float2 indUv : INDV;
    float2 strength : STRENGTH;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    float2 indUv = indTex.Sample(samp, vtf.indUv).ab * float2(2.0, 2.0) - float2(1.0 - 1.0 / 256.0, 1.0 - 1.0 / 256.0);
    return float4(sceneTex.Sample(samp, vtf.sceneUv + indUv * vtf.strength.xy).rgb, 1.0);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};
struct SpaceWarpUniform
{
    float4x4 mainMtx;
    float4x4 indMtx;
    float4 strength;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float2 sceneUv;
    float2 indUv;
    float2 strength;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SpaceWarpUniform& swu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.position = swu.mainMtx * float4(v.posIn.xy, 0.0, 1.0);
    vtf.sceneUv = vtf.position.xy * float2(0.5) + float2(0.5);
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.indUv = (float3x3(swu.indMtx[0].xyz, swu.indMtx[1].xyz, swu.indMtx[2].xyz) * float3(v.uvIn.xy, 1.0)).xy;
    vtf.indUv.y = 1.0 - vtf.indUv.y;
    vtf.strength = swu.strength.xy;
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 sceneUv;
    float2 indUv;
    float2 strength;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> sceneTex [[ texture(0) ]],
                      texture2d<float> indTex [[ texture(1) ]])
{
    float2 indUv = indTex.sample(samp, vtf.indUv).ab * float2(2.0) - float2(1.0 - 1.0 / 256.0);
    return float4(sceneTex.sample(samp, vtf.sceneUv + indUv * vtf.strength.xy).rgb, 1.0);
}
