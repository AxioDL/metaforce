#include "MetroidPrime/ScriptObjects/CScriptSound.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CEntityInfo.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"

#include "MetroidPrime/TCastTo.hpp"

#include <Collision/CRayCastResult.hpp>
#include <Kyoto/Audio/CSfxManager.hpp>
#include <Kyoto/Math/CFrustumPlanes.hpp>
#include <rstl/math.hpp>

bool CScriptSound::sFirstInFrame = false;

CScriptSound::CScriptSound(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                           const CTransform4f& xf, const ushort soundId, const bool active,
                           const float maxDist, const float distComp, const float startDelay,
                           const uint minVol, const uint vol, const uint w3, const uint prio,
                           const uint pan, const uint w6, const bool looped, const bool nonEmitter,
                           const bool autoStart, const bool occlusionTest, const bool acoustics,
                           const bool worldSfx, const bool allowDuplicates, const int pitch)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_occUpdateTimer(0.f)
, xf0_maxVol(0)
, xf4_maxVolUpdDelta(0)
, xf8_updateTimer(0.f)
, xfc_startDelay(startDelay)
, x100_soundId(CSfxManager::TranslateSFXID(soundId))
, x104_maxDist(maxDist)
, x108_distComp(distComp)
, x10c_minVol(minVol)
, x10e_vol(vol)
, x110_(w3)
, x112_prio(prio)
, x114_pan(pan)
, x116_(w6)
, x118_pitch(pitch + 8192)
, x11c_24_playRequested(false)
, x11c_25_looped(looped)
, x11c_26_nonEmitter(nonEmitter)
, x11c_27_autoStart(autoStart)
, x11c_28_occlusionTest(occlusionTest)
, x11c_29_acoustics(acoustics)
, x11c_30_worldSfx(worldSfx)
, x11c_31_selfFree(false)
, x11d_24_allowDuplicates(allowDuplicates)
, x11d_25_processedThisFrame(false) {
  if (x11c_30_worldSfx && !x11c_26_nonEmitter) {
    x11c_30_worldSfx = false;
  }

  if (x11c_30_worldSfx && !x11c_25_looped) {
    x11c_30_worldSfx = false;
  }
}

void CScriptSound::PreThink(float dt, CStateManager& mgr) {
  CEntity::PreThink(dt, mgr);
  sFirstInFrame = true;
  x11d_25_processedThisFrame = false;
}

CScriptSound::~CScriptSound() {}

void CScriptSound::Think(float dt, CStateManager& mgr) {
  if (x11c_31_selfFree && (!GetActive() || x11c_25_looped || !x11c_27_autoStart)) {
    mgr.DeleteObjectRequest(GetUniqueId());
    return;
  }

  if (!GetActive()) {
    return;
  }

  if (!x11c_25_looped && x11c_27_autoStart && !x11c_24_playRequested && xec_sfxHandle &&
      !CSfxManager::IsPlaying(xec_sfxHandle)) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  if (!x11c_26_nonEmitter && xec_sfxHandle) {
    if (xf8_updateTimer <= 0.f) {
      xf8_updateTimer = 0.25f;
      char updateDelta = xf2_maxVolUpd;
      CSfxManager::UpdateEmitter(xec_sfxHandle, GetTranslation(), CVector3f::Zero(), updateDelta);
    } else {
      xf8_updateTimer -= dt;
    }
  }

  if (xec_sfxHandle && !x11c_26_nonEmitter && x11c_28_occlusionTest) {
    if (xe8_occUpdateTimer <= 0.f && sFirstInFrame) {
      sFirstInFrame = false;
      const float occVol = GetOccludedVolumeAmount(GetTranslation(), mgr);
      short newMaxVol = CCast::FtoUS(x10e_vol * occVol);
      if (newMaxVol < x10c_minVol) {
        newMaxVol = x10c_minVol;
      }
      if (xf0_maxVol != newMaxVol) {
        xf0_maxVol = newMaxVol;
        const int delta = xf0_maxVol - xf2_maxVolUpd;
        xf4_maxVolUpdDelta = delta / 30;
        if (xf4_maxVolUpdDelta == 0) {
          if (xf2_maxVolUpd < xf0_maxVol) {
            xf4_maxVolUpdDelta = 1;
          } else {
            xf4_maxVolUpdDelta = -1;
          }
        }
      }
      xe8_occUpdateTimer = 0.5f;
    } else {
      xe8_occUpdateTimer -= dt;
    }

    if (xf2_maxVolUpd != xf0_maxVol) {
      xf2_maxVolUpd += xf4_maxVolUpdDelta;
      if (xf4_maxVolUpdDelta > 0 && xf2_maxVolUpd > xf0_maxVol) {
        xf2_maxVolUpd = xf0_maxVol;
      }
      if (xf4_maxVolUpdDelta < 0 && xf2_maxVolUpd < xf0_maxVol) {
        xf2_maxVolUpd = xf0_maxVol;
      }
      const uchar volume = xf2_maxVolUpd;
      CSfxManager::UpdateEmitter(xec_sfxHandle, GetTranslation(), CVector3f::Zero(), volume);
    }
  }

  if (x11c_24_playRequested) {
    xfc_startDelay -= dt;
    if (xfc_startDelay <= 0.f) {
      x11c_24_playRequested = false;
      PlaySound(mgr);
    }
  }
  if (x118_pitch != 8192 && xec_sfxHandle) {
    CSfxManager::PitchBend(xec_sfxHandle, x118_pitch);
  }
}

void CScriptSound::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    if (GetActive() && x11c_27_autoStart) {
      x11c_24_playRequested = true;
    }
    x11c_31_selfFree = mgr.IsGeneratingObject();
  } break;
  case kSM_Play: {
    if (GetActive()) {
      PlaySound(mgr);
    }
  } break;
  case kSM_Stop: {
    if (GetActive()) {
      StopSound(mgr);
    }
  } break;
  case kSM_Deactivate: {
    StopSound(mgr);
  } break;
  case kSM_Activate: {
    if (x11c_27_autoStart) {
      x11c_24_playRequested = true;
    }
  } break;
  case kSM_Deleted: {
    if (!x11c_30_worldSfx) {
      StopSound(mgr);
    }
  }
  }
}

void CScriptSound::PlaySound(CStateManager& mgr) {
  const int areaId = GetCurrentAreaId().Value();
  if ((!x11d_24_allowDuplicates && xec_sfxHandle && CSfxManager::IsHandleValid(xec_sfxHandle)) ||
      x11d_25_processedThisFrame) {
    return;
  }

  x11d_25_processedThisFrame = true;
  if (x11c_26_nonEmitter) {
    CWorld* world = mgr.World();
    if (!x11c_30_worldSfx || !world->HasGlobalSound(x100_soundId)) {
      const bool looped = x11c_25_looped;
      const bool acoustics = x11c_29_acoustics;
      xec_sfxHandle =
          CSfxManager::SfxStart(x100_soundId, x10e_vol, x114_pan, acoustics, x112_prio, looped,
                                x11c_30_worldSfx ? CSfxManager::kAllAreas : areaId);
      if (x11c_30_worldSfx) {
        world->AddGlobalSound(x100_soundId, xec_sfxHandle);
      }
    }
  } else {
    const float volume =
        x11c_28_occlusionTest ? GetOccludedVolumeAmount(GetTranslation(), mgr) : 1.f;
    xf0_maxVol = CCast::FtoUS(x10e_vol * volume);
    xf2_maxVolUpd = xf0_maxVol;
    CAudioSys::C3DEmitterParmData data(x104_maxDist, x108_distComp, 1, xf0_maxVol, x10c_minVol);
    data.x0_pos = GetTranslation();
    data.x24_sfxId = x100_soundId;
    if (x11c_25_looped) {
      xec_sfxHandle = CSfxManager::AddEmitter(data, x11c_29_acoustics, x112_prio, true, areaId);
    } else {
      xec_sfxHandle = CSfxManager::AddEmitter(data, x11c_29_acoustics, x112_prio, false, areaId);
    }
  }
}

void CScriptSound::StopSound(CStateManager& mgr) {
  x11c_24_playRequested = false;
  if (x11c_30_worldSfx && x11c_26_nonEmitter) {
    mgr.World()->StopGlobalSound(GetSoundId());
    xec_sfxHandle.Clear();
  } else if (xec_sfxHandle) {
    CSfxManager::RemoveEmitter(xec_sfxHandle);
    xec_sfxHandle.Clear();
  }
}

void CScriptSound::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {}

ENTITY_ACCEPT_IMPL(CScriptSound)

float CScriptSound::GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr) {
  const CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  const CVector3f soundToCam = camXf.GetTranslation() - pos;
  const float soundToCamMag = soundToCam.Magnitude();
  const CVector3f soundToCamNorm = soundToCam * (1.f / soundToCamMag);
  const CVector3f up(0.f, 0.f, 1.f);
  const CVector3f thirdEdge = up - soundToCamNorm * CVector3f::Dot(up, soundToCamNorm);
  const CVector3f cross = CVector3f::Cross(soundToCamNorm, thirdEdge);
  static const float kInfluenceAmount = 3.f / soundToCamMag;
  static const float kInfluenceIncrement = kInfluenceAmount;
  static CMaterialFilter kSolidFilter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
  int totalCount = 0;
  int invalCount = 0;
  for (float i = -kInfluenceAmount; i <= kInfluenceAmount; i += kInfluenceIncrement) {
    for (float j = -kInfluenceAmount; j <= kInfluenceAmount; j += kInfluenceIncrement) {
      ++totalCount;

      const CVector3f rayDir = (soundToCamNorm + i * thirdEdge) + j * cross;
      if (mgr.RayStaticIntersection(pos, rayDir.AsNormalized(), soundToCamMag, kSolidFilter)
              .IsInvalid()) {
        ++invalCount;
      }
    }
  }
  return invalCount / static_cast< float >(totalCount) * (1.f - 0.58f) + 0.58f;
}
