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
, mOccUpdateTimer(0.f)
, mMaxVol(0)
, mMaxVolUpdDelta(0)
, mUpdateTimer(0.f)
, mStartDelay(startDelay)
, mSoundId(CSfxManager::TranslateSFXID(soundId))
, mMaxDist(maxDist)
, mDistComp(distComp)
, mMinVol(minVol)
, mVol(vol)
, x110_(w3)
, mPrio(prio)
, mPan(pan)
, x116_(w6)
, mPitch(pitch + 8192)
, mPlayRequested(false)
, mLooped(looped)
, mNonEmitter(nonEmitter)
, mAutoStart(autoStart)
, mOcclusionTest(occlusionTest)
, mAcoustics(acoustics)
, mWorldSfx(worldSfx)
, mSelfFree(false)
, mAllowDuplicates(allowDuplicates)
, mProcessedThisFrame(false) {
  if (mWorldSfx && !mNonEmitter) {
    mWorldSfx = false;
  }

  if (mWorldSfx && !mLooped) {
    mWorldSfx = false;
  }
}

void CScriptSound::PreThink(float dt, CStateManager& mgr) {
  CEntity::PreThink(dt, mgr);
  sFirstInFrame = true;
  mProcessedThisFrame = false;
}

CScriptSound::~CScriptSound() {}

void CScriptSound::Think(float dt, CStateManager& mgr) {
  if (mSelfFree && (!GetActive() || mLooped || !mAutoStart)) {
    mgr.DeleteObjectRequest(GetUniqueId());
    return;
  }

  if (!GetActive()) {
    return;
  }

  if (!mLooped && mAutoStart && !mPlayRequested && mSfxHandle &&
      !CSfxManager::IsPlaying(mSfxHandle)) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }

  if (!mNonEmitter && mSfxHandle) {
    if (mUpdateTimer <= 0.f) {
      mUpdateTimer = 0.25f;
      char updateDelta = mMaxVolUpd;
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), CVector3f::Zero(), updateDelta);
    } else {
      mUpdateTimer -= dt;
    }
  }

  if (mSfxHandle && !mNonEmitter && mOcclusionTest) {
    if (mOccUpdateTimer <= 0.f && sFirstInFrame) {
      sFirstInFrame = false;
      const float occVol = GetOccludedVolumeAmount(GetTranslation(), mgr);
      short newMaxVol = CCast::FtoUS(mVol * occVol);
      if (newMaxVol < mMinVol) {
        newMaxVol = mMinVol;
      }
      if (mMaxVol != newMaxVol) {
        mMaxVol = newMaxVol;
        const int delta = mMaxVol - mMaxVolUpd;
        mMaxVolUpdDelta = delta / 30;
        if (mMaxVolUpdDelta == 0) {
          if (mMaxVolUpd < mMaxVol) {
            mMaxVolUpdDelta = 1;
          } else {
            mMaxVolUpdDelta = -1;
          }
        }
      }
      mOccUpdateTimer = 0.5f;
    } else {
      mOccUpdateTimer -= dt;
    }

    if (mMaxVolUpd != mMaxVol) {
      mMaxVolUpd += mMaxVolUpdDelta;
      if (mMaxVolUpdDelta > 0 && mMaxVolUpd > mMaxVol) {
        mMaxVolUpd = mMaxVol;
      }
      if (mMaxVolUpdDelta < 0 && mMaxVolUpd < mMaxVol) {
        mMaxVolUpd = mMaxVol;
      }
      const uchar volume = mMaxVolUpd;
      CSfxManager::UpdateEmitter(mSfxHandle, GetTranslation(), CVector3f::Zero(), volume);
    }
  }

  if (mPlayRequested) {
    mStartDelay -= dt;
    if (mStartDelay <= 0.f) {
      mPlayRequested = false;
      PlaySound(mgr);
    }
  }
  if (mPitch != 8192 && mSfxHandle) {
    CSfxManager::PitchBend(mSfxHandle, mPitch);
  }
}

void CScriptSound::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Registered: {
    if (GetActive() && mAutoStart) {
      mPlayRequested = true;
    }
    mSelfFree = mgr.IsGeneratingObject();
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
    if (mAutoStart) {
      mPlayRequested = true;
    }
  } break;
  case kSM_Deleted: {
    if (!mWorldSfx) {
      StopSound(mgr);
    }
  }
  }
}

void CScriptSound::PlaySound(CStateManager& mgr) {
  const int areaId = GetCurrentAreaId().Value();
  if ((!mAllowDuplicates && mSfxHandle && CSfxManager::IsHandleValid(mSfxHandle)) ||
      mProcessedThisFrame) {
    return;
  }

  mProcessedThisFrame = true;
  if (mNonEmitter) {
    CWorld* world = mgr.World();
    if (!mWorldSfx || !world->HasGlobalSound(mSoundId)) {
      const bool looped = mLooped;
      const bool acoustics = mAcoustics;
      mSfxHandle =
          CSfxManager::SfxStart(mSoundId, mVol, mPan, acoustics, mPrio, looped,
                                mWorldSfx ? CSfxManager::kAllAreas : areaId);
      if (mWorldSfx) {
        world->AddGlobalSound(mSoundId, mSfxHandle);
      }
    }
  } else {
    const float volume =
        mOcclusionTest ? GetOccludedVolumeAmount(GetTranslation(), mgr) : 1.f;
    mMaxVol = CCast::FtoUS(mVol * volume);
    mMaxVolUpd = mMaxVol;
    CAudioSys::C3DEmitterParmData data(mMaxDist, mDistComp, 1, mMaxVol, mMinVol);
    data.mPos = GetTranslation();
    data.mSfxId = mSoundId;
    if (mLooped) {
      mSfxHandle = CSfxManager::AddEmitter(data, mAcoustics, mPrio, true, areaId);
    } else {
      mSfxHandle = CSfxManager::AddEmitter(data, mAcoustics, mPrio, false, areaId);
    }
  }
}

void CScriptSound::StopSound(CStateManager& mgr) {
  mPlayRequested = false;
  if (mWorldSfx && mNonEmitter) {
    mgr.World()->StopGlobalSound(GetSoundId());
    mSfxHandle.Clear();
  } else if (mSfxHandle) {
    CSfxManager::RemoveEmitter(mSfxHandle);
    mSfxHandle.Clear();
  }
}

void CScriptSound::AddToRenderer(const CFrustumPlanes& planes, const CStateManager& mgr) const {}

ENTITY_ACCEPT_IMPL(CScriptSound)

float CScriptSound::GetOccludedVolumeAmount(const CVector3f& pos, const CStateManager& mgr) {
  const CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  const CVector3f soundToCam = camXf.GetTranslation() - pos;
  const float soundToCamMag = soundToCam.Magnitude();
#if VERSION >= VERSION_GM8E_02
  const CVector3f soundToCamNorm = soundToCam / soundToCamMag;
#else
  const CVector3f soundToCamNorm = soundToCam * (1.f / soundToCamMag);
#endif
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
#if VERSION >= VERSION_GM8E_02
      const CRayCastResult result =
          mgr.RayStaticIntersection(pos, rayDir.AsNormalized(), soundToCamMag, kSolidFilter);
      if (result.IsInvalid()) {
#else
      if (mgr.RayStaticIntersection(pos, rayDir.AsNormalized(), soundToCamMag, kSolidFilter)
              .IsInvalid()) {
#endif
        ++invalCount;
      }
    }
  }
  return invalCount / static_cast< float >(totalCount) * (1.f - 0.58f) + 0.58f;
}
