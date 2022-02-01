#pragma once

#include "Runtime/RetroTypes.hpp"

#include "DataSpec/DNACommon/MetaforceVersionInfo.hpp"
#include "Runtime/CMainFlowBase.hpp"
#include <amuse/amuse.hpp>
#include <boo/audiodev/IAudioVoiceEngine.hpp>
#include <boo/boo.hpp>
#include <hecl/Runtime.hpp>

namespace hecl {
class Console;
class CVarManager;
} // namespace hecl

namespace metaforce {
using ERegion = DataSpec::ERegion;
using EGame = DataSpec::EGame;

class CStopwatch;
enum class EGameplayResult { None, Win, Lose, Playing };

class IMain {
public:
  virtual ~IMain() = default;
  virtual void Init(const hecl::Runtime::FileStoreManager& storeMgr, hecl::CVarManager* cvarMgr,
                    boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend) = 0;
  virtual void Draw() = 0;
  virtual bool Proc(float dt) = 0;
  virtual void Shutdown() = 0;
  virtual EClientFlowStates GetFlowState() const = 0;
  virtual void SetFlowState(EClientFlowStates) = 0;
  virtual size_t GetExpectedIdSize() const = 0;
  virtual void WarmupShaders() = 0;
  virtual EGame GetGame() const = 0;
  virtual ERegion GetRegion() const = 0;
  virtual bool IsPAL() const = 0;
  virtual bool IsJapanese() const = 0;
  virtual bool IsUSA() const = 0;
  virtual bool IsTrilogy() const = 0;
  virtual std::string_view GetVersionString() const = 0;
  virtual void Quit() = 0;
  virtual bool IsPaused() const = 0;
  virtual void SetPaused(bool b) = 0;
};
} // namespace metaforce
