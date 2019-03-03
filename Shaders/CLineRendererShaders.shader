#shader CLineRendererShaderTexAlpha
#attribute position4
#attribute color
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#alphawrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;
layout(location=2) in vec4 uvIn;

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
    gl_Position = posIn;
}

#fragment glsl
struct Fog
{
    vec4 color;
    float A;
    float B;
    float C;
    int mode;
};

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

vec4 MainPostFunc(vec4 colorIn)
{
    float fogZ;
    float fogF = clamp((fog.A / (fog.B - gl_FragCoord.z)) - fog.C, 0.0, 1.0);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return vec4(mix(colorIn, fog.color, clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = MainPostFunc(vtf.color * moduColor * texture(tex, vtf.uv));
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 colorIn : COLOR;
    float4 uvIn : UV;
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
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn.xy;
    vtf.position = v.posIn;
    return vtf;
}

#fragment hlsl
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
    Fog fog;
};

SamplerState samp : register(s3);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)
{
    float fogZ;
    float fogF = saturate((fog.A / (fog.B - (1.0 - vtf.position.z))) - fog.C);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(lerp(colorIn, fog.color, saturate(fogZ)).rgb, colorIn.a);
}

float4 main(in VertToFrag vtf) : SV_Target0
{
    return MainPostFunc(vtf, vtf.color * moduColor * tex0.Sample(samp, vtf.uv));
}

#vertex metal
struct VertData
{
    float4 posIn;
    float4 colorIn;
    float4 uvIn;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],
                        uint vertId [[ vertex_id ]])
{
    VertToFrag vtf;
    constant VertData& v = va[vertId];
    vtf.color = v.colorIn;
    vtf.uv = v.uvIn.xy;
    vtf.position = v.posIn;
    return vtf;
}

#fragment metal
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

struct LineUniform
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

static float4 MainPostFunc(thread VertToFrag& vtf, constant LineUniform& line, float4 colorIn)
{
    float fogZ;
    float fogF = saturate((line.fog.A / (line.fog.B - vtf.position.z)) - line.fog.C);
    switch (line.fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(mix(colorIn, line.fog.color, saturate(fogZ)).rgb, colorIn.a);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> tex0 [[ texture(0) ]],
                      constant LineUniform& line [[ buffer(2) ]])
{
    return MainPostFunc(vtf, line, vtf.color * line.moduColor * tex0.sample(samp, vtf.uv));
}

#shader CLineRendererShaderTexAlphaAWrite : CLineRendererShaderTexAlpha
#alphawrite true

#shader CLineRendererShaderTexAdditive : CLineRendererShaderTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest none
#alphawrite false

#fragment glsl
struct Fog
{
    vec4 color;
    float A;
    float B;
    float C;
    int mode;
};

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

vec4 MainPostFunc(vec4 colorIn)
{
    float fogZ;
    float fogF = clamp((fog.A / (fog.B - gl_FragCoord.z)) - fog.C, 0.0, 1.0);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return vec4(mix(colorIn, vec4(0.0), clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = MainPostFunc(vtf.color * moduColor * texture(tex, vtf.uv));
}

#fragment hlsl
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
    Fog fog;
};

SamplerState samp : register(s3);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

static float4 MainPostFunc(in VertToFrag vtf, float4 colorIn)
{
    float fogZ;
    float fogF = saturate((fog.A / (fog.B - (1.0 - vtf.position.z))) - fog.C);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(lerp(colorIn, float4(0.0,0.0,0.0,0.0), saturate(fogZ)).rgb, colorIn.a);
}

float4 main(in VertToFrag vtf) : SV_Target0
{
    return MainPostFunc(vtf, vtf.color * moduColor * tex0.Sample(samp, vtf.uv));
}

#fragment metal
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

struct LineUniform
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

static float4 MainPostFunc(thread VertToFrag& vtf, constant LineUniform& line, float4 colorIn)
{
    float fogZ;
    float fogF = saturate((line.fog.A / (line.fog.B - vtf.position.z)) - line.fog.C);
    switch (line.fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(mix(colorIn, float4(0.0), saturate(fogZ)).rgb, colorIn.a);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(3) ]],
                      texture2d<float> tex0 [[ texture(0) ]],
                      constant LineUniform& line [[ buffer(2) ]])
{
    return MainPostFunc(vtf, line, vtf.color * line.moduColor * tex0.sample(samp, vtf.uv));
}

#shader CLineRendererShaderTexAdditiveAWrite : CLineRendererShaderTexAdditive
#alphawrite true

#shader CLineRendererShaderTexAlphaZ : CLineRendererShaderTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal
#alphawrite false

#shader CLineRendererShaderTexAlphaZAWrite : CLineRendererShaderTexAlphaZ
#alphawrite true

#shader CLineRendererShaderTexAdditiveZ : CLineRendererShaderTexAdditive
#srcfac srcalpha
#dstfac one
#depthtest lequal
#alphawrite false

#shader CLineRendererShaderTexAdditiveZAWrite : CLineRendererShaderTexAdditiveZ
#alphawrite true

#shader CLineRendererShaderNoTexAlpha
#attribute position4
#attribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#alphawrite false
#culling none

#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;

struct VertToFrag
{
    vec4 color;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = colorIn;
    gl_Position = posIn;
}

#fragment glsl
struct Fog
{
    vec4 color;
    float A;
    float B;
    float C;
    int mode;
};

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    vec4 color;
};

vec4 MainPostFunc(vec4 colorIn)
{
    float fogZ;
    float fogF = clamp((fog.A / (fog.B - gl_FragCoord.z)) - fog.C, 0.0, 1.0);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return vec4(mix(colorIn, fog.color, clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = MainPostFunc(vtf.color * moduColor);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float4 colorIn : COLOR;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = v.colorIn;
    vtf.position = v.posIn;
    return vtf;
}

#fragment hlsl
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

static float4 MainPostFunc(float4 colorIn, float4 FragCoord)
{
    float fogZ;
    float fogF = saturate((fog.A / (fog.B - (1.0 - FragCoord.z))) - fog.C);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(lerp(colorIn, fog.color, saturate(fogZ)).rgb, colorIn.a);
}

float4 main(in VertToFrag vtf) : SV_Target0
{
    return MainPostFunc(vtf.color * moduColor, vtf.position);
}

#vertex metal
struct VertData
{
    float4 posIn;
    float4 colorIn;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],
                        uint vertId [[ vertex_id ]])
{
    VertToFrag vtf;
    constant VertData& v = va[vertId];
    vtf.color = v.colorIn;
    vtf.position = v.posIn;
    return vtf;
}

#fragment metal
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

struct LineUniform
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

static float4 MainPostFunc(float4 colorIn, constant LineUniform& line, float FragCoord)
{
    float fogZ;
    float fogF = saturate((line.fog.A / (line.fog.B - FragCoord.z)) - line.fog.C);
    switch (line.fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(mix(colorIn, line.fog.color, saturate(fogZ)).rgb, colorIn.a);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      constant LineUniform& line [[ buffer(2) ]])
{
    return MainPostFunc(vtf.color * line.moduColor, line, vtf.position);
}

#shader CLineRendererShaderNoTexAlphaAWrite : CLineRendererShaderNoTexAlpha
#alphawrite true

#shader CLineRendererShaderNoTexAdditive : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac one
#depthtest none
#alphawrite false

#fragment glsl
struct Fog
{
    vec4 color;
    float A;
    float B;
    float C;
    int mode;
};

UBINDING0 uniform LineUniform
{
    vec4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    vec4 color;
};

vec4 MainPostFunc(vec4 colorIn)
{
    float fogZ;
    float fogF = clamp((fog.A / (fog.B - gl_FragCoord.z)) - fog.C, 0.0, 1.0);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return vec4(mix(colorIn, vec4(0.0), clamp(fogZ, 0.0, 1.0)).rgb, colorIn.a);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = MainPostFunc(vtf.color * moduColor);
}

#fragment hlsl
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

cbuffer LineUniform : register(b0)
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

static float4 MainPostFunc(float4 colorIn, float4 FragCoord)
{
    float fogZ;
    float fogF = saturate((fog.A / (fog.B - (1.0 - FragCoord.z))) - fog.C);
    switch (fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(lerp(colorIn, float4(0.0,0.0,0.0,0.0), saturate(fogZ)).rgb, colorIn.a);
}

float4 main(in VertToFrag vtf) : SV_Target0
{
    return MainPostFunc(vtf.color * moduColor, vtf.position);
}

#fragment metal
struct Fog
{
    float4 color;
    float A;
    float B;
    float C;
    int mode;
};

struct LineUniform
{
    float4 moduColor;
    Fog fog;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

static float4 MainPostFunc(float4 colorIn, constant LineUniform& line, float FragCoord)
{
    float fogZ;
    float fogF = saturate((line.fog.A / (line.fog.B - FragCoord.z)) - line.fog.C);
    switch (line.fog.mode)
    {
    case 2:
        fogZ = fogF;
        break;
    case 4:
        fogZ = 1.0 - exp2(-8.0 * fogF);
        break;
    case 5:
        fogZ = 1.0 - exp2(-8.0 * fogF * fogF);
        break;
    case 6:
        fogZ = exp2(-8.0 * (1.0 - fogF));
        break;
    case 7:
        fogF = 1.0 - fogF;
        fogZ = exp2(-8.0 * fogF * fogF);
        break;
    default:
        fogZ = 0.0;
        break;
    }
    return float4(mix(colorIn, float4(0.0), saturate(fogZ)).rgb, colorIn.a);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      constant LineUniform& line [[ buffer(2) ]])
{
    return MainPostFunc(vtf.color * line.moduColor, line, vtf.position);
}

#shader CLineRendererShaderNoTexAdditiveAWrite : CLineRendererShaderNoTexAdditive
#alphawrite true

#shader CLineRendererShaderNoTexAlphaZ : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal
#alphawrite false

#shader CLineRendererShaderNoTexAlphaZAWrite : CLineRendererShaderNoTexAlphaZ
#alphawrite true

#shader CLineRendererShaderNoTexAdditiveZ : CLineRendererShaderNoTexAdditive
#srcfac srcalpha
#dstfac one
#depthtest lequal
#alphawrite false

#shader CLineRendererShaderNoTexAdditiveZAWrite : CLineRendererShaderNoTexAdditiveZ
#alphawrite true

#shader CLineRendererShaderNoTexAlphaZGEqual : CLineRendererShaderNoTexAlpha
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest gequal
#alphawrite false

#shader CLineRendererShaderNoTexAlphaZGEqualAWrite : CLineRendererShaderNoTexAlphaZGEqual
#alphawrite true
