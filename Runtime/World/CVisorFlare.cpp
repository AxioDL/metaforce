#include "CVisorFlare.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"

namespace urde {

std::experimental::optional<CVisorFlare::CFlareDef> CVisorFlare::LoadFlareDef(CInputStream& in) {
  u32 propCount = in.readUint32Big();
  if (propCount != 4)
    return {};

  CAssetId txtrId = in.readUint32Big();
  float f1 = in.readFloatBig();
  float f2 = in.readFloatBig();
  zeus::CColor color = zeus::CColor::ReadRGBABig(in);
  if (!txtrId.IsValid())
    return {};

  TToken<CTexture> tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), txtrId});

  return CFlareDef(tex, f1, f2, color);
}

CVisorFlare::CVisorFlare(EBlendMode blendMode, bool b1, float f1, float f2, float f3, u32 w1, u32 w2,
                         const std::vector<CFlareDef>& flares)
: x0_blendMode(blendMode)
, x4_flareDefs(flares)
, x14_b1(b1)
, x18_f1(std::max(f1, 0.0001f))
, x1c_f2(f2)
, x20_f3(f3)
, x2c_w1(w1)
, x30_w2(w2) {}

void CVisorFlare::Update(float dt, const zeus::CVector3f& pos, const CActor* act, CStateManager& mgr) {}

void CVisorFlare::Render(const zeus::CVector3f& pos, const CStateManager& mgr) const {}

} // namespace urde
