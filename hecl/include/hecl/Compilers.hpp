#pragma once
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/Vulkan.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"

namespace hecl
{

namespace PlatformType
{
using PlatformEnum = boo::IGraphicsDataFactory::Platform;
struct Null {};
struct OpenGL { static constexpr PlatformEnum Enum = PlatformEnum::OpenGL; static const char* Name;
#if BOO_HAS_GL
using Context = boo::GLDataFactory::Context;
#endif
};
inline const char* OpenGL::Name = "OpenGL";
struct D3D11 { static constexpr PlatformEnum Enum = PlatformEnum::D3D11; static const char* Name;
#if _WIN32
using Context = boo::D3DDataFactory::Context;
#endif
};
inline const char* D3D11::Name = "D3D11";
struct Metal { static constexpr PlatformEnum Enum = PlatformEnum::Metal; static const char* Name;
#if BOO_HAS_METAL
using Context = boo::MetalDataFactory::Context;
#endif
};
inline const char* Metal::Name = "Metal";
struct Vulkan { static constexpr PlatformEnum Enum = PlatformEnum::Vulkan; static const char* Name;
#if BOO_HAS_VULKAN
using Context = boo::VulkanDataFactory::Context;
#endif
};
inline const char* Vulkan::Name = "Vulkan";
struct NX { static constexpr PlatformEnum Enum = PlatformEnum::NX; static const char* Name;
#if BOO_HAS_NX
using Context = boo::NXDataFactory::Context;
#endif
};
inline const char* NX::Name = "NX";
}

namespace PipelineStage
{
using StageEnum = boo::PipelineStage;
struct Null { static constexpr StageEnum Enum = StageEnum::Null; static const char* Name; };
inline const char* Null::Name = "Null";
struct Vertex { static constexpr StageEnum Enum = StageEnum::Vertex; static const char* Name; };
inline const char* Vertex::Name = "Vertex";
struct Fragment { static constexpr StageEnum Enum = StageEnum::Fragment; static const char* Name; };
inline const char* Fragment::Name = "Fragment";
struct Geometry { static constexpr StageEnum Enum = StageEnum::Geometry; static const char* Name; };
inline const char* Geometry::Name = "Geometry";
struct Control { static constexpr StageEnum Enum = StageEnum::Control; static const char* Name; };
inline const char* Control::Name = "Control";
struct Evaluation { static constexpr StageEnum Enum = StageEnum::Evaluation; static const char* Name; };
inline const char* Evaluation::Name = "Evaluation";
}

template<typename P, typename S>
std::pair<std::shared_ptr<uint8_t[]>, size_t> CompileShader(std::string_view text);

}