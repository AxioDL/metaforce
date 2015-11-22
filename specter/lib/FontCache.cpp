#include "Specter/FontCache.hpp"
#include <LogVisor/LogVisor.hpp>
#include <stdint.h>

extern "C" uint8_t* DROIDSANS_PERMISSIVE;
extern "C" size_t DROIDSANS_PERMISSIVE_SZ;

extern "C" uint8_t* BMONOFONT;
extern "C" size_t BMONOFONT_SZ;

namespace Specter
{
static LogVisor::LogModule Log("Specter::FontCache");

FontCache::FontCache(const HECL::Runtime::FileStoreManager& fileMgr)
: m_fileMgr(fileMgr)
{
    FT_Error err = FT_Init_FreeType(&m_fontLib);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_Init_FreeType");
    err = FT_New_Memory_Face(m_fontLib, DROIDSANS_PERMISSIVE, DROIDSANS_PERMISSIVE_SZ, 0, &m_regFace);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_New_Memory_Face for main UI font");
    err = FT_New_Memory_Face(m_fontLib, BMONOFONT, BMONOFONT_SZ, 0, &m_monoFace);
    if (err)
        Log.report(LogVisor::FatalError, "unable to FT_New_Memory_Face for mono UI font");
}

FontCache::~FontCache()
{
    FT_Done_FreeType(m_fontLib);
}

}
