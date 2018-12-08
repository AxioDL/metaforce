#include "CMidiManager.hpp"

namespace urde {

std::unordered_set<CMidiHandle> CMidiManager::m_MidiWrappers = {};

void CMidiManager::StopAll() {
  for (auto it = m_MidiWrappers.begin(); it != m_MidiWrappers.end();)
    it = Stop(it, 0.f);
}

void CMidiManager::Stop(const CMidiHandle& handle, float fadeTime) {
  handle->GetAudioSysHandle()->stopSong(fadeTime);
  m_MidiWrappers.erase(handle);
}

std::unordered_set<CMidiHandle>::iterator CMidiManager::Stop(std::unordered_set<CMidiHandle>::iterator handle,
                                                             float fadeTime) {
  const CMidiHandle& h = *handle;
  h->GetAudioSysHandle()->stopSong(fadeTime);
  return m_MidiWrappers.erase(handle);
}

CMidiHandle CMidiManager::Play(const CMidiData& data, float fadeTime, bool stopExisting, float volume) {
  if (stopExisting)
    for (auto it = m_MidiWrappers.begin(); it != m_MidiWrappers.end();)
      it = Stop(it, fadeTime);

  CMidiHandle handle = *m_MidiWrappers.insert(std::make_shared<CMidiWrapper>()).first;
  handle->SetAudioSysHandle(
      CAudioSys::GetAmuseEngine().seqPlay(data.GetGroupId(), data.GetSetupId(), data.GetArrData()));
  handle->GetAudioSysHandle()->setVolume(volume, fadeTime);
  handle->SetSongId(data.GetSetupId());
  return handle;
}

CMidiManager::CMidiData::CMidiData(CInputStream& in) {
  in.readUint32Big();
  x0_setupId = in.readUint32Big();
  x2_groupId = in.readUint32Big();
  x4_agscId = in.readUint32Big();
  u32 length = in.readUint32Big();
  x8_arrData.reset(new u8[length]);
  in.readUBytesToBuf(x8_arrData.get(), length);
}

CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& parms,
                                  CObjectReference* selfRef) {
  return TToken<CMidiManager::CMidiData>::GetIObjObjectFor(std::make_unique<CMidiManager::CMidiData>(in));
}

} // namespace urde
