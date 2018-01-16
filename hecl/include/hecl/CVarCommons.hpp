#ifndef CVARCOMMONS_HPP
#define CVARCOMMONS_HPP

#include "CVarManager.hpp"

#undef min
#undef max

namespace hecl
{

using namespace std::literals;

#ifdef _WIN32
#define DEFAULT_GRAPHICS_API "D3D11"
#elif defined(__APPLE__)
#define DEFAULT_GRAPHICS_API "Metal"
#else
#define DEFAULT_GRAPHICS_API "OpenGL"
#endif

class CVarCommons
{
    CVarManager& m_mgr;
    std::string m_graphicsApi = DEFAULT_GRAPHICS_API;
    uint32_t m_drawSamples = 1;
    uint32_t m_texAnisotropy = 1;
    bool m_deepColor = false;
public:
    CVarCommons(CVarManager& manager) : m_mgr(manager)
    {
        m_mgr.findOrMakeCVar("graphicsApi"sv,
            "API to use for rendering graphics"sv,
            m_graphicsApi, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_mgr.findOrMakeCVar("drawSamples"sv,
            "Number of MSAA samples to use for render targets"sv,
            m_drawSamples, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_mgr.findOrMakeCVar("texAnisotropy"sv,
            "Number of anisotropic samples to use for sampling textures"sv,
            m_texAnisotropy, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
        m_mgr.findOrMakeCVar("deepColor"sv,
            "Allow framebuffer with color depth greater-then 24-bits"sv,
            m_deepColor, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
    }

    std::string getGraphicsApi() const
    {
        return m_graphicsApi;
    }

    void setGraphicsApi(std::string_view api)
    {
        m_graphicsApi = api;
    }

    uint32_t getSamples() const
    {
        return std::max(uint32_t(1), m_drawSamples);
    }

    void setSamples(uint32_t v)
    {
        m_drawSamples = std::max(uint32_t(1), v);
    }

    uint32_t getAnisotropy() const
    {
        return std::max(uint32_t(1), m_texAnisotropy);
    }

    void setAnisotropy(uint32_t v)
    {
        m_texAnisotropy = std::max(uint32_t(1), v);
    }

    bool getDeepColor() const
    {
        return m_deepColor;
    }

    void setDeepColor(bool b)
    {
        m_deepColor = b;
    }

    void serialize()
    {
        m_mgr.serialize();
    }
};

}

#endif // CVARCOMMONS_HPP
