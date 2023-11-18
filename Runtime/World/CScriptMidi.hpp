#pragma once

#include <string_view>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Audio/CMidiManager.hpp"
#include "Runtime/World/CEntity.hpp"

namespace metaforce {

class CScriptMidi : public CEntity {
  TToken<CMidiManager::CMidiData> x34_song;
  CMidiHandle x3c_handle;
  float x40_fadeInTime;
  float x44_fadeOutTime;
  u16 x48_volume;

  void StopInternal(float fadeTime);

public:
  DEFINE_ENTITY
  CScriptMidi(TUniqueId id, const CEntityInfo& info, std::string_view name, bool active, CAssetId csng, float, float,
              s32);
  ~CScriptMidi() override;

  void Stop(CStateManager& mgr, float fadeTime);
  void Play(CStateManager& mgr, float fadeTime);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

} // namespace metaforce
