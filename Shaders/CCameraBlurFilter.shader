#shader CCameraBlurFilter
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

UBINDING0 uniform CameraBlurUniform
{
    vec4 uv0;
    vec4 uv1;
    vec4 uv2;
    vec4 uv3;
    vec4 uv4;
    vec4 uv5;
    float opacity;
};

struct VertToFrag
{
    vec2 uvReg;
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec2 uv3;
    vec2 uv4;
    vec2 uv5;
    float opacity;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uvReg = uvIn.xy;
    vtf.uv0 = uv0.xy + uvIn.xy;
    vtf.uv1 = uv1.xy + uvIn.xy;
    vtf.uv2 = uv2.xy + uvIn.xy;
    vtf.uv3 = uv3.xy + uvIn.xy;
    vtf.uv4 = uv4.xy + uvIn.xy;
    vtf.uv5 = uv5.xy + uvIn.xy;
    vtf.opacity = opacity;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec2 uvReg;
    vec2 uv0;
    vec2 uv1;
    vec2 uv2;
    vec2 uv3;
    vec2 uv4;
    vec2 uv5;
    float opacity;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D sceneTex;
void main()
{
    vec4 colorSample = texture(sceneTex, vtf.uvReg) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv0) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv1) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv2) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv3) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv4) * 0.14285715;
    colorSample += texture(sceneTex, vtf.uv5) * 0.14285715;
    colorOut = vec4(colorSample.rgb, vtf.opacity);
}


#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer CameraBlurUniform : register(b0)
{
    float4 uv0;
    float4 uv1;
    float4 uv2;
    float4 uv3;
    float4 uv4;
    float4 uv5;
    float opacity;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float2 uvReg : UV6;
    float2 uv0 : UV0;
    float2 uv1 : UV1;
    float2 uv2 : UV2;
    float2 uv3 : UV3;
    float2 uv4 : UV4;
    float2 uv5 : UV5;
    float opacity : OPACITY;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.uvReg = v.uvIn.xy;
    vtf.uvReg.y = 1.0 - vtf.uvReg.y;
    vtf.uv0 = uv0.xy + v.uvIn.xy;
    vtf.uv0.y = 1.0 - vtf.uv0.y;
    vtf.uv1 = uv1.xy + v.uvIn.xy;
    vtf.uv1.y = 1.0 - vtf.uv1.y;
    vtf.uv2 = uv2.xy + v.uvIn.xy;
    vtf.uv2.y = 1.0 - vtf.uv2.y;
    vtf.uv3 = uv3.xy + v.uvIn.xy;
    vtf.uv3.y = 1.0 - vtf.uv3.y;
    vtf.uv4 = uv4.xy + v.uvIn.xy;
    vtf.uv4.y = 1.0 - vtf.uv4.y;
    vtf.uv5 = uv5.xy + v.uvIn.xy;
    vtf.uv5.y = 1.0 - vtf.uv5.y;
    vtf.opacity = opacity;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
Texture2D sceneTex : register(t0);
SamplerState samp : register(s0);
struct VertToFrag
{
    float4 position : SV_Position;
    float2 uvReg : UV6;
    float2 uv0 : UV0;
    float2 uv1 : UV1;
    float2 uv2 : UV2;
    float2 uv3 : UV3;
    float2 uv4 : UV4;
    float2 uv5 : UV5;
    float opacity : OPACITY;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 colorSample = sceneTex.Sample(samp, vtf.uvReg) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv0) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv1) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv2) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv3) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv4) * 0.14285715;
    colorSample += sceneTex.Sample(samp, vtf.uv5) * 0.14285715;
    return float4(colorSample.rgb, vtf.opacity);
}


#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct CameraBlurUniform
{
    float4 uv0;
    float4 uv1;
    float4 uv2;
    float4 uv3;
    float4 uv4;
    float4 uv5;
    float opacity;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float2 uvReg;
    float2 uv0;
    float2 uv1;
    float2 uv2;
    float2 uv3;
    float2 uv4;
    float2 uv5;
    float opacity;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant CameraBlurUniform& cbu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.uvReg = v.uvIn.xy;
    vtf.uvReg.y = 1.0 - vtf.uvReg.y;
    vtf.uv0 = cbu.uv0.xy + v.uvIn.xy;
    vtf.uv0.y = 1.0 - vtf.uv0.y;
    vtf.uv1 = cbu.uv1.xy + v.uvIn.xy;
    vtf.uv1.y = 1.0 - vtf.uv1.y;
    vtf.uv2 = cbu.uv2.xy + v.uvIn.xy;
    vtf.uv2.y = 1.0 - vtf.uv2.y;
    vtf.uv3 = cbu.uv3.xy + v.uvIn.xy;
    vtf.uv3.y = 1.0 - vtf.uv3.y;
    vtf.uv4 = cbu.uv4.xy + v.uvIn.xy;
    vtf.uv4.y = 1.0 - vtf.uv4.y;
    vtf.uv5 = cbu.uv5.xy + v.uvIn.xy;
    vtf.uv5.y = 1.0 - vtf.uv5.y;
    vtf.opacity = cbu.opacity;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 uvReg;
    float2 uv0;
    float2 uv1;
    float2 uv2;
    float2 uv3;
    float2 uv4;
    float2 uv5;
    float opacity;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]], sampler samp [[ sampler(0) ]],
                      texture2d<float> sceneTex [[ texture(0) ]])
{
    float4 colorSample = sceneTex.sample(samp, vtf.uvReg) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv0) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv1) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv2) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv3) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv4) * 0.14285715;
    colorSample += sceneTex.sample(samp, vtf.uv5) * 0.14285715;
    return float4(colorSample.rgb, vtf.opacity);
}
