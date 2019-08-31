#include "Badging.hpp"
#include "athena/MemoryReader.hpp"

#include <specter/Icon.hpp>
#include <zlib.h>

extern "C" uint8_t URDE_BADGE[];
extern "C" size_t URDE_BADGE_SZ;

namespace urde {
static logvisor::Module Log("URDE::badging");
static specter::Icon g_BadgeIcon;

void InitializeBadging(specter::ViewResources& viewRes) {
  athena::io::MemoryReader r(URDE_BADGE, URDE_BADGE_SZ);

  size_t fmt = r.readUint32Big();
  if (fmt != 16)
    Log.report(logvisor::Fatal, fmt("incorrect icon texture format"));
  size_t width = r.readUint16Big();
  size_t height = r.readUint16Big();
  size_t mips = r.readUint32Big();
  size_t decompSz = r.readUint32Big();

  std::unique_ptr<uint8_t[]> texels(new uint8_t[decompSz]);
  uLongf destSz = decompSz;
  size_t pos = r.position();
  if (uncompress(texels.get(), &destSz, URDE_BADGE + pos, URDE_BADGE_SZ - pos) != Z_OK)
    Log.report(logvisor::Fatal, fmt("unable to decompress badge"));

  viewRes.m_factory->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
    specter::IconAtlas<1, 1> atlas;

    atlas.initializeAtlas(ctx.newStaticTexture(width, height, mips, boo::TextureFormat::RGBA8,
                                               boo::TextureClampMode::Repeat, texels.get(), destSz));
    g_BadgeIcon = atlas.getIcon(0, 0);
    return true;
  } BooTrace);
}

void DestroyBadging() { g_BadgeIcon.m_tex.reset(); }

specter::Icon& GetBadge() { return g_BadgeIcon; }

} // namespace urde
