#shader CColoredQuadFilter
#attribute position4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest none
#depthwrite false
#culling none


#vertex glsl
layout(location=0) in vec4 posIn;

UBINDING0 uniform ColoredQuadUniform
{
    mat4 xf;
    vec4 color;
};

struct VertToFrag
{
    vec4 color;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vtf.color = color;
    gl_Position = xf * vec4(posIn.xyz, 1.0);
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vtf.color;
}


#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
};

cbuffer ColoredQuadUniform : register(b0)
{
    float4x4 xf;
    float4 color;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertToFrag main(in VertData v)
{
    VertToFrag vtf;
    vtf.color = color;
    vtf.position = mul(xf, float4(v.posIn.xyz, 1.0));
    return vtf;
}

#fragment hlsl
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color;
}


#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
};

struct ColoredQuadUniform
{
    float4x4 xf;
    float4 color;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ColoredQuadUniform& cqu [[ buffer(2) ]])
{
    VertToFrag vtf;
    vtf.color = cqu.color;
    vtf.position = cqu.xf * float4(v.posIn.xyz, 1.0);
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]])
{
    return vtf.color;
}


#shader CColoredQuadFilterAdd : CColoredQuadFilter
#srcfac srcalpha
#dstfac one

#shader CColoredQuadFilterMul : CColoredQuadFilter
#srcfac zero
#dstfac srccolor
#overwritealpha true
