#include "WeaponCommon.hpp"
#include "Character/CAnimData.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Character/CPrimitive.hpp"
#include "Audio/CSfxManager.hpp"
#include "CStateManager.hpp"

namespace urde::NWeaponTypes {

void primitive_set_to_token_vector(const CAnimData& animData, const std::set<CPrimitive>& primSet,
                                   std::vector<CToken>& tokensOut, bool preLock) {
  int eventCount = 0;
  for (const CPrimitive& prim : primSet)
    if (animData.GetEventResourceIdForAnimResourceId(prim.GetAnimResId()).IsValid())
      ++eventCount;

  tokensOut.clear();
  tokensOut.reserve(primSet.size() + eventCount);

  SObjectTag atag{FOURCC('ANIM'), 0};
  SObjectTag etag{FOURCC('EVNT'), 0};
  for (const CPrimitive& prim : primSet) {
    CAssetId eId = animData.GetEventResourceIdForAnimResourceId(prim.GetAnimResId());
    if (eId.IsValid()) {
      etag.id = prim.GetAnimResId();
      tokensOut.push_back(g_SimplePool->GetObj(etag));
      if (preLock)
        tokensOut.back().Lock();
    }
    atag.id = prim.GetAnimResId();
    tokensOut.push_back(g_SimplePool->GetObj(atag));
    if (preLock)
      tokensOut.back().Lock();
  }
}

void unlock_tokens(std::vector<CToken>& anims) {
  for (CToken& tok : anims)
    tok.Unlock();
}

void lock_tokens(std::vector<CToken>& anims) {
  for (CToken& tok : anims)
    tok.Lock();
}

bool are_tokens_ready(const std::vector<CToken>& anims) {
  for (const CToken& tok : anims)
    if (!tok.IsLoaded())
      return false;
  return true;
}

void get_token_vector(const CAnimData& animData, int begin, int end, std::vector<CToken>& tokensOut, bool preLock) {
  std::set<CPrimitive> prims;
  for (int i = begin; i < end; ++i) {
    CAnimPlaybackParms parms(i, -1, 1.f, true);
    animData.GetAnimationPrimitives(parms, prims);
  }
  primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

void get_token_vector(const CAnimData& animData, int animIdx, std::vector<CToken>& tokensOut, bool preLock) {
  std::set<CPrimitive> prims;
  CAnimPlaybackParms parms(animIdx, -1, 1.f, true);
  animData.GetAnimationPrimitives(parms, prims);
  primitive_set_to_token_vector(animData, prims, tokensOut, preLock);
}

void do_sound_event(std::pair<u16, CSfxHandle>& sfxHandle, float& pitch, bool doPitchBend, u32 soundId, float weight,
                    u32 flags, float falloff, float maxDist, float minVol, float maxVol,
                    const zeus::CVector3f& posToCam, const zeus::CVector3f& pos, TAreaId aid, CStateManager& mgr) {
  if (posToCam.magSquared() >= maxDist * maxDist)
    return;

  u16 useSfxId = CSfxManager::TranslateSFXID(u16(soundId));
  u32 useFlags = 0x1; // Continuous parameter update
  if ((flags & 0x8) != 0)
    useFlags |= 0x8; // Doppler effect
  bool useAcoustics = (flags & 0x80) == 0;

  CAudioSys::C3DEmitterParmData parms;
  parms.x0_pos = pos;
  parms.xc_dir = zeus::skUp;
  parms.x18_maxDist = maxDist;
  parms.x1c_distComp = falloff;
  parms.x20_flags = useFlags;
  parms.x24_sfxId = useSfxId;
  parms.x26_maxVol = maxVol;
  parms.x27_minVol = minVol;
  parms.x28_important = false;
  parms.x29_prio = 0x7f;

  if (mgr.GetActiveRandom()->Float() <= weight) {
    if ((soundId & 0x80000000) != 0) {
      if (!sfxHandle.second) {
        CSfxHandle hnd;
        if ((soundId & 0x40000000) != 0)
          hnd = CSfxManager::SfxStart(useSfxId, 1.f, 0.f, true, 0x7f, true, aid);
        else
          hnd = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
        if (hnd) {
          sfxHandle.first = useSfxId;
          sfxHandle.second = hnd;
          if (doPitchBend)
            CSfxManager::PitchBend(hnd, pitch);
        }
      } else {
        if (sfxHandle.first == useSfxId) {
          CSfxManager::UpdateEmitter(sfxHandle.second, parms.x0_pos, parms.xc_dir, parms.x26_maxVol);
        } else if ((flags & 0x4) != 0) // Pausable
        {
          CSfxManager::RemoveEmitter(sfxHandle.second);
          CSfxHandle hnd = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
          if (hnd) {
            sfxHandle.first = useSfxId;
            sfxHandle.second = hnd;
            if (doPitchBend)
              CSfxManager::PitchBend(hnd, pitch);
          }
        }
      }
    } else {
      CSfxHandle hnd;
      if ((soundId & 0x40000000) != 0)
        hnd = CSfxManager::SfxStart(useSfxId, 1.f, 0.f, true, 0x7f, false, aid);
      else
        hnd = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, false, aid);
      if (doPitchBend)
        CSfxManager::PitchBend(hnd, pitch);
    }
  }
}

CAssetId get_asset_id_from_name(const char* name) {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName(name);
  if (!tag)
    return {};
  return tag->id;
}

CPlayerState::EPlayerSuit get_current_suit(const CStateManager& mgr) {
  CPlayerState::EPlayerSuit suit = mgr.GetPlayerState()->GetCurrentSuit();
  if (suit < CPlayerState::EPlayerSuit::Power || suit > CPlayerState::EPlayerSuit::FusionGravity)
    suit = CPlayerState::EPlayerSuit::Power;
  if (suit == CPlayerState::EPlayerSuit::FusionPower)
    suit = CPlayerState::EPlayerSuit(int(suit) + int(mgr.GetPlayerState()->GetCurrentSuitRaw()));
  return suit;
}

CSfxHandle play_sfx(u16 sfx, bool underwater, bool looped, float pan) {
  CSfxHandle hnd = CSfxManager::SfxStart(sfx, 1.f, pan, true, 0x7f, looped, kInvalidAreaId);
  CSfxManager::SfxSpan(hnd, 0.f);
  if (underwater)
    CSfxManager::PitchBend(hnd, -1.f);
  return hnd;
}

} // namespace urde::NWeaponTypes
