#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <boo/graphicsdev/Metal.hpp>
#include <boo/graphicsdev/Vulkan.hpp>

namespace hecl {

namespace PlatformType {
using PlatformEnum = boo::IGraphicsDataFactory::Platform;
struct Null {};
struct OpenGL {
  static constexpr PlatformEnum Enum = PlatformEnum::OpenGL;
  static constexpr char Name[] = "OpenGL";
#if BOO_HAS_GL
  using Context = boo::GLDataFactory::Context;
#endif
};
struct D3D11 {
  static constexpr PlatformEnum Enum = PlatformEnum::D3D11;
  static constexpr char Name[] = "D3D11";
#if _WIN32
  using Context = boo::D3D11DataFactory::Context;
#endif
};
struct Metal {
  static constexpr PlatformEnum Enum = PlatformEnum::Metal;
  static constexpr char Name[] = "Metal";
#if BOO_HAS_METAL
  using Context = boo::MetalDataFactory::Context;
#endif
};
struct Vulkan {
  static constexpr PlatformEnum Enum = PlatformEnum::Vulkan;
  static constexpr char Name[] = "Vulkan";
#if BOO_HAS_VULKAN
  using Context = boo::VulkanDataFactory::Context;
#endif
};
struct NX {
  static constexpr PlatformEnum Enum = PlatformEnum::NX;
  static constexpr char Name[] = "NX";
#if BOO_HAS_NX
  using Context = boo::NXDataFactory::Context;
#endif
};
} // namespace PlatformType

namespace PipelineStage {
using StageEnum = boo::PipelineStage;
struct Null {
  static constexpr StageEnum Enum = StageEnum::Null;
  static constexpr char Name[] = "Null";
};
struct Vertex {
  static constexpr StageEnum Enum = StageEnum::Vertex;
  static constexpr char Name[] = "Vertex";
};
struct Fragment {
  static constexpr StageEnum Enum = StageEnum::Fragment;
  static constexpr char Name[] = "Fragment";
};
struct Geometry {
  static constexpr StageEnum Enum = StageEnum::Geometry;
  static constexpr char Name[] = "Geometry";
};
struct Control {
  static constexpr StageEnum Enum = StageEnum::Control;
  static constexpr char Name[] = "Control";
};
struct Evaluation {
  static constexpr StageEnum Enum = StageEnum::Evaluation;
  static constexpr char Name[] = "Evaluation";
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
