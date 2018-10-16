#shader CFogVolumeFilter1Way
#attribute position4
#attribute uv4
#srcfac dstalpha
#dstfac one
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec2 uvIn;

UBINDING0 uniform FogVolumeFilterUniform
{
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
    gl_Position = vec4(posIn.xy, 0.0, 1.0);
    vtf.color = color;
    vtf.uv = uvIn;
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D zFrontfaceTex;
TBINDING1 uniform sampler2D zBackfaceTex;
TBINDING2 uniform sampler2D zLinearizer;
void main()
{
    float y;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float x = modf(texture(zFrontfaceTex, vtf.uv).r * linScale, y);
    const float uvBias = 0.5 / 256.0;
    float alpha = texture(zLinearizer, vec2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;
    colorOut = vtf.color * alpha;
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float2 uvIn : UV;
};

cbuffer FogVolumeFilterUniform : register(b0)
{
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
    vtf.position = float4(v.posIn.x, -v.posIn.y, 0.0, 1.0);
    vtf.color = color;
    vtf.uv = v.uvIn;
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

Texture2D zFrontfaceTex : register(t0);
Texture2D zBackfaceTex : register(t1);
Texture2D zLinearizer : register(t2);
SamplerState samp : register(s0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float y;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float x = modf((1.0 - zFrontfaceTex.Sample(samp, vtf.uv).r) * linScale, y);
    const float uvBias = 0.5 / 256.0;
    float alpha = zLinearizer.Sample(samp, float2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;
    return vtf.color * alpha;
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float2 uvIn [[ attribute(1) ]];
};

struct FogVolumeFilterUniform
{
    float4 color;
};

struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]],
                        constant FogVolumeFilterUniform& fu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.pos = float4(v.posIn.x, -v.posIn.y, 0.0, 1.0);
    vtf.color = fu.color;
    vtf.uv = v.uvIn;
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> zFrontfaceTex [[ texture(0) ]],
                      texture2d<float> zBackfaceTex [[ texture(1) ]],
                      texture2d<float> zLinearizer [[ texture(2) ]])
{
    float y;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float x = modf((1.0 - zFrontfaceTex.sample(samp, vtf.uv).r) * linScale, y);
    const float uvBias = 0.5 / 256.0;
    float alpha = zLinearizer.sample(samp, float2(x * 255.0 / 256.0 + uvBias, y / 256.0 + uvBias)).r * 10.0;
    return vtf.color * alpha;
}


#shader CFogVolumeFilter2Way : CFogVolumeFilter1Way
#srcfac srcalpha
#dstfac one

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D zFrontfaceTex;
TBINDING1 uniform sampler2D zBackfaceTex;
TBINDING2 uniform sampler2D zLinearizer;
void main()
{
    float frontY;
    float backY;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float frontX = modf(texture(zFrontfaceTex, vtf.uv).r * linScale, frontY);
    float backX = modf(texture(zBackfaceTex, vtf.uv).r * linScale, backY);
    const float uvBias = 0.5 / 256.0;
    float frontLin = texture(zLinearizer, vec2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;
    float backLin = texture(zLinearizer, vec2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;
    colorOut = vec4(vtf.color.rgb, (frontLin - backLin) * 10.0);
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

Texture2D zFrontfaceTex : register(t0);
Texture2D zBackfaceTex : register(t1);
Texture2D zLinearizer : register(t2);
SamplerState samp : register(s0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float frontY;
    float backY;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float frontX = modf((1.0 - zFrontfaceTex.Sample(samp, vtf.uv).r) * linScale, frontY);
    float backX = modf((1.0 - zBackfaceTex.Sample(samp, vtf.uv).r) * linScale, backY);
    const float uvBias = 0.5 / 256.0;
    float frontLin = zLinearizer.Sample(samp, float2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;
    float backLin = zLinearizer.Sample(samp, float2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;
    return float4(vtf.color.rgb, (frontLin - backLin) * 10.0);
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> zFrontfaceTex [[ texture(0) ]],
                      texture2d<float> zBackfaceTex [[ texture(1) ]],
                      texture2d<float> zLinearizer [[ texture(2) ]])
{
    float frontY;
    float backY;
    const float linScale = 65535.0 / 65536.0 * 256.0;
    float frontX = modf((1.0 - zFrontfaceTex.sample(samp, vtf.uv).r) * linScale, frontY);
    float backX = modf((1.0 - zBackfaceTex.sample(samp, vtf.uv).r) * linScale, backY);
    const float uvBias = 0.5 / 256.0;
    float frontLin = zLinearizer.sample(samp, float2(frontX * 255.0 / 256.0 + uvBias, frontY / 256.0 + uvBias)).r;
    float backLin = zLinearizer.sample(samp, float2(backX * 255.0 / 256.0 + uvBias, backY / 256.0 + uvBias)).r;
    return float4(vtf.color.rgb, (frontLin - backLin) * 10.0);
}
