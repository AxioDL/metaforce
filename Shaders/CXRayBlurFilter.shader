#shader CXRayBlurFilter
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

UBINDING0 uniform XRayBlurUniform
{
    mat4 uv0;
    mat4 uv1;
    mat4 uv2;
    mat4 uv3;
    mat4 uv4;
    mat4 uv5;
    mat4 uv6;
    mat4 uv7;
};

struct VertToFrag
{
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec2 uv3;
    vec2 uv4;
    vec2 uv5;
    vec2 uv6;
    vec2 uv7;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uv0 = (uv0 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv1 = (uv1 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv2 = (uv2 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv3 = (uv3 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv4 = (uv4 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv5 = (uv5 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv6 = (uv6 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv7 = (uv7 * vec4(uvIn.xy, 0.0, 1.0)).xy;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec2 uv3;
    vec2 uv4;
    vec2 uv5;
    vec2 uv6;
    vec2 uv7;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
TBINDING1 uniform sampler2D paletteTex;
const vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);
void main()
{
    vec4 colorSample = texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv4), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv5), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv6), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += texture(paletteTex, vec2(dot(texture(sceneTex, vtf.uv7), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorOut = colorSample;
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer XRayBlurUniform : register(b0)
{
    float4x4 uv0;
    float4x4 uv1;
    float4x4 uv2;
    float4x4 uv3;
    float4x4 uv4;
    float4x4 uv5;
    float4x4 uv6;
    float4x4 uv7;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float2 uv0 : UV0;
    float2 uv1 : UV1;
    float2 uv2 : UV2;
    float2 uv3 : UV3;
    float2 uv4 : UV4;
    float2 uv5 : UV5;
    float2 uv6 : UV6;
    float2 uv7 : UV7;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.uv0 = mul(uv0, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv0.y = 1.0 - vtf.uv0.y;
    vtf.uv1 = mul(uv1, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv1.y = 1.0 - vtf.uv1.y;
    vtf.uv2 = mul(uv2, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv2.y = 1.0 - vtf.uv2.y;
    vtf.uv3 = mul(uv3, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv3.y = 1.0 - vtf.uv3.y;
    vtf.uv4 = mul(uv4, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv4.y = 1.0 - vtf.uv4.y;
    vtf.uv5 = mul(uv5, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv5.y = 1.0 - vtf.uv5.y;
    vtf.uv6 = mul(uv6, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv6.y = 1.0 - vtf.uv6.y;
    vtf.uv7 = mul(uv7, float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv7.y = 1.0 - vtf.uv7.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
Texture2D paletteTex : register(t1);
SamplerState samp : register(s3);
struct VertToFrag
{
    float4 position : SV_Position;
    float2 uv0 : UV0;
    float2 uv1 : UV1;
    float2 uv2 : UV2;
    float2 uv3 : UV3;
    float2 uv4 : UV4;
    float2 uv5 : UV5;
    float2 uv6 : UV6;
    float2 uv7 : UV7;
};

static const float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 colorSample = paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv4), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv5), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv6), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.Sample(samp, float2(dot(sceneTex.Sample(samp, vtf.uv7), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    return colorSample;
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct XRayBlurUniform
{
    float4x4 uv0;
    float4x4 uv1;
    float4x4 uv2;
    float4x4 uv3;
    float4x4 uv4;
    float4x4 uv5;
    float4x4 uv6;
    float4x4 uv7;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float2 uv0;
    float2 uv1;
    float2 uv2;
    float2 uv3;
    float2 uv4;
    float2 uv5;
    float2 uv6;
    float2 uv7;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant XRayBlurUniform& xbu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.uv0 = (xbu.uv0 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv0.y = 1.0 - vtf.uv0.y;
    vtf.uv1 = (xbu.uv1 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv1.y = 1.0 - vtf.uv1.y;
    vtf.uv2 = (xbu.uv2 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv2.y = 1.0 - vtf.uv2.y;
    vtf.uv3 = (xbu.uv3 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv3.y = 1.0 - vtf.uv3.y;
    vtf.uv4 = (xbu.uv4 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv4.y = 1.0 - vtf.uv4.y;
    vtf.uv5 = (xbu.uv5 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv5.y = 1.0 - vtf.uv5.y;
    vtf.uv6 = (xbu.uv6 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv6.y = 1.0 - vtf.uv6.y;
    vtf.uv7 = (xbu.uv7 * float4(v.uvIn.xy, 0.0, 1.0)).xy;
    vtf.uv7.y = 1.0 - vtf.uv7.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 uv0;
    float2 uv1;
    float2 uv2;
    float2 uv3;
    float2 uv4;
    float2 uv5;
    float2 uv6;
    float2 uv7;
};

constant float4 kRGBToYPrime = float4(0.299, 0.587, 0.114, 0.0);
fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> sceneTex [[ texture(0) ]],
                      texture2d<float> paletteTex [[ texture(1) ]])
{
    float4 colorSample = paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv0), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv1), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv2), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv3), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv4), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv5), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv6), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    colorSample += paletteTex.sample(samp, float2(dot(sceneTex.sample(samp, vtf.uv7), kRGBToYPrime) * 0.98 + 0.01, 0.5)) * 0.125;
    return colorSample;
}
