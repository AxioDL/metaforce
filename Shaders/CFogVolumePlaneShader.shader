#shader CFogVolumePlaneShader0
#attribute position4
#srcfac zero
#dstfac zero
#primitive tristrips
#depthtest lequal
#depthwrite true
#colorwrite false
#alphawrite true
#culling frontface

#vertex glsl
layout(location=0) in vec4 posIn;
void main()
{
    gl_Position = posIn;
}

#fragment glsl
layout(location=0) out vec4 colorOut;
void main()
{
    colorOut = vec4(1.0);
}

#vertex hlsl
struct VertData
{
    float4 posIn : POSITION;
};

float4 main(in VertData v) : SV_Position
{
    return v.posIn;
}

#fragment hlsl
float4 main() : SV_Target0
{
    return float4(1.0, 1.0, 1.0, 1.0);
}

#vertex metal
struct VertData
{
    float4 posIn [[ attribute(0) ]];
};

struct VertToFrag
{
    float4 position [[ position ]];
};

vertex VertToFrag vmain(VertData v [[ stage_in ]])
{
    VertToFrag vtf;
    vtf.position = v.posIn;
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]])
{
    return float4(1.0, 1.0, 1.0, 1.0);
}

#shader CFogVolumePlaneShader1 : CFogVolumePlaneShader0
#depthtest none
#depthwrite false
#colorwrite false
#alphawrite true
#culling frontface

#shader CFogVolumePlaneShader2 : CFogVolumePlaneShader0
#depthtest lequal
#depthwrite true
#colorwrite false
#alphawrite true
#culling backface

#shader CFogVolumePlaneShader3 : CFogVolumePlaneShader0
#depthtest greater
#depthwrite false
#colorwrite false
#alphawrite true
#culling backface
