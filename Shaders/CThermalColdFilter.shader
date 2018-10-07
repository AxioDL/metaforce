#shader CThermalColdFilter
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

UBINDING0 uniform ThermalColdUniform
{
    mat4 shiftMtx;
    mat4 indMtx;
    vec4 shiftScale;
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
};

struct VertToFrag
{
    mat3 indMtx;
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
    vec2 sceneUv;
    vec2 shiftUv;
    vec2 shiftScale;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.indMtx = mat3(indMtx);
    vtf.colorReg0 = colorReg0;
    vtf.colorReg1 = colorReg1;
    vtf.colorReg2 = colorReg2;
    vtf.sceneUv = uvIn.xy;
    vtf.shiftUv = (mat3(shiftMtx) * uvIn.xyz).xy;
    vtf.shiftScale = shiftScale.xy;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    mat3 indMtx;
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
    vec2 sceneUv;
    vec2 shiftUv;
    vec2 shiftScale;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
TBINDING1 uniform sampler2D shiftTex;
const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);
void main()
{
    vec2 shiftCoordTexel = texture(shiftTex, vtf.shiftUv).xy;
    vec2 shiftCoord = vtf.sceneUv + shiftCoordTexel * vtf.shiftScale;
    float shiftScene0 = dot(texture(sceneTex, shiftCoord), kRGBToYPrime);
    float shiftScene1 = dot(texture(sceneTex, shiftCoord + vec2(vtf.shiftScale.x / 8.0, 0.0)), kRGBToYPrime);
    vec2 indCoord = (vtf.indMtx * vec3(shiftScene0 - 0.5, shiftScene1 - 0.5, 1.0)).xy;
    float indScene = dot(texture(sceneTex, vtf.sceneUv + indCoord), kRGBToYPrime);
    colorOut = vtf.colorReg0 * indScene + vtf.colorReg1 * shiftScene0 + vtf.colorReg2;
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer ThermalColdUniform : register(b0)
{
    float4x4 shiftMtx;
    float4x4 indMtx;
    float4 shiftScale;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float3x3 indMtx : INDMTX;
    float4 colorReg0 : COLORREG0;
    float4 colorReg1 : COLORREG1;
    float4 colorReg2 : COLORREG2;
    float2 sceneUv : SCENEUV;
    float2 shiftUv : SHIFTUV;
    float2 shiftScale : SHIFTSCALE;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.indMtx = float3x3(indMtx[0].xyz, indMtx[1].xyz, indMtx[2].xyz);
    vtf.colorReg0 = colorReg0;
    vtf.colorReg1 = colorReg1;
    vtf.colorReg2 = colorReg2;
    vtf.sceneUv = v.uvIn.xy;
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.shiftUv = (mul(float3x3(shiftMtx[0].xyz, shiftMtx[1].xyz, shiftMtx[2].xyz), v.uvIn.xyz)).xy;
    vtf.shiftScale = shiftScale.xy;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
Texture2D shiftTex : register(t1);
SamplerState samp : register(s3);
struct VertToFrag
{
    float4 position : SV_Position;
    float3x3 indMtx : INDMTX;
    float4 colorReg0 : COLORREG0;
    float4 colorReg1 : COLORREG1;
    float4 colorReg2 : COLORREG2;
    float2 sceneUv : SCENEUV;
    float2 shiftUv : SHIFTUV;
    float2 shiftScale : SHIFTSCALE;
};

static const float4 kRGBToYPrime = {0.299, 0.587, 0.114, 0.0};
float4 main(in VertToFrag vtf) : SV_Target0
{
    float2 shiftCoordTexel = shiftTex.Sample(samp, vtf.shiftUv).xy;
    float2 shiftCoord = vtf.sceneUv + shiftCoordTexel * vtf.shiftScale;
    float shiftScene0 = dot(sceneTex.Sample(samp, shiftCoord), kRGBToYPrime);
    float shiftScene1 = dot(sceneTex.Sample(samp, shiftCoord + float2(vtf.shiftScale.x / 8.0, 0.0)), kRGBToYPrime);
    float2 indCoord = (mul(vtf.indMtx, float3(shiftScene0 - 0.5, shiftScene1 - 0.5, 1.0))).xy;
    float indScene = dot(sceneTex.Sample(samp, vtf.sceneUv + indCoord), kRGBToYPrime);
    return vtf.colorReg0 * indScene + vtf.colorReg1 * shiftScene0 + vtf.colorReg2;
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct ThermalColdUniform
{
    float4x4 shiftMtx;
    float4x4 indMtx;
    float4 shiftScale;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float3 indMtx0;
    float3 indMtx1;
    float3 indMtx2;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
    float2 sceneUv;
    float2 shiftUv;
    float2 shiftScale;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ThermalColdUniform& tcu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.indMtx0 = tcu.indMtx[0].xyz;
    vtf.indMtx1 = tcu.indMtx[1].xyz;
    vtf.indMtx2 = tcu.indMtx[2].xyz;
    vtf.colorReg0 = tcu.colorReg0;
    vtf.colorReg1 = tcu.colorReg1;
    vtf.colorReg2 = tcu.colorReg2;
    vtf.sceneUv = v.uvIn.xy;
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.shiftUv = (float3x3(tcu.shiftMtx[0].xyz, tcu.shiftMtx[1].xyz, tcu.shiftMtx[2].xyz) * v.uvIn.xyz).xy;
    vtf.shiftScale = tcu.shiftScale.xy;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float3 indMtx0;
    float3 indMtx1;
    float3 indMtx2;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
    float2 sceneUv;
    float2 shiftUv;
    float2 shiftScale;
};

constant float4 kRGBToYPrime = {0.299, 0.587, 0.114, 0.0};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> sceneTex [[ texture(0) ]],
                      texture2d<float> shiftTex [[ texture(1) ]])
{
    float2 shiftCoordTexel = shiftTex.sample(samp, vtf.shiftUv).xy;
    float2 shiftCoord = vtf.sceneUv + shiftCoordTexel * vtf.shiftScale;
    float shiftScene0 = dot(sceneTex.sample(samp, shiftCoord), kRGBToYPrime);
    float shiftScene1 = dot(sceneTex.sample(samp, shiftCoord + float2(vtf.shiftScale.x / 8.0, 0.0)), kRGBToYPrime);
    float2 indCoord = (float3x3(vtf.indMtx0, vtf.indMtx1, vtf.indMtx2) * float3(shiftScene0 - 0.5, shiftScene1 - 0.5, 1.0)).xy;
    float indScene = dot(sceneTex.sample(samp, vtf.sceneUv + indCoord), kRGBToYPrime);
    return vtf.colorReg0 * indScene + vtf.colorReg1 * shiftScene0 + vtf.colorReg2;
}
