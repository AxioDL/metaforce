#include "MetroidPrime/ScriptObjects/CScriptVisorGoo.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"

#include "rstl/math.hpp"

CScriptVisorGoo::CScriptVisorGoo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf, CAssetId particle, CAssetId electric,
                                 float minRange, float maxRange, float chanceMinRange,
                                 float chanceMaxRange, const CColor& color, int sfx,
                                 bool noViewCheck, const bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mParticleDesc(nullptr)
, mElectricDesc(nullptr)
, mSfx(CSfxManager::TranslateSFXID(sfx))
, mParticleId(particle)
, mElectricId(electric)
, mMinRange(minRange)
, mMaxRange(rstl::max_val(maxRange, minRange + 0.01f))
, mChanceMinRange(chanceMinRange)
, mChanceMaxRange(chanceMaxRange)
, mColor(color) {
  mViewCheck = !noViewCheck;
  if (particle != kInvalidAssetId) {
    mParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', particle));
  }
  if (electric != kInvalidAssetId) {
    mElectricDesc = gpSimplePool->GetObj(SObjectTag('ELSC', electric));
  }
}

CScriptVisorGoo::~CScriptVisorGoo() {}

void CScriptVisorGoo::Think(float, CStateManager& mgr) {
  if (GetActive()) {
    bool loaded = false;
    // MPR has IsElectricLoaded()/IsElectricValid(),
    // but no IsParticleLoaded()/IsParticleValid()?
    if (mParticleId != kInvalidAssetId) {
      if (GetParticleDesc().IsLoaded()) {
        if (mElectricId != kInvalidAssetId) {
          if (GetElectricDesc().IsLoaded()) {
            loaded = true;
          }
        } else {
          loaded = true;
        }
      }
    } else {
      if (GetElectricDesc().IsLoaded()) {
        loaded = true;
      }
    }

    if (loaded) {
      bool showGoo = false;
      if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
        const CVector3f eyeToGoo = GetTranslation() - mgr.GetPlayer()->GetEyePosition();
        const float eyeToGooDist = eyeToGoo.Magnitude();
        if (eyeToGooDist >= GetMinRange() && eyeToGooDist <= GetMaxRange()) {
          if (GetViewCheck()) {
            CVector3f colNorm = mgr.GetCameraManager()
                                    ->GetCurrentCameraTransform(mgr)
                                    .GetColumn(kDY)
                                    .AsNormalized();
            float angleThresh = 45.f;
            float angle = CMath::Rad2Rev(CMath::FastArcCosR(
                              CVector3f::Dot(eyeToGoo.AsNormalized(), colNorm))) *
                          360.f;
            if (eyeToGooDist < 4.f) {
              angleThresh *= 4.f / eyeToGooDist;
              angleThresh = rstl::min_val(angleThresh, 90.f);
            }
            if (angle <= angleThresh) {
              showGoo = true;
            }
          } else {
            showGoo = true;
          }
          if (showGoo) {
            const float t = (GetMaxRange() - eyeToGooDist) / (GetMaxRange() - GetMinRange());
            float prob = t * GetChanceMinRange() + (1.0f - t) * GetChanceMaxRange();
            if (mgr.Random()->Float() * 100.f <= prob) {
              mgr.AddObject(rs_new CHUDBillboardEffect(
                  mParticleId != kInvalidAssetId
                      ? rstl::optional_object< TToken< CGenDescription > >(GetParticleDesc())
                      : rstl::optional_object_null(),
                  mElectricId != kInvalidAssetId
                      ? rstl::optional_object< TToken< CElectricDescription > >(GetElectricDesc())
                      : rstl::optional_object_null(),
                  mgr.AllocateUniqueId(), true, rstl::string_l("VisorGoo"),
                  CHUDBillboardEffect::GetNearClipDistance(mgr),
                  CHUDBillboardEffect::GetScaleForPOV(mgr), GetColor(), CVector3f(1.f, 1.f, 1.f),
                  CVector3f(0.f, 0.f, 0.f)));
              CSfxManager::SfxStart(CCast::ToUint16(mSfx), 0x7f, 0x40, false,
                                    CSfxManager::kMedPriority);
            }
          }
        }
      }
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }
}

void CScriptVisorGoo::Touch(CActor& other, CStateManager& mgr) {
  // Empty
}

rstl::optional_object< CAABox > CScriptVisorGoo::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CScriptVisorGoo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                      CStateManager& mgr) {
  switch (msg) {
  case kSM_Activate:
    if (mParticleId != kInvalidAssetId) {
      mParticleDesc.Lock();
    }
    if (mElectricId != kInvalidAssetId) {
      mElectricDesc.Lock();
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(msg, objId, mgr);
}

void CScriptVisorGoo::Render(const CStateManager& mgr) const {
  // Empty
}

void CScriptVisorGoo::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  // Empty
}

ENTITY_ACCEPT_IMPL(CScriptVisorGoo)
