#shader CMoviePlayerShader
#attribute position4
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec3 posIn;
layout(location=1) in vec2 uvIn;
UBINDING0 uniform ViewBlock
{
    mat4 mv;
    vec4 mulColor;
};
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};
SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uv = uvIn;
    vtf.color = mulColor;
    gl_Position = mv * vec4(posIn, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};
SBINDING(0) in VertToFrag vtf;
TBINDING0 uniform sampler2D texY;
TBINDING1 uniform sampler2D texU;
TBINDING2 uniform sampler2D texV;
layout(location=0) out vec4 colorOut;
void main()
{
    vec3 yuv;
    yuv.r = texture(texY, vtf.uv).r;
    yuv.g = texture(texU, vtf.uv).r;
    yuv.b = texture(texV, vtf.uv).r;
    yuv.r = 1.1643*(yuv.r-0.0625);
    yuv.g = yuv.g-0.5;
    yuv.b = yuv.b-0.5;
    colorOut = vec4(yuv.r+1.5958*yuv.b,
                    yuv.r-0.39173*yuv.g-0.81290*yuv.b,
                    yuv.r+2.017*yuv.g, 1.0) * vtf.color;
}

#vertex hlsl
struct VertData
{
    float3 posIn : POSITION;
    float2 uvIn : UV;
};
cbuffer ViewBlock : register(b0)
{
    float4x4 mv;
    float4 mulColor;
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
    vtf.uv = v.uvIn;
    vtf.color = mulColor;
    vtf.position = mul(mv, float4(v.posIn, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};
Texture2D texs[3] : register(t0);
SamplerState samp : register(s0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float3 yuv;
    yuv.r = texs[0].Sample(samp, vtf.uv).r;
    yuv.g = texs[1].Sample(samp, vtf.uv).r;
    yuv.b = texs[2].Sample(samp, vtf.uv).r;
    yuv.r = 1.1643*(yuv.r-0.0625);
    yuv.g = yuv.g-0.5;
    yuv.b = yuv.b-0.5;
    return float4(yuv.r+1.5958*yuv.b,
                  yuv.r-0.39173*yuv.g-0.81290*yuv.b,
                  yuv.r+2.017*yuv.g, 1.0) * vtf.color;
}

#vertex metal
struct VertData
{
    float3 posIn [[ attribute(0) ]];
    float2 uvIn [[ attribute(1) ]];
};
struct ViewBlock
{
    float4x4 mv;
    float4 mulColor;
};
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};
vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ViewBlock& view [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.uv = v.uvIn;
    vtf.color = view.mulColor;
    vtf.position = view.mv * float4(v.posIn, 1.0);
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
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> tex0 [[ texture(0) ]],
                      texture2d<float> tex1 [[ texture(1) ]],
                      texture2d<float> tex2 [[ texture(2) ]])
{
    float3 yuv;
    yuv.r = tex0.sample(samp, vtf.uv).r;
    yuv.g = tex1.sample(samp, vtf.uv).r;
    yuv.b = tex2.sample(samp, vtf.uv).r;
    yuv.r = 1.1643*(yuv.r-0.0625);
    yuv.g = yuv.g-0.5;
    yuv.b = yuv.b-0.5;
    return float4(yuv.r+1.5958*yuv.b,
                  yuv.r-0.39173*yuv.g-0.81290*yuv.b,
                  yuv.r+2.017*yuv.g, 1.0) * vtf.color;
}
