#pragma once

#include "CEntity.hpp"

namespace urde {

class CScriptStreamedMusic : public CEntity {
  std::string x34_fileName;
  bool x44_noStopOnDeactivate;
  bool x45_fileIsDsp; // As opposed to .adp for DTK streaming
  bool x46_loop;
  bool x47_music;
  float x48_fadeIn;
  float x4c_fadeOut;
  u32 x50_volume;
  static bool IsDSPFile(std::string_view fileName);
  void StopStream(CStateManager& mgr);
  void StartStream(CStateManager& mgr);
  void TweakOverride(CStateManager& mgr);

public:
  CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, std::string_view name, bool active,
                       std::string_view fileName, bool noStopOnDeactivate, float fadeIn, float fadeOut, u32 volume,
                       bool loop, bool music);

  void Stop(CStateManager& mgr);
  void Play(CStateManager& mgr);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

} // namespace urde
