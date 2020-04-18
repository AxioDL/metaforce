#pragma once

#include "Runtime/RetroTypes.hpp"

#include <amuse/amuse.hpp>
#include <boo/boo.hpp>
#include <boo/audiodev/IAudioVoiceEngine.hpp>
#include <hecl/Runtime.hpp>
#include "DataSpec/DNACommon/URDEVersionInfo.hpp"

namespace hecl {
class Console;
class CVarManager;
} // namespace hecl

namespace urde {
using ERegion = DataSpec::ERegion;
using EGame = DataSpec::EGame;

class CStopwatch;
enum class EGameplayResult { None, Win, Lose, Playing };

enum class EFlowState {
  None,
  WinBad,
  WinGood,
  WinBest,
  LoseGame,
  Default,
  StateSetter,
};

class IMain {
public:
  virtual ~IMain() = default;
  virtual void Init(const hecl::Runtime::FileStoreManager& storeMgr, hecl::CVarManager* cvarMgr, boo::IWindow* window,
                    boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend) = 0;
  virtual void Draw() = 0;
  virtual bool Proc() = 0;
  virtual void Shutdown() = 0;
  virtual boo::IWindow* GetMainWindow() const = 0;
  virtual EFlowState GetFlowState() const = 0;
  virtual void SetFlowState(EFlowState) = 0;
  virtual size_t GetExpectedIdSize() const = 0;
  virtual void WarmupShaders() = 0;
  virtual hecl::Console* Console() const = 0;
  virtual EGame GetGame() const =0;
  virtual ERegion GetRegion() const =0;
  virtual bool IsPAL() const = 0;
  virtual bool IsJapanese() const = 0;
  virtual bool IsUSA() const = 0;
  virtual bool IsTrilogy() const = 0;
  virtual std::string_view GetVersionString() const=0;
};
} // namespace urde
