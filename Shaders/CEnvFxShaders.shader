#shader CEnvFxSnowShader
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#srcfac one
#dstfac one
#primitive tristrips
#depthtest lequal
#depthwrite false
#alphawrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;
layout(location=5) in vec4 uvsIn[4];

UBINDING0 uniform EnvFxUniform
{
    mat4 mv;
    mat4 proj;
    mat4 envMtx;
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 mvPos;
    vec4 color;
    vec2 uvFlake;
    vec2 uvEnv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec4 pos = posIn[gl_VertexID];
    vtf.color = colorIn * moduColor;
    vtf.uvFlake = uvsIn[gl_VertexID].xy;
    vtf.uvEnv = (envMtx * pos).xy;
    vtf.mvPos = mv * pos;
    gl_Position = proj * vtf.mvPos;
}

#fragment glsl
struct VertToFrag
{
    vec4 mvPos;
    vec4 color;
    vec2 uvFlake;
    vec2 uvEnv;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D texFlake;
TBINDING1 uniform sampler2D texEnv;

UBINDING1 uniform FogUniform
{
    int mode;
    vec4 color;
    float rangeScale;
    float start;
};

vec4 MainPostFunc(vec4 colorIn)
{
    float fogZ, temp;
    switch (mode)
    {
    case 2:
        fogZ = (-vtf.mvPos.z - start) * rangeScale;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - start) * rangeScale);
        break;
    case 5:
        temp = (-vtf.mvPos.z - start) * rangeScale;
        fogZ = 1.0 - exp2(-8.0 * temp * temp);
        break;
    case 6:
        fogZ = exp2(-8.0 * (start + vtf.mvPos.z) * rangeScale);
        break;
    case 7:
        temp = (start + vtf.mvPos.z) * rangeScale;
        fogZ = exp2(-8.0 * temp * temp);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return vec4(mix(colorIn, color, clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
}

void main()
{
    colorOut = MainPostFunc(vtf.color * texture(texFlake, vtf.uvFlake) * texture(texEnv, vtf.uvEnv));
}

#vertex hlsl
struct VertData
{
    float4 posIn[4] : POSITION;
    float4 colorIn : COLOR;
    float4 uvsIn[4] : UV;
};

cbuffer EnvFxUniform : register(b0)
{
    float4x4 mv;
    float4x4 proj;
    float4x4 envMtx;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 mvPos : POSITION;
    float4 color : COLOR;
    float2 uvFlake : UV0;
    float2 uvEnv : UV1;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.uvFlake = v.uvsIn[vertId].xy;
    vtf.uvEnv = mul(envMtx, v.posIn[vertId]).xy;
    vtf.mvPos = mul(mv, v.posIn[vertId]);
    vtf.position = mul(proj, vtf.mvPos);
    return vtf;
}

#fragment hlsl
SamplerState samp : register(s0);
SamplerState sampClamp : register(s3);
Texture2D texFlake : register(t0);
Texture2D texEnv : register(t1);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 mvPos : POSITION;
    float4 color : COLOR;
    float2 uvFlake : UV0;
    float2 uvEnv : UV1;
};

cbuffer FogUniform : register(b1)
{
    int mode;
    float4 color;
    float rangeScale;
    float start;
};

static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)
{
    float fogZ, temp;
    switch (mode)
    {
    case 2:
        fogZ = (-vtf.mvPos.z - start) * rangeScale;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - start) * rangeScale);
        break;
    case 5:
        temp = (-vtf.mvPos.z - start) * rangeScale;
        fogZ = 1.0 - exp2(-8.0 * temp * temp);
        break;
    case 6:
        fogZ = exp2(-8.0 * (start + vtf.mvPos.z) * rangeScale);
        break;
    case 7:
        temp = (start + vtf.mvPos.z) * rangeScale;
        fogZ = exp2(-8.0 * temp * temp);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(lerp(colorIn, color, saturate(fogZ)).rgb, colorIn.a);
}

float4 main(in VertToFrag vtf) : SV_Target0
{
    return MainPostFunc(vtf, vtf.color * texFlake.Sample(samp, vtf.uvFlake) * texEnv.Sample(sampClamp, vtf.uvEnv));
}

#vertex metal
struct VertData
{
    float4 posIn[4];
    float4 colorIn;
    float4 uvsIn[4];
};

struct EnvFxUniform
{
    float4x4 mv;
    float4x4 proj;
    float4x4 envMtx;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 mvPos;
    float4 color;
    float2 uvFlake;
    float2 uvEnv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant EnvFxUniform& particle [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * particle.moduColor;
    vtf.uvFlake = v.uvsIn[vertId].xy;
    vtf.uvEnv = (envMtx * v.posIn[vertId]).xy;
    vtf.mvPos = particle.mv * v.posIn[vertId];
    vtf.position = particle.proj * vtf.mvPos;
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 mvPos;
    float4 color;
    float2 uvFlake;
    float2 uvEnv;
};

struct FogUniform
{
    int mode;
    float4 color;
    float rangeScale;
    float start;
};

float4 MainPostFunc(thread VertToFrag& vtf, constant FogUniform& fu, float4 colorIn)
{
    float fogZ, temp;
    switch (lu.mode)
    {
    case 2:
        fogZ = (-vtf.mvPos.z - fu.start) * fu.rangeScale;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * (-vtf.mvPos.z - fu.start) * fu.rangeScale);
        break;
    case 5:
        temp = (-vtf.mvPos.z - fu.start) * fu.rangeScale;
        fogZ = 1.0 - exp2(-8.0 * temp * temp);
        break;
    case 6:
        fogZ = exp2(-8.0 * (fu.start + vtf.mvPos.z) * fu.rangeScale);
        break;
    case 7:
        temp = (fu.start + vtf.mvPos.z) * fu.rangeScale;
        fogZ = exp2(-8.0 * temp * temp);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(mix(colorIn, fu.color, saturate(fogZ)).rgb, colorIn.a);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      sampler sampClamp [[ sampler(3) ]],
                      constant FogUniform& fu [[ buffer(3) ]],
                      texture2d<float> texFlake [[ texture(0) ]],
                      texture2d<float> texEnv [[ texture(1) ]])
{
    return MainPostFunc(vtf, fu, vtf.color * texFlake.sample(samp, vtf.uvFlake) * texEnv.sample(sampClamp, vtf.uvEnv));
}

#shader CEnvFxUnderwaterShader : CEnvFxSnowShader
#srcfac srcalpha
#dstfac invsrcalpha
