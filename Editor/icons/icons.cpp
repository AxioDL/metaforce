#include "icons.hpp"
#include "Athena/MemoryReader.hpp"
#include <zlib.h>

extern "C" uint8_t URDE_ICONS[];
extern "C" size_t URDE_ICONS_SZ;

namespace URDE
{
static LogVisor::LogModule Log("URDE::icons");

Specter::IconAtlas<8,8> g_IconAtlas;

boo::GraphicsDataToken InitializeIcons(Specter::ViewResources& viewRes)
{
    Athena::io::MemoryReader r(URDE_ICONS, URDE_ICONS_SZ);
    size_t fmt = r.readUint32Big();
    if (fmt != 16)
        Log.report(LogVisor::FatalError, "incorrect icon texture format");
    size_t width = r.readUint16Big();
    size_t height = r.readUint16Big();
    size_t mips = r.readUint32Big();
    size_t decompSz = r.readUint32Big();

    std::unique_ptr<uint8_t[]> texels(new uint8_t[decompSz]);
    uLongf destSz = decompSz;
    size_t pos = r.position();
    if (uncompress(texels.get(), &destSz, URDE_ICONS + pos, URDE_ICONS_SZ - pos) != Z_OK)
        Log.report(LogVisor::FatalError, "unable to decompress icons");

    g_IconAtlas.initializeAtlas(viewRes.m_factory->newStaticTexture(width, height, mips, boo::TextureFormat::RGBA8,
                                                                    texels.get(), destSz));
    return viewRes.m_factory->commit();
}

Specter::Icon& GetIcon(SpaceIcon icon)
{
    switch (icon)
    {
    case SpaceIcon::ResourceBrowser:
        return g_IconAtlas.getIcon(0, 0);
    case SpaceIcon::ParticleEditor:
        return g_IconAtlas.getIcon(0, 1);
    case SpaceIcon::WorldEditor:
        return g_IconAtlas.getIcon(0, 2);
    case SpaceIcon::InformationCenter:
        return g_IconAtlas.getIcon(0, 3);
    case SpaceIcon::ModelViewer:
        return g_IconAtlas.getIcon(0, 4);
    }
}

Specter::Icon& GetIcon(MonoIcon icon)
{
    switch (icon)
    {
    case MonoIcon::Sync:
        return g_IconAtlas.getIcon(7, 0);
    case MonoIcon::Edit:
        return g_IconAtlas.getIcon(7, 1);
    case MonoIcon::Caution:
        return g_IconAtlas.getIcon(7, 2);
    case MonoIcon::Save:
        return g_IconAtlas.getIcon(7, 3);
    case MonoIcon::Filter:
        return g_IconAtlas.getIcon(7, 4);
    case MonoIcon::Document:
        return g_IconAtlas.getIcon(7, 5);
    case MonoIcon::ZoomOut:
        return g_IconAtlas.getIcon(7, 6);
    case MonoIcon::ZoomIn:
        return g_IconAtlas.getIcon(7, 7);
    case MonoIcon::Exclaim:
        return g_IconAtlas.getIcon(6, 0);
    case MonoIcon::Clock:
        return g_IconAtlas.getIcon(6, 1);
    case MonoIcon::Gamepad:
        return g_IconAtlas.getIcon(6, 2);
    case MonoIcon::Unlink:
        return g_IconAtlas.getIcon(6, 3);
    case MonoIcon::Link:
        return g_IconAtlas.getIcon(6, 4);
    case MonoIcon::Folder:
        return g_IconAtlas.getIcon(6, 5);
    case MonoIcon::Info:
        return g_IconAtlas.getIcon(6, 6);
    }
}

}
