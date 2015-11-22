#include "Specter/FontCache.hpp"
#include <LogVisor/LogVisor.hpp>
#include <stdint.h>

#include FT_GZIP_H
#include FT_SYSTEM_H
#include <freetype/internal/internal.h>
#include <freetype/internal/ftstream.h>

extern "C" const uint8_t DROIDSANS_PERMISSIVE[];
extern "C" size_t DROIDSANS_PERMISSIVE_SZ;

extern "C" const uint8_t BMONOFONT[];
extern "C" size_t BMONOFONT_SZ;

namespace Specter
{
static LogVisor::LogModule Log("Specter::FontCache");

FreeTypeGZipMemFace::FreeTypeGZipMemFace(FT_Library lib, const uint8_t* data, size_t sz)
{
    m_comp.base = (unsigned char*)data;
    m_comp.size = sz;
    m_comp.memory = lib->memory;
    if (FT_Stream_OpenGzip(&m_decomp, &m_comp))
        Log.report(LogVisor::FatalError, "unable to open FreeType gzip stream");

    FT_Open_Args args =
    {
        FT_OPEN_STREAM,
        nullptr,
        0,
        nullptr,
        &m_decomp
    };

    if (FT_Open_Face(lib, &args, 0, &m_face))
        Log.report(LogVisor::FatalError, "unable to open FreeType gzip face");

    FT_Done_Face(m_face);
    if (m_decomp.close)
        m_decomp.close(&m_decomp);
}

FreeTypeGZipMemFace::~FreeTypeGZipMemFace()
{
    FT_Done_Face(m_face);
    if (m_decomp.close)
        m_decomp.close(&m_decomp);
}

static FT_Library InitLib()
{
    FT_Library ret;
    FT_Error err = FT_Init_FreeType(&ret);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_Init_FreeType");
    return ret;
}

FontCache::FontCache(const HECL::Runtime::FileStoreManager& fileMgr)
: m_fileMgr(fileMgr),
  m_fontLib(InitLib()),
  m_regFace(m_fontLib, DROIDSANS_PERMISSIVE, DROIDSANS_PERMISSIVE_SZ),
  m_monoFace(m_fontLib, BMONOFONT, BMONOFONT_SZ) {}

FontCache::~FontCache()
{
    FT_Done_FreeType(m_fontLib);
}

}
