#shader CRandomStaticFilterAlpha
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

UBINDING0 uniform RandomStaticUniform
{
    vec4 color;
    float randOff;
    float discardThres;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float randOff;
    float discardThres;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = color;
    vtf.uv = uvIn.xy;
    vtf.randOff = randOff;
    vtf.discardThres = discardThres;
    gl_Position = vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float randOff;
    float discardThres;
};

ivec2 Lookup8BPP(in vec2 uv, in float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return ivec2(addr & 0x3ff, addr >> 10);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = texelFetch(tex, Lookup8BPP(vtf.uv, vtf.randOff), 0) * vtf.color;
    colorOut.a = vtf.color.a;
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
    float2 uvIn : UV;
};

cbuffer RandomStaticUniform : register(b0)
{
    float4 color;
    float randOff;
   float discardThres;
};

struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float randOff : RANDOFF;
    float discardThres : DISCARDTHRES;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = color;
    vtf.uv = v.uvIn.xy;
    vtf.randOff = randOff;
    vtf.discardThres = discardThres;
    vtf.pos = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float randOff : RANDOFF;
    float discardThres : DISCARDTHRES;
};

static int3 Lookup8BPP(float2 uv, float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return int3(addr & 0x3ff, addr >> 10, 0);
}

Texture2D tex : register(t0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 colorOut = tex.Load(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;
    colorOut.a = vtf.color.a;
    return colorOut;
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
    float2 uvIn [[ attribute(1) ]];
};

struct RandomStaticUniform
{
    float4 color;
    float randOff;
    float discardThres;
};

struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
    float randOff;
    float discardThres;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]],
                        constant RandomStaticUniform& su [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = su.color;
    vtf.uv = v.uvIn.xy;
    vtf.randOff = su.randOff;
    vtf.discardThres = su.discardThres;
    vtf.pos = float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
    float randOff;
    float discardThres;
};

static uint2 Lookup8BPP(float2 uv, float randOff)
{
    int bx = int(uv.x) >> 3;
    int rx = int(uv.x) & 0x7;
    int by = int(uv.y) >> 2;
    int ry = int(uv.y) & 0x3;
    int bidx = by * 128 + bx;
    int addr = bidx * 32 + ry * 8 + rx + int(randOff);
    return uint2(addr & 0x3ff, addr >> 10);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    float4 colorOut = tex.read(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;
    colorOut.a = vtf.color.a;
    return colorOut;
}

#shader CRandomStaticFilterAdd : CRandomStaticFilterAlpha
#srcfac srcalpha
#dstfac one

#shader CRandomStaticFilterMult : CRandomStaticFilterAlpha
#srcfac zero
#dstfac srccolor
#overwritealpha true

#shader CRandomStaticFilterCookieCutter : CRandomStaticFilterAlpha
#srcfac zero
#dstfac srccolor
#overwritealpha true
#depthwrite true
#colorwrite false
#depthtest lequal

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec2 uv;
    float randOff;
    float discardThres;
};

ivec2 Lookup8BPP(in vec2 uv, in float randOff)
{
    float bx;
    float rx = modf(uv.x / 8.0, bx) * 8.0;
    float by;
    float ry = modf(uv.y / 4.0, by) * 4.0;
    float bidx = by * 80.0 + bx;
    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;
    float y;
    float x = modf(addr / 1024.0, y) * 1024.0;
    return ivec2(x, y);
}

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D tex;
void main()
{
    colorOut = texelFetch(tex, Lookup8BPP(vtf.uv, vtf.randOff), 0) * vtf.color;
    if (colorOut.a < vtf.discardThres)
        discard;
}

#fragment hlsl
struct VertToFrag
{
    float4 pos : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
    float randOff : RANDOFF;
    float discardThres : DISCARDTHRES;
};

static int3 Lookup8BPP(float2 uv, float randOff)
{
    float bx;
    float rx = modf(uv.x / 8.0, bx) * 8.0;
    float by;
    float ry = modf(uv.y / 4.0, by) * 4.0;
    float bidx = by * 80.0 + bx;
    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;
    float y;
    float x = modf(addr / 1024.0, y) * 1024.0;
    return int3(x, y, 0);
}

Texture2D tex : register(t0);
float4 main(in VertToFrag vtf) : SV_Target0
{
    float4 colorOut = tex.Load(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;
    if (colorOut.a < vtf.discardThres)
        discard;
    return colorOut;
}

#fragment metal
struct VertToFrag
{
    float4 pos [[ position ]];
    float4 color;
    float2 uv;
    float randOff;
    float discardThres;
};

static uint2 Lookup8BPP(float2 uv, float randOff)
{
    float bx;
    float rx = modf(uv.x / 8.0, bx) * 8.0;
    float by;
    float ry = modf(uv.y / 4.0, by) * 4.0;
    float bidx = by * 80.0 + bx;
    float addr = bidx * 32.0 + ry * 8.0 + rx + randOff;
    float y;
    float x = modf(addr / 1024.0, y) * 1024.0;
    return uint2(x, y);
}

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      texture2d<float> tex [[ texture(0) ]])
{
    float4 colorOut = tex.read(Lookup8BPP(vtf.uv, vtf.randOff)) * vtf.color;
    if (colorOut.a < vtf.discardThres)
        discard_fragment();
    return colorOut;
}
