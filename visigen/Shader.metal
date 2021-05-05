#include <metal_stdlib>
#include <simd/simd.h>

#include "ShaderTypes.h"

using namespace metal;

/*static const matrix_float4x4 LookMATs[] = {
    {// Forward
     {1.f, 0.f, 0.f, 0.f},
     {0.f, 0.f, 1.f, 0.f},
     {0.f, -1.f, 0.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
    {// Backward
     {-1.f, 0.f, 0.f, 0.f},
     {0.f, 0.f, 1.f, 0.f},
     {0.f, 1.f, 0.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
    {// Up
     {1.f, 0.f, 0.f, 0.f},
     {0.f, -1.f, 0.f, 0.f},
     {0.f, 0.f, -1.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
    {// Down
     {1.f, 0.f, 0.f, 0.f},
     {0.f, 1.f, 0.f, 0.f},
     {0.f, 0.f, 1.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
    {// Left
     {0.f, 1.f, 0.f, 0.f},
     {0.f, 0.f, 1.f, 0.f},
     {1.f, 0.f, 0.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
    {// Right
     {0.f, -1.f, 0.f, 0.f},
     {0.f, 0.f, 1.f, 0.f},
     {-1.f, 0.f, 0.f, 0.f},
     {0.f, 0.f, 0.f, 1.f}},
};*/

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
    position.y *= -1.f;
    out.position = uniforms.projectionMatrix * uniforms.modelViewMatrix * position;
    out.color = in.color;

    return out;
}

fragment float4 fragmentShader(ColorInOut in [[stage_in]])
{
    return in.color;
}