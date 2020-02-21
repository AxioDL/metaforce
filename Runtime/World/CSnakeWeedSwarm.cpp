#include "Runtime/World/CSnakeWeedSwarm.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CAnimationParameters.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                 const zeus::CVector3f& pos, const zeus::CVector3f& scale, const CAnimRes& animRes,
                                 const CActorParameters& actParms, float f1, float f2, float f3, float f4, float f5,
                                 float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13,
                                 float f14, const CDamageInfo& dInfo, float /*f15*/, u32 sfxId1, u32 sfxId2, u32 sfxId3,
                                 CAssetId w4, u32 w5, CAssetId w6, float f16)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable), actParms, kInvalidUniqueId)
, xe8_scale(scale)
, xf4_(f1)
, xf8_(f2)
, xfc_(f3)
, x100_(f4)
, x104_(f5)
, x108_(f6)
, x10c_(f7)
, x110_(f8)
, x114_(f9)
, x118_(f10)
, x11c_(std::cos(zeus::degToRad(f11)))
, x120_(f12)
, x124_(f13)
, x128_(f14)
, x15c_dInfo(dInfo)
, x1d0_sfx1(CSfxManager::TranslateSFXID(sfxId1))
, x1d2_sfx2(CSfxManager::TranslateSFXID(sfxId2))
, x1d4_sfx3(CSfxManager::TranslateSFXID(sfxId3))
, x1fc_(w5)
, x200_(f16) {
  SetActorLights(actParms.GetLightParameters().MakeActorLights());
  x1b4_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b4_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b4_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  x1b4_modelData.emplace_back(std::make_unique<CModelData>(animRes));
  if (actParms.GetXRayAssets().first.IsValid()) {
    for (int i = 0; i < 4; ++i)
      x1b4_modelData[i]->SetXRayModel(actParms.GetXRayAssets());
    x13c_xbf_modelAssetDirty = true;
  }
  if (actParms.GetThermalAssets().first.IsValid()) {
    for (int i = 0; i < 4; ++i)
      x1b4_modelData[i]->SetInfraModel(actParms.GetThermalAssets());
    x13c_xbf_modelAssetDirty = true;
  }
  if (w4.IsValid()) {
    x1dc_ = g_SimplePool->GetObj({FOURCC('PART'), w4});
    x1ec_ = std::make_unique<CElementGen>(x1dc_);
  }
  if (w6.IsValid()) {
    x1dc_ = g_SimplePool->GetObj({FOURCC('PART'), w6});
    x1f4_ = std::make_unique<CElementGen>(x1dc_);
  }
}

void CSnakeWeedSwarm::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
