#shader CThermalHotFilter
#attribute position4
#attribute uv4
#srcfac dstalpha
#dstfac invdstalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;

UBINDING0 uniform ThermalHotUniform
{
    vec4 colorReg0;
    vec4 colorReg1;
    vec4 colorReg2;
};

struct VertToFrag
{
    vec2 sceneUv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.sceneUv = uvIn.xy;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec2 sceneUv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
TBINDING1 uniform sampler2D paletteTex;
const vec4 kRGBToYPrime = vec4(0.257, 0.504, 0.098, 0.0);
void main()
{
    float sceneSample = dot(texture(sceneTex, vtf.sceneUv), kRGBToYPrime) + 16.0 / 255.0;
    vec4 colorSample = texture(paletteTex, vec2(sceneSample / 16.0, 0.5));
    colorOut = vec4(colorSample.rgb, 0.0);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer ThermalHotUniform : register(b0)
{
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float2 sceneUv : UV;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.sceneUv = v.uvIn.xy;
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
Texture2D paletteTex : register(t1);
SamplerState samp : register(s0);
struct VertToFrag
{
    float4 position : SV_Position;
    float2 sceneUv : UV;
};

static const float4 kRGBToYPrime = float4(0.257, 0.504, 0.098, 0.0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float sceneSample = dot(sceneTex.Sample(samp, vtf.sceneUv), kRGBToYPrime) + 16.0 / 255.0;
    float4 colorSample = paletteTex.Sample(samp, float2(sceneSample / 16.0, 0.5));
    return float4(colorSample.rgb, 0.0);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct ThermalHotUniform
{
    float4 colorReg0;
    float4 colorReg1;
    float4 colorReg2;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float2 sceneUv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ThermalHotUniform& thu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.sceneUv = v.uvIn.xy;
    vtf.sceneUv.y = 1.0 - vtf.sceneUv.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 sceneUv;
};

constant float4 kRGBToYPrime = float4(0.257, 0.504, 0.098, 0.0);
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> sceneTex [[ texture(0) ]],
                      texture2d<float> paletteTex [[ texture(1) ]])
{
    float sceneSample = dot(sceneTex.sample(samp, vtf.sceneUv), kRGBToYPrime) + 16.0 / 255.0;
    float4 colorSample = paletteTex.sample(samp, float2(sceneSample / 16.0, 0.5));
    return float4(colorSample.rgb, 0.0);
}
