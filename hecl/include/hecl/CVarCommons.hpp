#ifndef CVARCOMMONS_HPP
#define CVARCOMMONS_HPP

#include "CVarManager.hpp"

#undef min
#undef max

namespace hecl
{

using namespace std::literals;

#ifdef _WIN32
#define DEFAULT_GRAPHICS_API "D3D11"sv
#elif defined(__APPLE__)
#define DEFAULT_GRAPHICS_API "Metal"sv
#else
#define DEFAULT_GRAPHICS_API "OpenGL"sv
#endif

struct CVarCommons
{
    CVarManager& m_mgr;
    CVar* m_graphicsApi = nullptr;
    CVar* m_drawSamples = nullptr;
    CVar* m_texAnisotropy = nullptr;
    CVar* m_deepColor = nullptr;

    CVarCommons(CVarManager& manager) : m_mgr(manager)
    {
        m_graphicsApi = m_mgr.findOrMakeCVar("graphicsApi"sv,
            "API to use for rendering graphics"sv,
            DEFAULT_GRAPHICS_API, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_drawSamples = m_mgr.findOrMakeCVar("drawSamples"sv,
            "Number of MSAA samples to use for render targets"sv,
            1, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_texAnisotropy = m_mgr.findOrMakeCVar("texAnisotropy"sv,
            "Number of anisotropic samples to use for sampling textures"sv,
            1, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_deepColor = m_mgr.findOrMakeCVar("deepColor"sv,
            "Allow framebuffer with color depth greater-then 24-bits"sv,
            false, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
    }

    std::string getGraphicsApi() const
    {
        return m_graphicsApi->toLiteral();
    }

    void setGraphicsApi(std::string_view api)
    {
        m_graphicsApi->fromLiteral(api);
    }

    uint32_t getSamples() const
    {
        return std::max(uint32_t(1), uint32_t(m_drawSamples->toInteger()));
    }

    void setSamples(uint32_t v)
    {
        m_drawSamples->fromInteger(std::max(uint32_t(1), v));
    }

    uint32_t getAnisotropy() const
    {
        return std::max(uint32_t(1), uint32_t(m_texAnisotropy->toInteger()));
    }

    void setAnisotropy(uint32_t v)
    {
        m_texAnisotropy->fromInteger(std::max(uint32_t(1), v));
    }

    bool getDeepColor() const
    {
        return m_deepColor->toBoolean();
    }

    void setDeepColor(bool b)
    {
        m_deepColor->fromBoolean(b);
    }

    void serialize()
    {
        m_mgr.serialize();
    }
};

}

#endif // CVARCOMMONS_HPP
