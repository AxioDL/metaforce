#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/CMainFlowBase.hpp"
#include "Runtime/ConsoleVariables/FileStoreManager.hpp"

#include <amuse/amuse.hpp>
#include <boo/audiodev/IAudioVoiceEngine.hpp>
#include <boo/boo.hpp>

namespace metaforce {
class Console;
class CVarManager;
enum class ERegion { Invalid = -1, NTSC_U = 'E', PAL = 'P', NTSC_J = 'J' };
enum class EGame {
  Invalid = 0,
  MetroidPrime1,
  MetroidPrime2,
  MetroidPrime3,
};

struct MetaforceVersionInfo {
  std::string version;
  ERegion region;
  EGame game;
  bool isTrilogy;
};

class CStopwatch;
enum class EGameplayResult { None, Win, Lose, Playing };

class IMain {
public:
  virtual ~IMain() = default;
  virtual void Init(const FileStoreManager& storeMgr, CVarManager* cvarMgr,
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
