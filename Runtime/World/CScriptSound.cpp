#include "Runtime/World/CScriptSound.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
bool CScriptSound::sFirstInFrame = false;

CScriptSound::CScriptSound(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           u16 soundId, bool active, float maxDist, float distComp, float startDelay, u32 minVol,
                           u32 vol, u32 w3, u32 prio, u32 pan, u32 w6, bool looped, bool nonEmitter, bool autoStart,
                           bool occlusionTest, bool acoustics, bool worldSfx, bool allowDuplicates, s32 pitch)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xfc_startDelay(startDelay)
, x100_soundId(CSfxManager::TranslateSFXID(soundId))
, x104_maxDist(maxDist)
, x108_distComp(distComp)
, x10c_minVol(minVol / 127.f)
, x10e_vol(vol / 127.f)
, x110_(w3)
, x112_prio(s16(prio))
, x114_pan(pan / 64.f - 1.f)
, x116_(w6)
, x118_pitch(pitch / 8192.f)
, x11c_25_looped(looped)
, x11c_26_nonEmitter(nonEmitter)
, x11c_27_autoStart(autoStart)
, x11c_28_occlusionTest(occlusionTest)
, x11c_29_acoustics(acoustics)
, x11c_30_worldSfx(worldSfx)
, x11d_24_allowDuplicates(allowDuplicates) {
  if (x11c_30_worldSfx && (!x11c_26_nonEmitter || !x11c_25_looped))
    x11c_30_worldSfx = false;
}

void CScriptSound::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptSound::PreThink(float dt, CStateManager& mgr) {
  CEntity::PreThink(dt, mgr);
  sFirstInFrame = true;
  x11d_25_processedThisFrame = false;
}

constexpr CMaterialFilter kSolidFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough});

float CScriptSound::GetOccludedVolumeAmount(const zeus::CVector3f& pos, const CStateManager& mgr) {
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  zeus::CVector3f soundToCam = camXf.origin - pos;
  float soundToCamMag = soundToCam.magnitude();
  zeus::CVector3f soundToCamNorm = soundToCam * (1.f / soundToCamMag);
  zeus::CVector3f thirdEdge = zeus::skUp - soundToCamNorm * soundToCamNorm.dot(zeus::skUp);
  zeus::CVector3f cross = soundToCamNorm.cross(thirdEdge);
  static float kInfluenceAmount = 3.f / soundToCamMag;
  static float kInfluenceIncrement = kInfluenceAmount;

  int totalCount = 0;
  int invalCount = 0;
  float f17 = -kInfluenceAmount;
  while (f17 <= kInfluenceAmount) {
    zeus::CVector3f angledDir = thirdEdge * f17 + soundToCamNorm;
    float f16 = -kInfluenceAmount;
    while (f16 <= kInfluenceAmount) {
      if (mgr.RayStaticIntersection(pos, (cross * f16 + angledDir).normalized(), soundToCamMag, kSolidFilter)
              .IsInvalid())
        ++invalCount;
      ++totalCount;
      f16 += kInfluenceIncrement;
    }
    f17 += kInfluenceIncrement;
  }

  return invalCount / float(totalCount) * 0.42f + 0.58f;
}

void CScriptSound::Think(float dt, CStateManager& mgr) {
  if (x11c_31_selfFree && (!GetActive() || x11c_25_looped || !x11c_27_autoStart)) {
    mgr.FreeScriptObject(GetUniqueId());
  } else if (GetActive()) {
    if (!x11c_25_looped && x11c_27_autoStart && !x11c_24_playRequested && xec_sfxHandle &&
        !CSfxManager::IsPlaying(xec_sfxHandle))
      mgr.FreeScriptObject(GetUniqueId());

    if (!x11c_26_nonEmitter && xec_sfxHandle) {
      if (xf8_updateTimer <= 0.f) {
        xf8_updateTimer = 0.25f;
        CSfxManager::UpdateEmitter(xec_sfxHandle, GetTranslation(), zeus::skZero3f, xf2_maxVolUpd);
      } else {
        xf8_updateTimer -= dt;
      }
    }

    if (xec_sfxHandle && !x11c_26_nonEmitter && x11c_28_occlusionTest) {
      if (xe8_occUpdateTimer <= 0.f && sFirstInFrame) {
        sFirstInFrame = false;
        float occVol = GetOccludedVolumeAmount(GetTranslation(), mgr);
        float newMaxVol = std::max(occVol * x10e_vol, x10c_minVol);
        if (newMaxVol != xf0_maxVol) {
          xf0_maxVol = newMaxVol;
          float delta = xf0_maxVol - xf2_maxVolUpd;
          xf4_maxVolUpdDelta = delta / 10.5f;
          if (xf4_maxVolUpdDelta == 0.f) {
            if (xf2_maxVolUpd < xf0_maxVol)
              xf4_maxVolUpdDelta = 1.f / 127.f;
            else
              xf4_maxVolUpdDelta = -1.f / 127.f;
          }
        }
        xe8_occUpdateTimer = 0.5f;
      } else {
        xe8_occUpdateTimer -= dt;
      }

      if (xf2_maxVolUpd != xf0_maxVol) {
        xf2_maxVolUpd += xf4_maxVolUpdDelta;
        if (xf4_maxVolUpdDelta > 0.f && xf2_maxVolUpd > xf0_maxVol)
          xf2_maxVolUpd = xf0_maxVol;
        if (xf4_maxVolUpdDelta < 0.f && xf2_maxVolUpd < xf0_maxVol)
          xf2_maxVolUpd = xf0_maxVol;
        CSfxManager::UpdateEmitter(xec_sfxHandle, GetTranslation(), zeus::skZero3f, xf2_maxVolUpd);
      }
    }

    if (x11c_24_playRequested) {
      xfc_startDelay -= dt;
      if (xfc_startDelay <= 0.f) {
        x11c_24_playRequested = false;
        PlaySound(mgr);
      }
    }

    if (x118_pitch != 0.f && xec_sfxHandle)
      CSfxManager::PitchBend(xec_sfxHandle, x118_pitch);
  }
}

void CScriptSound::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Registered: {
    if (GetActive() && x11c_27_autoStart)
      x11c_24_playRequested = true;
    x11c_31_selfFree = mgr.GetIsGeneratingObject();
  } break;
  case EScriptObjectMessage::Play: {
    if (GetActive())
      PlaySound(mgr);
  } break;
  case EScriptObjectMessage::Stop: {
    if (GetActive())
      StopSound(mgr);
  } break;
  case EScriptObjectMessage::Deactivate: {
    StopSound(mgr);
  } break;
  case EScriptObjectMessage::Activate: {
    if (GetActive())
      x11c_24_playRequested = true;
  } break;
  case EScriptObjectMessage::Deleted: {
    if (!x11c_30_worldSfx)
      StopSound(mgr);
  } break;
  default:
    break;
  }
}

void CScriptSound::PlaySound(CStateManager& mgr) {
  if ((x11d_24_allowDuplicates || !xec_sfxHandle || xec_sfxHandle->IsClosed()) && !x11d_25_processedThisFrame) {
    x11d_25_processedThisFrame = true;
    if (x11c_26_nonEmitter) {
      if (!x11c_30_worldSfx || !mgr.GetWorld()->HasGlobalSound(x100_soundId)) {
        xec_sfxHandle = CSfxManager::SfxStart(x100_soundId, x10e_vol, x114_pan, x11c_29_acoustics, x112_prio,
                                              x11c_25_looped, x11c_30_worldSfx ? kInvalidAreaId : GetAreaIdAlways());
        if (x11c_30_worldSfx)
          mgr.GetWorld()->AddGlobalSound(xec_sfxHandle);
      }
    } else {
      float occVol = x11c_28_occlusionTest ? GetOccludedVolumeAmount(GetTranslation(), mgr) : 1.f;
      xf0_maxVol = xf2_maxVolUpd = x10e_vol * occVol;
      CAudioSys::C3DEmitterParmData data = {};
      data.x0_pos = GetTranslation();
      data.x18_maxDist = x104_maxDist;
      data.x1c_distComp = x108_distComp;
      data.x20_flags = 1; // Continuous parameter update
      data.x24_sfxId = x100_soundId;
      data.x26_maxVol = xf0_maxVol;
      data.x27_minVol = x10c_minVol;
      data.x29_prio = 0x7f;
      xec_sfxHandle = CSfxManager::AddEmitter(data, x11c_29_acoustics, x112_prio, x11c_25_looped, GetAreaIdAlways());
    }
  }
}

void CScriptSound::StopSound(CStateManager& mgr) {
  x11c_24_playRequested = false;
  if (x11c_30_worldSfx && x11c_26_nonEmitter) {
    mgr.GetWorld()->StopGlobalSound(x100_soundId);
    xec_sfxHandle.reset();
  } else if (xec_sfxHandle) {
    CSfxManager::RemoveEmitter(xec_sfxHandle);
    xec_sfxHandle.reset();
  }
}
} // namespace urde
