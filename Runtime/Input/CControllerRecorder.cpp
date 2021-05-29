#include "Runtime/Input/CControllerRecorder.hpp"

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CArchitectureMessage.hpp"

namespace metaforce {
bool CControllerRecorder::ProcessInput(const CFinalInput& in, s32 frame, CArchitectureQueue& queue) {
  if (m_mode == EMode::Record) {
    m_finalInputs[frame] = in;
    return false;
  } else {
    SetMode(EMode::Record);
  }
  if (m_mode == EMode::Play && m_finalInputs.find(frame) != m_finalInputs.end()) {
    queue.Push(MakeMsg::CreateUserInput(EArchMsgTarget::Game, m_finalInputs[frame]));
    return true;
  }

  return false;
}

void CControllerRecorder::SetMode(EMode mode) {
  EMode oldMode = m_mode;
  m_mode = mode;
  if (m_mode == EMode::Record && oldMode != EMode::Record) {
    m_initialState = *g_GameState;
  } else if (oldMode == EMode::Record && m_mode == EMode::None) {
    //auto w = athena::io::FileWriter("/home/antidote/Documents/test.inp");
    //PutTo(w);
  } else if (oldMode == EMode::None && m_mode == EMode::Play) {
    m_oldGameStatePtr = g_GameState;
    g_GameState = &m_initialState;
  } else if (oldMode == EMode::Play && m_mode == EMode::None) {
  }
}

void CControllerRecorder::PutTo(COutputStream& out) {
  out.writeUint32Big(1); // Version
  m_initialState.WriteBackupBuf();
  auto& buf = m_initialState.BackupBuf();
  out.writeUint32Big(buf.size());
  out.writeBytes(buf.data(), buf.size());
  out.writeUint32Big(m_finalInputs.size());
  for (auto pair : m_finalInputs) {
    out.writeUint32Big(pair.first);
    pair.second.PutTo(out);
  }
}
}
