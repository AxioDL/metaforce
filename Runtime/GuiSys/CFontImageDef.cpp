#include "CFontImageDef.hpp"
#include "Graphics/CTexture.hpp"

namespace urde {

CFontImageDef::CFontImageDef(const std::vector<TToken<CTexture>>& texs, float interval,
                             const zeus::CVector2f& cropFactor)
: x0_fps(interval), x14_cropFactor(cropFactor) {
  x4_texs.reserve(texs.size());
  for (const TToken<CTexture>& tok : texs)
    x4_texs.push_back(tok);
}

CFontImageDef::CFontImageDef(const TToken<CTexture>& tex, const zeus::CVector2f& cropFactor)
: x0_fps(0.f), x14_cropFactor(cropFactor) {
  x4_texs.push_back(tex);
}

bool CFontImageDef::IsLoaded() const {
  for (const TToken<CTexture>& tok : x4_texs)
    if (!tok.IsLoaded())
      return false;
  return true;
}

s32 CFontImageDef::CalculateBaseline() const {
  const CTexture* tex = x4_texs.front().GetObj();
  return s32(tex->GetHeight() * x14_cropFactor.y()) * 2.5f / 3.f;
}

s32 CFontImageDef::CalculateHeight() const {
  const CTexture* tex = x4_texs.front().GetObj();
  s32 scaledH = tex->GetHeight() * x14_cropFactor.y();
  return scaledH - (scaledH - CalculateBaseline());
}

} // namespace urde
