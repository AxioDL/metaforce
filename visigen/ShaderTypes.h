#ifndef ShaderTypes_h
#define ShaderTypes_h

#ifdef __METAL_VERSION__
#define NS_ENUM(_type, _name)                                                                                          \
  enum _name : _type _name;                                                                                            \
  enum _name : _type
#define NSInteger metal::int32_t
#else

#import <Foundation/Foundation.h>

#endif

#include <simd/simd.h>

typedef NS_ENUM(NSInteger, BufferIndex) {
  BufferIndexVertex = 0,
  BufferIndexUniforms = 1,
};

typedef NS_ENUM(NSInteger, VertexAttribute) {
  VertexAttributePosition = 0,
  VertexAttributeColor = 1,
};

typedef struct Uniforms {
  matrix_float4x4 projectionMatrix;
  matrix_float4x4 modelViewMatrix;
} Uniforms;

#endif /* ShaderTypes_h */
