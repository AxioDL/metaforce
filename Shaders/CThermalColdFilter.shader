#shader CThermalColdFilter
#attribute position4
#attribute uv4 0
#attribute uv4 1
#srcfac one
#dstfac zero
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;
layout(location=2) in vec4 uvNoiseIn;

UBINDING0 uniform ThermalColdUniform
{
    mat4 indMtx;
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
    float randOff;
};

struct VertToFrag
{
    mat3 indMtx;
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
    vec2 sceneUv;
    vec2 noiseUv;
    float randOff;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.indMtx = mat3(indMtx);
    vtf.colorReg0 = colorReg0;
    vtf.colorReg1 = colorReg1;
    vtf.colorReg2 = colorReg2;
    vtf.sceneUv = uvIn.xy;
    vtf.noiseUv = uvNoiseIn.xy;
    vtf.randOff = randOff;
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
    vec2 noiseUv;
    float randOff;
};

ivec2 Lookup8BPP(in vec2 uv, in float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return ivec2(addr & 0x3ff, addr >> 10);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
TBINDING1 uniform sampler2D noiseTex;
const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);
void main()
{
    vec4 noiseTexel = texelFetch(noiseTex, Lookup8BPP(vtf.noiseUv, vtf.randOff), 0);
    vec2 indCoord = (vtf.indMtx * vec3(noiseTexel.x - 0.5, noiseTexel.y - 0.5, 1.0)).xy;
    float indScene = dot(texture(sceneTex, vtf.sceneUv + indCoord), kRGBToYPrime);
    colorOut = vtf.colorReg0 * indScene + vtf.colorReg1 * noiseTexel + vtf.colorReg2;
    colorOut.a = vtf.colorReg1.a + vtf.colorReg1.a * noiseTexel.a + vtf.colorReg2.a;
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV0;
    float4 uvNoiseIn : UV1;
};

cbuffer ThermalColdUniform : register(b0)
{
    float4x4 indMtx;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
    float randOff;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float3x3 indMtx : INDMTX;
    float4 colorReg0 : COLORREG0;
    float4 colorReg1 : COLORREG1;
    float4 colorReg2 : COLORREG2;
    float2 sceneUv : SCENEUV;
    float2 noiseUv : NOISEUV;
    float randOff : RANDOFF;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.indMtx = float3x3(indMtx[0].xyz, indMtx[1].xyz, indMtx[2].xyz);
    vtf.colorReg0 = colorReg0;
    vtf.colorReg1 = colorReg1;
    vtf.colorReg2 = colorReg2;
    vtf.sceneUv = v.uvIn.xy;
    vtf.noiseUv = v.uvNoiseIn.xy;
    vtf.randOff = randOff;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
Texture2D noiseTex : register(t1);
SamplerState samp : register(s3);
struct VertToFrag
{
    float4 position : SV_Position;
    float3x3 indMtx : INDMTX;
    float4 colorReg0 : COLORREG0;
    float4 colorReg1 : COLORREG1;
    float4 colorReg2 : COLORREG2;
    float2 sceneUv : SCENEUV;
    float2 noiseUv : NOISEUV;
    float randOff : RANDOFF;
};

static int3 Lookup8BPP(float2 uv, float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return int3(addr & 0x3ff, addr >> 10, 0);
}

static const float4 kRGBToYPrime = {0.299, 0.587, 0.114, 0.0};
float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 noiseTexel = noiseTex.Load(Lookup8BPP(vtf.noiseUv, vtf.randOff));
    float2 indCoord = mul(vtf.indMtx, float3(noiseTexel.x - 0.5, noiseTexel.y - 0.5, 1.0)).xy;
    float indScene = dot(sceneTex.Sample(samp, vtf.sceneUv + indCoord), kRGBToYPrime);
    float4 colorOut = vtf.colorReg0 * indScene + vtf.colorReg1 * noiseTexel + vtf.colorReg2;
    colorOut.a = vtf.colorReg1.a + vtf.colorReg1.a * noiseTexel.a + vtf.colorReg2.a;
    return colorOut;
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
    float4 uvNoiseIn [[ attribute(2) ]];
};

struct ThermalColdUniform
{
    float4x4 indMtx;
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
    float randOff;
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
    float2 noiseUv;
    float randOff;
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
    vtf.noiseUv = v.uvNoiseIn.xy;
    vtf.randOff = tcu.randOff;
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
    float2 noiseUv;
    float randOff;
};

static uint2 Lookup8BPP(float2 uv, float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return uint2(addr & 0x3ff, addr >> 10);
}

constant float4 kRGBToYPrime = {0.299, 0.587, 0.114, 0.0};
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> sceneTex [[ texture(0) ]],
                      texture2d<float> noiseTex [[ texture(1) ]])
{
    float4 noiseTexel = noiseTex.read(Lookup8BPP(vtf.noiseUv, vtf.randOff));
    float2 indCoord = (vtf.indMtx * float3(noiseTexel.x - 0.5, noiseTexel.y - 0.5, 1.0)).xy;
    float indScene = dot(sceneTex.sample(samp, vtf.sceneUv + indCoord), kRGBToYPrime);
    float4 colorOut = vtf.colorReg0 * indScene + vtf.colorReg1 * noiseTexel + vtf.colorReg2;
    colorOut.a = vtf.colorReg1.a + vtf.colorReg1.a * noiseTexel.a + vtf.colorReg2.a;
    return colorOut;
}
