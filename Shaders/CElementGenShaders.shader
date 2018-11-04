#shader CElementGenShaderTexZTestZWrite
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite true
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;
layout(location=5) in vec4 uvsIn[4];

UBINDING0 uniform ParticleUniform
{
    mat4 mvp;
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 color;
    vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec4 pos = posIn[gl_VertexID];
    vtf.color = colorIn * moduColor;
    vtf.uv = uvsIn[gl_VertexID].xy;
    gl_Position = mvp * pos;
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
    float4 colorIn : COLOR;
    float4 uvsIn[4] : UV;
};

cbuffer ParticleUniform : register(b0)
{
    float4x4 mvp;
    float4 moduColor;
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
    vtf.color = v.colorIn * moduColor;
    vtf.uv = v.uvsIn[vertId].xy;
    vtf.position = mul(mvp, v.posIn[vertId]);
    return vtf;
}

#fragment hlsl
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return vtf.color * tex0.Sample(samp, vtf.uv);
}

#vertex metal
struct VertData
{
    float4 posIn[4];
    float4 colorIn;
    float4 uvsIn[4];
};

struct ParticleUniform
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float2 uv;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant ParticleUniform& particle [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * particle.moduColor;
    vtf.uv = v.uvsIn[vertId].xy;
    vtf.position = particle.mvp * v.posIn[vertId];
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
                      texture2d<float> tex0 [[ texture(0) ]])
{
    return vtf.color * tex0.sample(samp, vtf.uv);
}

#shader CElementGenShaderTexNoZTestZWrite : CElementGenShaderTexZTestZWrite
#depthtest none
#depthwrite true

#shader CElementGenShaderTexZTestNoZWrite : CElementGenShaderTexZTestZWrite
#depthtest lequal
#depthwrite false

#shader CElementGenShaderTexNoZTestNoZWrite : CElementGenShaderTexZTestZWrite
#depthtest none
#depthwrite false

#shader CElementGenShaderTexAdditiveZTest : CElementGenShaderTexZTestZWrite
#srcfac srcalpha
#dstfac one
#depthtest lequal
#depthwrite false

#shader CElementGenShaderTexAdditiveNoZTest : CElementGenShaderTexZTestZWrite
#srcfac srcalpha
#dstfac one
#depthtest none
#depthwrite false

#shader CElementGenShaderTexZTestNoZWriteSub : CElementGenShaderTexZTestZWrite
#srcfac subtract
#dstfac subtract
#depthtest lequal
#depthwrite false

#shader CElementGenShaderTexNoZTestNoZWriteSub : CElementGenShaderTexZTestZWrite
#srcfac subtract
#dstfac subtract
#depthtest none
#depthwrite false

#shader CElementGenShaderTexRedToAlphaZTest : CElementGenShaderTexZTestZWrite
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest lequal
#depthwrite false

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
    colorOut = vtf.color;
    colorOut.a = texture(tex, vtf.uv).r;
}

#fragment hlsl
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : UV;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    return float4(vtf.color.rgb, tex0.Sample(samp, vtf.uv).r);
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
                      texture2d<float> tex0 [[ texture(0) ]])
{
    return float4(vtf.color.rgb, tex0.sample(samp, vtf.uv).r);
}

#shader CElementGenShaderTexRedToAlphaNoZTest : CElementGenShaderTexRedToAlphaZTest
#depthtest none

#shader CElementGenShaderTexRedToAlphaZTestSub : CElementGenShaderTexRedToAlphaZTest
#srcfac subtract
#dstfac subtract
#depthtest lequal

#shader CElementGenShaderTexRedToAlphaNoZTestSub : CElementGenShaderTexRedToAlphaZTest
#srcfac subtract
#dstfac subtract
#depthtest none

#shader CElementGenShaderIndTexZWrite
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#instattribute uv4 0
#instattribute uv4 1
#instattribute uv4 2
#instattribute uv4 3
#instattribute uv4 4
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest none
#depthwrite true

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;
layout(location=5) in vec4 uvsInTexrTind[4];
layout(location=9) in vec4 uvsInScene;

UBINDING0 uniform ParticleUniform
{
    mat4 mvp;
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 color;
    vec4 uvScene;
    vec2 uvTexr;
    vec2 uvTind;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec4 pos = posIn[gl_VertexID];
    vtf.color = colorIn * moduColor;
    vtf.uvScene = uvsInScene;
    vtf.uvTexr = uvsInTexrTind[gl_VertexID].xy;
    vtf.uvTind = uvsInTexrTind[gl_VertexID].zw;
    gl_Position = mvp * pos;
}

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec4 uvScene;
    vec2 uvTexr;
    vec2 uvTind;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D texrMap;
TBINDING1 uniform sampler2D sceneMap;
TBINDING2 uniform sampler2D tindMap;
void main()
{
    vec2 tindTexel = texture(tindMap, vtf.uvTind).zw;
    vec4 sceneTexel = texture(sceneMap, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    vec4 texrTexel = texture(texrMap, vtf.uvTexr);
    colorOut = vtf.color * vec4(sceneTexel.rgb, 1.0) + texrTexel;
    colorOut.a = vtf.color.a * texrTexel.a;
}

#vertex hlsl
struct VertData
{
    float4 posIn[4] : POSITION;
    float4 colorIn : COLOR;
    float4 uvsInTexrTind[4] : UV0;
    float4 uvsInScene : UV4;
};

cbuffer ParticleUniform : register(b0)
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 uvScene : UV0;
    float2 uvTexr : UV1;
    float2 uvTind : UV2;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.uvScene = v.uvsInScene;
    vtf.uvScene.y = 1.0 - vtf.uvScene.y;
    vtf.uvScene.w = 1.0 - vtf.uvScene.w;
    vtf.uvTexr = v.uvsInTexrTind[vertId].xy;
    vtf.uvTind = v.uvsInTexrTind[vertId].zw;
    vtf.position = mul(mvp, v.posIn[vertId]);
    return vtf;
}

#fragment hlsl
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 uvScene : UV0;
    float2 uvTexr : UV1;
    float2 uvTind : UV2;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    float2 tindTexel = tex2.Sample(samp, vtf.uvTind).zw;
    float4 sceneTexel = tex1.Sample(samp, lerp(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    float4 texrTexel = tex0.Sample(samp, vtf.uvTexr);
    float4 colorOut = vtf.color * float4(sceneTexel.rgb, 1.0) + texrTexel;
    colorOut.a = vtf.color.a * texrTexel.a;
    return colorOut;
}

#vertex metal
struct VertData
{
    float4 posIn[4];
    float4 colorIn;
    float4 uvsInTexrTind[4];
    float4 uvsInScene;
};

struct ParticleUniform
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float4 uvScene;
    float2 uvTexr;
    float2 uvTind;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant ParticleUniform& particle [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * particle.moduColor;
    vtf.uvScene = v.uvsInScene;
    vtf.uvScene.y = 1.0 - vtf.uvScene.y;
    vtf.uvScene.w = 1.0 - vtf.uvScene.w;
    vtf.uvTexr = v.uvsInTexrTind[vertId].xy;
    vtf.uvTind = v.uvsInTexrTind[vertId].zw;
    vtf.position = particle.mvp * v.posIn[vertId];
    return vtf;
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float4 uvScene;
    float2 uvTexr;
    float2 uvTind;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> tex0 [[ texture(0) ]],
                      texture2d<float> tex1 [[ texture(1) ]],
                      texture2d<float> tex2 [[ texture(2) ]])
{
    float2 tindTexel = tex2.sample(samp, vtf.uvTind).ba;
    float4 sceneTexel = tex1.sample(samp, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    float4 texrTexel = tex0.sample(samp, vtf.uvTexr);
    float4 colr = vtf.color * float4(sceneTexel.rgb, 1.0) + texrTexel;
    return float4(colr.rgb, vtf.color.a * texrTexel.a);
}

#shader CElementGenShaderIndTexNoZWrite : CElementGenShaderIndTexZWrite
#depthwrite false

#shader CElementGenShaderIndTexAdditive : CElementGenShaderIndTexZWrite
#depthwrite true
#srcfac srcalpha
#dstfac one

#shader CElementGenShaderCindTexZWrite : CElementGenShaderIndTexZWrite
#srcfac srcalpha
#dstfac invsrcalpha
#depthtest none
#depthwrite true

#fragment glsl
struct VertToFrag
{
    vec4 color;
    vec4 uvScene;
    vec2 uvTexr;
    vec2 uvTind;
};

SBINDING(0) in VertToFrag vtf;
layout(location=0) out vec4 colorOut;
TBINDING0 uniform sampler2D texrMap;
TBINDING1 uniform sampler2D sceneMap;
TBINDING2 uniform sampler2D tindMap;
void main()
{
    vec2 tindTexel = texture(tindMap, vtf.uvTind).zw;
    vec4 sceneTexel = texture(sceneMap, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    colorOut = vtf.color * vec4(sceneTexel.rgb, 1.0) * texture(texrMap, vtf.uvTexr);
}

#fragment hlsl
SamplerState samp : register(s0);
Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);
Texture2D tex2 : register(t2);
struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float4 uvScene : UV0;
    float2 uvTexr : UV1;
    float2 uvTind : UV2;
};

float4 main(in VertToFrag vtf) : SV_Target0
{
    float2 tindTexel = tex2.Sample(samp, vtf.uvTind).ba;
    float4 sceneTexel = tex1.Sample(samp, lerp(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    return vtf.color * float4(sceneTexel.rgb, 1.0) * tex0.Sample(samp, vtf.uvTexr);
}

#fragment metal
struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
    float4 uvScene;
    float2 uvTexr;
    float2 uvTind;
};

fragment float4 fmain(VertToFrag vtf [[ stage_in ]],
                      sampler samp [[ sampler(0) ]],
                      texture2d<float> tex0 [[ texture(0) ]],
                      texture2d<float> tex1 [[ texture(1) ]],
                      texture2d<float> tex2 [[ texture(2) ]])
{
    float2 tindTexel = tex2.sample(samp, vtf.uvTind).ba;
    float4 sceneTexel = tex1.sample(samp, mix(vtf.uvScene.xy, vtf.uvScene.zw, tindTexel));
    return vtf.color * float4(sceneTexel.rgb, 1.0) * tex0.sample(samp, vtf.uvTexr);
}

#shader CElementGenShaderCindTexNoZWrite : CElementGenShaderCindTexZWrite
#depthwrite false

#shader CElementGenShaderCindTexAdditive : CElementGenShaderCindTexZWrite
#depthwrite true
#srcfac srcalpha
#dstfac one

#shader CElementGenShaderNoTexZTestZWrite
#instattribute position4 0
#instattribute position4 1
#instattribute position4 2
#instattribute position4 3
#instattribute color
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite true
#culling none

#vertex glsl
layout(location=0) in vec4 posIn[4];
layout(location=4) in vec4 colorIn;

UBINDING0 uniform ParticleUniform
{
    mat4 mvp;
    vec4 moduColor;
};

struct VertToFrag
{
    vec4 color;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
    vec4 pos = posIn[gl_VertexID];
    vtf.color = colorIn * moduColor;
    gl_Position = mvp * pos;
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
    float4 posIn[4] : POSITION;
    float4 colorIn : COLOR;
};

cbuffer ParticleUniform : register(b0)
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertToFrag main(in VertData v, in uint vertId : SV_VertexID)
{
    VertToFrag vtf;
    vtf.color = v.colorIn * moduColor;
    vtf.position = mul(mvp, v.posIn[vertId]);
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
    float4 posIn[4];
    float4 colorIn;
};

struct ParticleUniform
{
    float4x4 mvp;
    float4 moduColor;
};

struct VertToFrag
{
    float4 position [[ position ]];
    float4 color;
};

vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],
                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],
                        constant ParticleUniform& particle [[ buffer(2) ]])
{
    VertToFrag vtf;
    constant VertData& v = va[instId];
    vtf.color = v.colorIn * particle.moduColor;
    vtf.position = particle.mvp * v.posIn[vertId];
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

#shader CElementGenShaderNoTexNoZTestZWrite : CElementGenShaderNoTexZTestZWrite
#depthtest none
#depthwrite true

#shader CElementGenShaderNoTexZTestNoZWrite : CElementGenShaderNoTexZTestZWrite
#depthtest lequal
#depthwrite false

#shader CElementGenShaderNoTexNoZTestNoZWrite : CElementGenShaderNoTexZTestZWrite
#depthtest none
#depthwrite false

#shader CElementGenShaderNoTexAdditiveZTest : CElementGenShaderNoTexZTestZWrite
#srcfac srcalpha
#dstfac one
#depthtest lequal
#depthwrite false

#shader CElementGenShaderNoTexAdditiveNoZTest : CElementGenShaderNoTexZTestZWrite
#srcfac srcalpha
#dstfac one
#depthtest none
#depthwrite false
