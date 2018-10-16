#shader CParticleSwooshShaderTexZWrite
#attribute position4
#attribute uv4
#attribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite true
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 uvIn;
layout(location=2) in vec4 colorIn;

UBINDING0 uniform SwooshUniform
{
    mat4 mvp;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn;
    vtf.uv = uvIn.xy;
    gl_Position = mvp * vec4(posIn.xyz, 1.0);
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
    float4 posIn : POSITION;
    float4 uvIn : UV;
    float4 colorIn : COLOR;
};

cbuffer SwooshUniform : register(b0)
{
    float4x4 mvp;
};

struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn.xy;
    vtf.pos = mul(mvp, float4(v.posIn.xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
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
    float4 posIn [[ attribute(0) ]];
    float4 uvIn [[ attribute(1) ]];
    float4 colorIn [[ attribute(2) ]];
};

struct SwooshUniform
{
    float4x4 mvp;
};

struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant SwooshUniform& su [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn.xy;
    vtf.pos = su.mvp * float4(v.posIn.xyz, 1.0);
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
                      texture2d<float> tex [[ texture(0) ]])
{
    return vtf.color * tex.sample(samp, vtf.uv);
}


#shader CParticleSwooshShaderTexNoZWrite : CParticleSwooshShaderTexZWrite
#srcfac srcalpha
#dstfac invsrcalpha
#depthwrite false

#shader CParticleSwooshShaderTexAdditiveZWrite : CParticleSwooshShaderTexZWrite
#srcfac srcalpha
#dstfac one
#depthwrite true

#shader CParticleSwooshShaderTexAdditiveNoZWrite : CParticleSwooshShaderTexZWrite
#srcfac srcalpha
#dstfac one
#depthwrite false

#shader CParticleSwooshShaderNoTexZWrite : CParticleSwooshShaderTexZWrite
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal
#depthwrite true

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vtf.color;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]])
{
    return vtf.color;
}


#shader CParticleSwooshShaderNoTexNoZWrite : CParticleSwooshShaderNoTexZWrite
#srcfac srcalpha
#dstfac invsrcalpha
#depthwrite false

#shader CParticleSwooshShaderNoTexAdditiveZWrite : CParticleSwooshShaderNoTexZWrite
#srcfac srcalpha
#dstfac one
#depthwrite true

#shader CParticleSwooshShaderNoTexAdditiveNoZWrite : CParticleSwooshShaderNoTexZWrite
#srcfac srcalpha
#dstfac one
#depthwrite false

