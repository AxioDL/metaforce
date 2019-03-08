#shader CPhazonSuitFilterInd
#attribute position4
#attribute uv4 0
#attribute uv4 1
#attribute uv4 2
#srcfac srcalpha
#dstfac one
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 screenUvIn;
layout(location=2) in vec4 indUvIn;
layout(location=3) in vec4 maskUvIn;

UBINDING0 uniform PhazonSuitUniform
{
    vec4 color;
    vec4 indScaleOff;
};

struct VertToFrag
{
    vec4 color;
    vec4 indScaleOff;
    vec2 screenUv;
    vec2 indUv;
    vec2 maskUv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = color;
    vtf.indScaleOff = indScaleOff;
    vtf.screenUv = screenUvIn.xy;
    vtf.indUv = indUvIn.xy;
    vtf.maskUv = maskUvIn.xy;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec4 indScaleOff;
    vec2 screenUv;
    vec2 indUv;
    vec2 maskUv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D screenTex;
TBINDING1 uniform sampler2D indTex;
TBINDING2 uniform sampler2D maskTex;
TBINDING3 uniform sampler2D maskTexBlur;
void main()
{
    vec2 indUv = (texture(indTex, vtf.indUv).ab - vec2(0.5, 0.5)) *
        vtf.indScaleOff.xy + vtf.indScaleOff.zw;
    float maskBlurAlpha = clamp(0.0, (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a) * 2.0, 1.0);
    colorOut = vtf.color * texture(screenTex, indUv + vtf.screenUv) * maskBlurAlpha;
    colorOut.a = vtf.color.a;
}

#vertex hlsl
struct VertData {
    float4 posIn : POSITION;
    float4 screenUvIn : UV0;
    float4 indUvIn : UV1;
    float4 maskUvIn : UV2;
};

cbuffer PhazonSuitUniform : register(b0)
{
    float4 color;
    float4 indScaleOff;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 indScaleOff : SCALEOFF;
    float2 screenUv : UV0;
    float2 indUv : UV1;
    float2 maskUv : UV2;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = color;
    vtf.indScaleOff = indScaleOff;
    vtf.screenUv = v.screenUvIn.xy;
    vtf.screenUv.y = 1.0 - vtf.screenUv.y;
    vtf.indUv = v.indUvIn.xy;
    vtf.maskUv = v.maskUvIn.xy;
    vtf.maskUv.y = 1.0 - vtf.maskUv.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 indScaleOff : SCALEOFF;
    float2 screenUv : UV0;
    float2 indUv : UV1;
    float2 maskUv : UV2;
};

SamplerState samp : register(s0);
Texture2D screenTex : register(t0);
Texture2D indTex : register(t1);
Texture2D maskTex : register(t2);
Texture2D maskTexBlur : register(t3);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float2 indUv = (indTex.Sample(samp, vtf.indUv).ab - float2(0.5, 0.5)) *
        vtf.indScaleOff.xy + vtf.indScaleOff.zw;
    float maskBlurAlpha = saturate((maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a) * 2.0);
    return float4((vtf.color * screenTex.Sample(samp, indUv + vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 screenUvIn [[ attribute(1) ]];
    float4 indUvIn [[ attribute(2) ]];
    float4 maskUvIn [[ attribute(3) ]];
};

struct PhazonSuitUniform
{
    float4 color;
    float4 indScaleOff;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float4 indScaleOff;
    float2 screenUv;
    float2 indUv;
    float2 maskUv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant PhazonSuitUniform& psu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = psu.color;
    vtf.indScaleOff = psu.indScaleOff;
    vtf.screenUv = v.screenUvIn.xy;
    vtf.screenUv.y = 1.0 - vtf.screenUv.y;
    vtf.indUv = v.indUvIn.xy;
    vtf.maskUv = v.maskUvIn.xy;
    vtf.maskUv.y = 1.0 - vtf.maskUv.y;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float4 indScaleOff;
    float2 screenUv;
    float2 indUv;
    float2 maskUv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> screenTex [[ texture(0) ]],
                      texture2d<float> indTex [[ texture(1) ]],
                      texture2d<float> maskTex [[ texture(2) ]],
                      texture2d<float> maskTexBlur [[ texture(3) ]])
{
    float2 indUv = (indTex.sample(samp, vtf.indUv).ab - float2(0.5, 0.5)) *
        vtf.indScaleOff.xy + vtf.indScaleOff.zw;
    float maskBlurAlpha = saturate((maskTexBlur.sample(samp, vtf.maskUv).a - maskTex.sample(samp, vtf.maskUv).a) * 2.0);
    return float4((vtf.color * screenTex.sample(samp, indUv + vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);
}


#shader CPhazonSuitFilterNoInd : CPhazonSuitFilterInd

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec4 indScaleOff;
    vec2 screenUv;
    vec2 indUv;
   vec2 maskUv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D screenTex;
TBINDING1 uniform sampler2D maskTex;
TBINDING2 uniform sampler2D maskTexBlur;
void main()
{
    float maskBlurAlpha = clamp(0.0, (texture(maskTexBlur, vtf.maskUv).a - texture(maskTex, vtf.maskUv).a) * 2.0, 1.0);
    colorOut = vtf.color * texture(screenTex, vtf.screenUv) * maskBlurAlpha;
    colorOut.a = vtf.color.a;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 indScaleOff : SCALEOFF;
    float2 screenUv : UV0;
    float2 indUv : UV1;
    float2 maskUv : UV2;
};

SamplerState samp : register(s3);
Texture2D screenTex : register(t0);
Texture2D maskTex : register(t1);
Texture2D maskTexBlur : register(t2);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float maskBlurAlpha = saturate((maskTexBlur.Sample(samp, vtf.maskUv).a - maskTex.Sample(samp, vtf.maskUv).a) * 2.0);
    return float4((vtf.color * screenTex.Sample(samp, vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);
}

#fragment metal
struct VertToFrag
{
    float4 color;
    float4 indScaleOff;
    float2 screenUv;
    float2 indUv;
    float2 maskUv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d<float> screenTex [[ texture(0) ]],
                      texture2d<float> maskTex [[ texture(1) ]],
                      texture2d<float> maskTexBlur [[ texture(2) ]])
{
    float maskBlurAlpha = saturate((maskTexBlur.sample(clampSamp, vtf.maskUv).a - maskTex.sample(clampSamp, vtf.maskUv).a) * 2.0);
    return float4((vtf.color * screenTex.sample(clampSamp, vtf.screenUv) * maskBlurAlpha).rgb, vtf.color.a);
}


#shader CPhazonSuitFilterBlur
#attribute position4
#attribute uv4
#srcfac one
#dstfac zero

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;

UBINDING0 uniform PhazonSuitBlurUniform
{
    vec4 blurDir;
};

struct VertToFrag
{
    vec2 uv;
    vec2 blurDir;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uv = uvIn.xy;
    vtf.blurDir = blurDir.xy;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec2 uv;
    vec2 blurDir;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D maskTex;
void main()
{
    //this will be our alpha sum
    float sum = 0.0;

    //apply blurring, using a 23-tap filter with predefined gaussian weights
    sum += texture(maskTex, vtf.uv + -11.0 * vtf.blurDir).a * 0.007249;
    sum += texture(maskTex, vtf.uv + -10.0 * vtf.blurDir).a * 0.011032;
    sum += texture(maskTex, vtf.uv + -9.0 * vtf.blurDir).a * 0.016133;
    sum += texture(maskTex, vtf.uv + -8.0 * vtf.blurDir).a * 0.022665;
    sum += texture(maskTex, vtf.uv + -7.0 * vtf.blurDir).a * 0.030595;
    sum += texture(maskTex, vtf.uv + -6.0 * vtf.blurDir).a * 0.039680;
    sum += texture(maskTex, vtf.uv + -5.0 * vtf.blurDir).a * 0.049444;
    sum += texture(maskTex, vtf.uv + -4.0 * vtf.blurDir).a * 0.059195;
    sum += texture(maskTex, vtf.uv + -3.0 * vtf.blurDir).a * 0.068091;
    sum += texture(maskTex, vtf.uv + -2.0 * vtf.blurDir).a * 0.075252;
    sum += texture(maskTex, vtf.uv + -1.0 * vtf.blurDir).a * 0.079905;
    sum += texture(maskTex, vtf.uv + 0.0 * vtf.blurDir).a * 0.081519;
    sum += texture(maskTex, vtf.uv + 1.0 * vtf.blurDir).a * 0.079905;
    sum += texture(maskTex, vtf.uv + 2.0 * vtf.blurDir).a * 0.075252;
    sum += texture(maskTex, vtf.uv + 3.0 * vtf.blurDir).a * 0.068091;
    sum += texture(maskTex, vtf.uv + 4.0 * vtf.blurDir).a * 0.059195;
    sum += texture(maskTex, vtf.uv + 5.0 * vtf.blurDir).a * 0.049444;
    sum += texture(maskTex, vtf.uv + 6.0 * vtf.blurDir).a * 0.039680;
    sum += texture(maskTex, vtf.uv + 7.0 * vtf.blurDir).a * 0.030595;
    sum += texture(maskTex, vtf.uv + 8.0 * vtf.blurDir).a * 0.022665;
    sum += texture(maskTex, vtf.uv + 9.0 * vtf.blurDir).a * 0.016133;
    sum += texture(maskTex, vtf.uv + 10.0 * vtf.blurDir).a * 0.011032;
    sum += texture(maskTex, vtf.uv + 11.0 * vtf.blurDir).a * 0.007249;

    colorOut = vec4(1.0, 1.0, 1.0, sum);
}

#vertex hlsl
struct VertData {
    float4 posIn : POSITION;
    float4 uvIn : UV;
};

cbuffer PhazonSuitBlurUniform : register(b0)
{
    float4 blurDir;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float2 uv : UV;
    float2 blurDir : BLURDIR;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.uv = v.uvIn.xy;
    vtf.uv.y = 1.0 - vtf.uv.y;
    vtf.blurDir = blurDir.xy;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float2 uv : UV;
    float2 blurDir : BLURDIR;
};

SamplerState samp : register(s3);
Texture2D maskTex : register(t0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    //this will be our alpha sum
    float sum = 0.0;

    //apply blurring, using a 23-tap filter with predefined gaussian weights
    sum += maskTex.Sample(samp, vtf.uv + -11.0 * vtf.blurDir).a * 0.007249;
    sum += maskTex.Sample(samp, vtf.uv + -10.0 * vtf.blurDir).a * 0.011032;
    sum += maskTex.Sample(samp, vtf.uv + -9.0 * vtf.blurDir).a * 0.016133;
    sum += maskTex.Sample(samp, vtf.uv + -8.0 * vtf.blurDir).a * 0.022665;
    sum += maskTex.Sample(samp, vtf.uv + -7.0 * vtf.blurDir).a * 0.030595;
    sum += maskTex.Sample(samp, vtf.uv + -6.0 * vtf.blurDir).a * 0.039680;
    sum += maskTex.Sample(samp, vtf.uv + -5.0 * vtf.blurDir).a * 0.049444;
    sum += maskTex.Sample(samp, vtf.uv + -4.0 * vtf.blurDir).a * 0.059195;
    sum += maskTex.Sample(samp, vtf.uv + -3.0 * vtf.blurDir).a * 0.068091;
    sum += maskTex.Sample(samp, vtf.uv + -2.0 * vtf.blurDir).a * 0.075252;
    sum += maskTex.Sample(samp, vtf.uv + -1.0 * vtf.blurDir).a * 0.079905;
    sum += maskTex.Sample(samp, vtf.uv + 0.0 * vtf.blurDir).a * 0.081519;
    sum += maskTex.Sample(samp, vtf.uv + 1.0 * vtf.blurDir).a * 0.079905;
    sum += maskTex.Sample(samp, vtf.uv + 2.0 * vtf.blurDir).a * 0.075252;
    sum += maskTex.Sample(samp, vtf.uv + 3.0 * vtf.blurDir).a * 0.068091;
    sum += maskTex.Sample(samp, vtf.uv + 4.0 * vtf.blurDir).a * 0.059195;
    sum += maskTex.Sample(samp, vtf.uv + 5.0 * vtf.blurDir).a * 0.049444;
    sum += maskTex.Sample(samp, vtf.uv + 6.0 * vtf.blurDir).a * 0.039680;
    sum += maskTex.Sample(samp, vtf.uv + 7.0 * vtf.blurDir).a * 0.030595;
    sum += maskTex.Sample(samp, vtf.uv + 8.0 * vtf.blurDir).a * 0.022665;
    sum += maskTex.Sample(samp, vtf.uv + 9.0 * vtf.blurDir).a * 0.016133;
    sum += maskTex.Sample(samp, vtf.uv + 10.0 * vtf.blurDir).a * 0.011032;
    sum += maskTex.Sample(samp, vtf.uv + 11.0 * vtf.blurDir).a * 0.007249;

    return float4(1.0, 1.0, 1.0, sum);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
};

struct PhazonSuitBlurUniform
{
    float4 blurDir;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float2 uv;
    float2 blurDir;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant PhazonSuitBlurUniform& psu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.uv = v.uvIn.xy;
    vtf.uv.y = 1.0 - vtf.uv.y;
    vtf.blurDir = psu.blurDir.xy;
    vtf.position = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float2 uv;
    float2 blurDir;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler clampSamp [[ sampler(3) ]],
                      texture2d<float> maskTex [[ texture(0) ]])
{
    //this will be our alpha sum
    float sum = 0.0;

    //apply blurring, using a 23-tap filter with predefined gaussian weights
    sum += maskTex.sample(clampSamp, vtf.uv + -11.0 * vtf.blurDir).a * 0.007249;
    sum += maskTex.sample(clampSamp, vtf.uv + -10.0 * vtf.blurDir).a * 0.011032;
    sum += maskTex.sample(clampSamp, vtf.uv + -9.0 * vtf.blurDir).a * 0.016133;
    sum += maskTex.sample(clampSamp, vtf.uv + -8.0 * vtf.blurDir).a * 0.022665;
    sum += maskTex.sample(clampSamp, vtf.uv + -7.0 * vtf.blurDir).a * 0.030595;
    sum += maskTex.sample(clampSamp, vtf.uv + -6.0 * vtf.blurDir).a * 0.039680;
    sum += maskTex.sample(clampSamp, vtf.uv + -5.0 * vtf.blurDir).a * 0.049444;
    sum += maskTex.sample(clampSamp, vtf.uv + -4.0 * vtf.blurDir).a * 0.059195;
    sum += maskTex.sample(clampSamp, vtf.uv + -3.0 * vtf.blurDir).a * 0.068091;
    sum += maskTex.sample(clampSamp, vtf.uv + -2.0 * vtf.blurDir).a * 0.075252;
    sum += maskTex.sample(clampSamp, vtf.uv + -1.0 * vtf.blurDir).a * 0.079905;
    sum += maskTex.sample(clampSamp, vtf.uv).a * 0.081519;
    sum += maskTex.sample(clampSamp, vtf.uv + 1.0 * vtf.blurDir).a * 0.079905;
    sum += maskTex.sample(clampSamp, vtf.uv + 2.0 * vtf.blurDir).a * 0.075252;
    sum += maskTex.sample(clampSamp, vtf.uv + 3.0 * vtf.blurDir).a * 0.068091;
    sum += maskTex.sample(clampSamp, vtf.uv + 4.0 * vtf.blurDir).a * 0.059195;
    sum += maskTex.sample(clampSamp, vtf.uv + 5.0 * vtf.blurDir).a * 0.049444;
    sum += maskTex.sample(clampSamp, vtf.uv + 6.0 * vtf.blurDir).a * 0.039680;
    sum += maskTex.sample(clampSamp, vtf.uv + 7.0 * vtf.blurDir).a * 0.030595;
    sum += maskTex.sample(clampSamp, vtf.uv + 8.0 * vtf.blurDir).a * 0.022665;
    sum += maskTex.sample(clampSamp, vtf.uv + 9.0 * vtf.blurDir).a * 0.016133;
    sum += maskTex.sample(clampSamp, vtf.uv + 10.0 * vtf.blurDir).a * 0.011032;
    sum += maskTex.sample(clampSamp, vtf.uv + 11.0 * vtf.blurDir).a * 0.007249;

    return float4(1.0, 1.0, 1.0, sum);
}



