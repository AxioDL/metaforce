#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/World/CActor.hpp"

namespace urde {

class CScriptSound : public CActor {
  static bool sFirstInFrame;

  float xe8_occUpdateTimer = 0.f;
  CSfxHandle xec_sfxHandle;
  float xf0_maxVol = 0.f;
  float xf2_maxVolUpd = 0.f;
  float xf4_maxVolUpdDelta = 0.f;
  float xf8_updateTimer = 0.f;
  float xfc_startDelay;
  u16 x100_soundId;
  float x104_maxDist;
  float x108_distComp;
  float x10c_minVol;
  float x10e_vol;
  s16 x110_;
  s16 x112_prio;
  float x114_pan;
  bool x116_;
  float x118_pitch;
  bool x11c_24_playRequested : 1 = false;
  bool x11c_25_looped : 1;
  bool x11c_26_nonEmitter : 1;
  bool x11c_27_autoStart : 1;
  bool x11c_28_occlusionTest : 1;
  bool x11c_29_acoustics : 1;
  bool x11c_30_worldSfx : 1;
  bool x11c_31_selfFree : 1 = false;
  bool x11d_24_allowDuplicates : 1;
  bool x11d_25_processedThisFrame : 1 = false;

  static float GetOccludedVolumeAmount(const zeus::CVector3f& pos, const CStateManager& mgr);

public:
  CScriptSound(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, u16 soundId,
               bool active, float maxDist, float distComp, float startDelay, u32 minVol, u32 vol, u32 w3, u32 prio,
               u32 pan, u32 w6, bool looped, bool nonEmitter, bool autoStart, bool occlusionTest, bool acoustics,
               bool worldSfx, bool allowDuplicates, s32 pitch);

  void Accept(IVisitor& visitor) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override {}
  void PreThink(float, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PlaySound(CStateManager&);
  void StopSound(CStateManager&);
};
} // namespace urde
