#pragma once

#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/CGameState.hpp"

namespace metaforce {
class CArchitectureQueue;
class CControllerRecorder {
public:
  enum class EMode {
    None,
    Record,
    Play
  };
private:
  std::map<int, CFinalInput> m_finalInputs; // frame -> input
  EMode m_mode = EMode::None;
  CGameState m_initialState;
  const CGameState* m_oldGameStatePtr = nullptr;
public:
  CControllerRecorder() = default;

  bool ProcessInput(const CFinalInput& in, s32 frame, CArchitectureQueue& queue);
  void SetMode(EMode mode);
  void PutTo(COutputStream& out);
};

} // namespace metaforce
