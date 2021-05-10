#include <metal_stdlib>
#include <simd/simd.h>

#include "ShaderTypes.h"

using namespace metal;

typedef struct
{
    float4 position [[position]];
    float4 color;
} ColorInOut;

typedef struct
{
    float3 position [[attribute(VertexAttributePosition)]];
    float4 color [[attribute(VertexAttributeColor)]];
} Vertex;

vertex ColorInOut vertexShader(Vertex in [[stage_in]], constant Uniforms& uniforms [[buffer(BufferIndexUniforms)]])
{
    ColorInOut out;

    float4 position = float4(in.position, 1.0);
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.color = in.color;

    return out;
}

fragment float4 fragmentShader(ColorInOut in [[stage_in]])
{
    return in.color;
}
