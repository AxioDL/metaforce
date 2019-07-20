#pragma once
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/Vulkan.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"

namespace hecl {

namespace PlatformType {
using PlatformEnum = boo::IGraphicsDataFactory::Platform;
struct Null {};
struct OpenGL {
  static constexpr PlatformEnum Enum = PlatformEnum::OpenGL;
  static const char* Name;
#if BOO_HAS_GL
  using Context = boo::GLDataFactory::Context;
#endif
};
struct D3D11 {
  static constexpr PlatformEnum Enum = PlatformEnum::D3D11;
  static const char* Name;
#if _WIN32
  using Context = boo::D3D11DataFactory::Context;
#endif
};
struct Metal {
  static constexpr PlatformEnum Enum = PlatformEnum::Metal;
  static const char* Name;
#if BOO_HAS_METAL
  using Context = boo::MetalDataFactory::Context;
#endif
};
struct Vulkan {
  static constexpr PlatformEnum Enum = PlatformEnum::Vulkan;
  static const char* Name;
#if BOO_HAS_VULKAN
  using Context = boo::VulkanDataFactory::Context;
#endif
};
struct NX {
  static constexpr PlatformEnum Enum = PlatformEnum::NX;
  static const char* Name;
#if BOO_HAS_NX
  using Context = boo::NXDataFactory::Context;
#endif
};
} // namespace PlatformType

namespace PipelineStage {
using StageEnum = boo::PipelineStage;
struct Null {
  static constexpr StageEnum Enum = StageEnum::Null;
  static const char* Name;
};
struct Vertex {
  static constexpr StageEnum Enum = StageEnum::Vertex;
  static const char* Name;
};
struct Fragment {
  static constexpr StageEnum Enum = StageEnum::Fragment;
  static const char* Name;
};
struct Geometry {
  static constexpr StageEnum Enum = StageEnum::Geometry;
  static const char* Name;
};
struct Control {
  static constexpr StageEnum Enum = StageEnum::Control;
  static const char* Name;
};
struct Evaluation {
  static constexpr StageEnum Enum = StageEnum::Evaluation;
  static const char* Name;
};
} // namespace PipelineStage

#ifdef _LIBCPP_VERSION
using StageBinaryData = std::shared_ptr<uint8_t>;
inline StageBinaryData MakeStageBinaryData(size_t sz) {
  return StageBinaryData(new uint8_t[sz], std::default_delete<uint8_t[]>{});
}
#else
using StageBinaryData = std::shared_ptr<uint8_t[]>;
inline StageBinaryData MakeStageBinaryData(size_t sz) { return StageBinaryData(new uint8_t[sz]); }
#endif

template <typename P, typename S>
std::pair<StageBinaryData, size_t> CompileShader(std::string_view text);

} // namespace hecl
