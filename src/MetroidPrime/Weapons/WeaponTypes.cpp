#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/Weapons/WeaponCommon.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CGunWeapon.hpp"

#include "Kyoto/Animation/CPrimitive.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CToken.hpp"

#include "rstl/set.hpp"

namespace NWeaponTypes {

CAssetId get_asset_id_from_name(const char* name) {
  const SObjectTag* tag = gpResourceFactory->GetResourceIdByName(name);
  if (!tag) {
    return kInvalidAssetId;
  }
  return tag->GetId();
}

void get_token_vector(CAnimData& animData, int animIdx, rstl::vector< CToken >& tokensOut,
                      const bool preLock) {
  rstl::set< CPrimitive > prims;
  CAnimPlaybackParms parms(animIdx, -1, 1.f, true);
  animData.GetAnimationPrimitives(parms, prims);
  primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

void get_token_vector(const CAnimData& animData, int begin, int end,
                      rstl::vector< CToken >& tokensOut, const bool preLock) {
  rstl::set< CPrimitive > prims;
  for (int i = begin; i < end; ++i) {
    CAnimPlaybackParms parms(i, -1, 1.f, true);
    animData.GetAnimationPrimitives(parms, prims);
  }
  primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

bool are_tokens_ready(const rstl::vector< CToken >& anims) {
  for (rstl::vector< CToken >::const_iterator it = anims.begin(); it != anims.end(); ++it)
    if (!it->IsLoaded())
      return false;
  return true;
}

void lock_tokens(rstl::vector< CToken >& anims) {
  for (rstl::vector< CToken >::iterator it = anims.begin(); it != anims.end(); ++it)
    it->Lock();
}

void unlock_tokens(rstl::vector< CToken >& anims) {
  for (rstl::vector< CToken >::iterator it = anims.begin(); it != anims.end(); ++it)
    it->Unlock();
}

void primitive_set_to_token_vector(const CAnimData& animData,
                                   const rstl::set< CPrimitive >& primSet,
                                   rstl::vector< CToken >& tokensOut, const bool preLock) {

  int eventCount = 0;

  rstl::set< CPrimitive >::const_iterator it = primSet.begin();
  for (; it != primSet.end(); ++it) {
    if (animData.GetEventResourceIdForAnimResourceId(it->GetAnimResId()) != -1)
      ++eventCount;
  }

  tokensOut = rstl::vector< CToken >();
  tokensOut.reserve(primSet.size() + eventCount);

  for (it = primSet.begin(); it != primSet.end(); ++it) {
    CAssetId aId = it->GetAnimResId();
    int eId = animData.GetEventResourceIdForAnimResourceId(aId);

    if (eId != -1) {
      CToken token = gpSimplePool->GetObj(SObjectTag('EVNT', eId));
      if (preLock)
        token.Lock();
      tokensOut.push_back(token);
    }

    CToken token = gpSimplePool->GetObj(SObjectTag('ANIM', aId));
    if (preLock)
      token.Lock();
    tokensOut.push_back(token);
  }
}

void do_sound_event(rstl::pair< u16, CSfxHandle >& sfxHandle, int& pitch, bool doPitchBend,
                    uint soundId, float weight, uint flags, float falloff, float maxDist,
                    uchar minVol, const uchar maxVol, const CVector3f& posToCam,
                    const CVector3f& pos, int aid, CStateManager& mgr) {
  if (!(posToCam.MagSquared() < maxDist * maxDist))
    return;

  const u16 useSfxId = CSfxManager::TranslateSFXID(u16(soundId));
  const bool looping = (soundId & 0x80000000) != 0;
  const bool nonPositional = (soundId & 0x40000000) != 0;
  const bool useAcoustics = (flags & 0x80) == 0;
  uint useFlags = 0x1; // Continuous parameter update
  if ((flags & 0x8) != 0)
    useFlags |= 0x8; // Doppler effect

  CAudioSys::C3DEmitterParmData parms(maxDist, falloff, useFlags, maxVol, minVol);
  parms.mPos = pos;
  parms.mDir = CVector3f::Up();
  parms.mSfxId = useSfxId;

  if (mgr.Random()->Float() <= weight) {
    if (looping) {
      const CSfxHandle currentHandle = sfxHandle.second;
      const u16 currentId = sfxHandle.first;
      if (!currentHandle) {
        CSfxHandle hnd;
        if (nonPositional)
          hnd = CSfxManager::SfxStart(useSfxId, 0x7f, 0x40, true, CSfxManager::kMedPriority, true,
                                      aid);
        else
          hnd = CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
        if (hnd) {
          sfxHandle.first = useSfxId;
          sfxHandle.second = hnd;
          if (doPitchBend)
            CSfxManager::PitchBend(hnd, pitch);
        }
      } else {
        if (currentId == useSfxId) {
          CSfxManager::UpdateEmitter(currentHandle, parms.mPos, parms.mDir, maxVol);
        } else if ((flags & 0x4) != 0) // Pausable
        {
          CSfxManager::RemoveEmitter(currentHandle);
          CSfxHandle hnd =
              CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, true, aid);
          if (hnd) {
            sfxHandle.first = useSfxId;
            sfxHandle.second = hnd;
            if (doPitchBend && hnd)
              CSfxManager::PitchBend(hnd, pitch);
          }
        }
      }
    } else {
      CSfxHandle hnd;
      if (nonPositional)
        hnd = CSfxManager::SfxStart(useSfxId, 0x7f, 0x40, useAcoustics, CSfxManager::kMedPriority,
                                    false, aid);
      else
        hnd = CSfxManager::AddEmitter(parms, useAcoustics, CSfxManager::kMedPriority, false, aid);
      if (doPitchBend)
        CSfxManager::PitchBend(hnd, pitch);
    }
  }
}

CSfxHandle play_sfx(const ushort sfx, const bool underwater, const bool looped, const short pan) {
  CSfxHandle hnd = CSfxManager::SfxStart(sfx, 0x7f, pan, true, CSfxManager::kMaxPriority, looped);
  CSfxManager::SfxSpan(hnd, 0);
  if (underwater)
    CSfxManager::PitchBend(hnd, 0);
  return hnd;
}

CDamageInfo get_shot_damage(const CDamageInfo& shotParam, CStateManager& mgr) {
  CDamageInfo result = shotParam;
  if (gpGameState->GetHardMode()) {
    result.MultiplyDamage(gpGameState->GetHardModeWeaponMultiplier());
  }
  return result;
}

int get_current_suit(const CStateManager& mgr) {
  const CPlayerState* state = mgr.GetPlayerState();
  CPlayerState::EPlayerSuit suit = state->GetCurrentSuit();
  if (suit < CPlayerState::kPS_Power || suit > CPlayerState::kPS_FusionGravity)
    suit = CPlayerState::kPS_Power;
  if (suit == CPlayerState::kPS_FusionPower)
    suit = CPlayerState::EPlayerSuit(int(suit) + int(state->GetCurrentSuitRaw()));
  return suit;
}

} // namespace NWeaponTypes
