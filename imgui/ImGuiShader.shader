#shader ImGuiShader
#attribute position2
#attribute uv2
#attribute colorunorm
#srcfac srcalpha
#dstfac invsrcalpha
#primitive triangles
#depthtest none
#depthwrite false
#culling none


#vertex glsl
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
UBINDING0 uniform ImGuiShaderUniform
{
    mat4 xf;
};
struct VertToFrag
{
    vec2 uv;
    vec4 color;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.uv = uv;
    vtf.color = color;
    gl_Position = xf * vec4(pos.xy,0,1);
}

#fragment glsl
struct VertToFrag
{
    vec2 uv;
    vec4 color;
};

SBINDING(0) in VertToFrag vtf;
layout (location = 0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = vtf.color * texture(tex, vtf.uv.st);
}


#vertex hlsl
cbuffer ImGuiShaderUniform : register(b0)
{
  float4x4 xf;
};

struct VertData
{
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};

struct VertToFrag
{
  float4 pos : SV_POSITION;
  float2 uv  : TEXCOORD0;
  float4 col : COLOR0;
};

VertToFrag main(in VS_INPUT v)
{
    VertToFrag vtf;
    vtf.pos = mul(xf, float4(v.pos.xy, 0.f, 1.f));
    vtf.col = v.col;
    vtf.uv  = v.uv;
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.col * tex0.Sample(samp, vtf.uv);
}


#vertex metal
struct ImGuiShaderUniform {
    float4x4 xf;
};

struct VertexIn {
    float2 position  [[attribute(0)]];
    float2 texCoords [[attribute(1)]];
    uchar4 color     [[attribute(2)]];
};

struct VertToFrag {
    float4 position [[position]];
    float2 texCoords;
    float4 color;
};

vertex VertToFrag vmain(VertexIn v [[stage_in]],
                        constant ImGuiShaderUniform& u [[buffer(2)]]) {
    VertToFrag vtf;
    vtf.position = u.xf * float4(v.position, 0, 1);
    vtf.texCoords = v.texCoords;
    vtf.color = float4(v.color) / float4(255.0);
    return vtf;
}

#fragment metal
struct VertToFrag {
    float4 position [[position]];
    float2 texCoords;
    float4 color;
};

fragment half4 fmain(VertToFrag vtf [[stage_in]],
                     texture2d<half, access::sample> tex [[texture(0)]]) {
    constexpr sampler linearSampler(coord::normalized, min_filter::linear, mag_filter::linear, mip_filter::linear);
    half4 texColor = tex.sample(linearSampler, vtf.texCoords);
    return half4(vtf.color) * texColor;
}
