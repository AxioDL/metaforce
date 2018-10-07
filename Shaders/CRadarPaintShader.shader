#shader CRadarPaintShader
#attribute position4 0
#attribute position4 1
#attribute position4 2
#attribute position4 3
#attribute uv4 0
#attribute uv4 1
#attribute uv4 2
#attribute uv4 3
#attribute color
#srcfac srcalpha
#dstfac one
#primitive tristrips
#depthtest none
#depthwrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 uvIn[4];
layout(location=8) in vec4 colorIn;

UBINDING0 uniform RadarPaintUniform
{
    mat4 xf;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec3 pos = posIn[gl_VertexID].xyz;
    vtf.uv = uvIn[gl_VertexID].xy;
    vtf.color = colorIn;
    gl_Position = xf * vec4(pos, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = vtf.color * texture(tex, vtf.uv);
}

#vertex hlsl
struct VertData
{
    float4 posIn[4] : POSITION;
    float4 uvIn[4] : UV;
    float4 colorIn : COLOR;
};

cbuffer RadarPaintUniform : register(b0)
{
    float4x4 xf;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn[vertId].xy;
    vtf.position = mul(xf, float4(v.posIn[vertId].xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

SamplerState samp : register(s0);
Texture2D tex : register(t0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * tex.Sample(samp, vtf.uv);
}

#vertex metal
struct VertData
{
    float4 posIn[4];
    float4 uvIn[4];
    float4 colorIn;
};

struct RadarPaintUniform
{
    float4x4 xf;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant RadarPaintUniform& rpu [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn[vertId].xy;
    vtf.position = rpu.xf * float4(v.posIn[vertId].xyz, 1.0);
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
                      texture2d<float> tex [[ texture(0) ]])
{
    return vtf.color * tex.sample(samp, vtf.uv);
}

