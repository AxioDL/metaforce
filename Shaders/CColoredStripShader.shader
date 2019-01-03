#shader CColoredStripShader
#attribute position4
#attribute color
#attribute uv4
#srcfac srcalpha
#dstfac invsrcalpha
#primitive tristrips
#depthtest lequal
#depthwrite false
#culling none


#vertex glsl
layout(location=0) in vec4 posIn;
layout(location=1) in vec4 colorIn;
layout(location=2) in vec4 uvIn;

UBINDING0 uniform ColoredQuadUniform
{
  mat4 xf;
  vec4 color;
};

struct VertToFrag
{
  vec4 color;
  vec2 uv;
};

SBINDING(0) out VertToFrag vtf;
void main()
{
  vtf.color = colorIn * color;
  vtf.uv = uvIn.xy;
  gl_Position = xf * vec4(posIn.xyz, 1.0);
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
  float4 colorIn : COLOR;
  float4 uvIn : UV;
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
  float2 uv : UV;
};

VertToFrag main(in VertData v)
{
  VertToFrag vtf;
  vtf.color = v.colorIn * color;
  vtf.uv = v.uvIn.xy;
  vtf.position = mul(xf, float4(v.posIn.xyz, 1.0));
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
  float4 posIn [[ attribute(0) ]];
  float4 colorIn [[ attribute(1) ]];
  float4 uvIn [[ attribute(2) ]];
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
  float2 uv;
};

vertex VertToFrag vmain(VertData v [[ stage_in ]], constant ColoredQuadUniform& cqu [[ buffer(2) ]])
{
  VertToFrag vtf;
  vtf.color = v.colorIn * cqu.color;
  vtf.uv = v.uvIn.xy;
  vtf.position = cqu.xf * float4(v.posIn.xyz, 1.0);
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

#shader CColoredStripShaderAdditive : CColoredStripShader
#srcfac srcalpha
#dstfac one

#shader CColoredStripShaderFullAdditive : CColoredStripShader
#srcfac one
#dstfac one
