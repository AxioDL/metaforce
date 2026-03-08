#pragma once

#include <memory>

#include "Runtime/Audio/CSfxManager.hpp"

namespace metaforce {

class CMidiManager {
public:
  class CMidiData {
    u16 x0_setupId;
    u16 x2_groupId;
    CAssetId x4_agscId;
    std::unique_ptr<u8[]> x8_arrData;

  public:
    u16 GetSetupId() const { return x0_setupId; }
    u16 GetGroupId() const { return x2_groupId; }
    CAssetId GetAGSCAssetId() const { return x4_agscId; }
    const u8* GetArrData() const { return x8_arrData.get(); }
    explicit CMidiData(CInputStream& in);
  };

  class CMidiWrapper {
    u32 x0_sysHandle = 0;
    u16 x8_songId = 0;
    bool xa_available = true;

  public:
    u32 GetAudioSysHandle() const { return x0_sysHandle; }
    void SetAudioSysHandle(u32 handle) { x0_sysHandle = handle; }
    bool IsAvailable() const { return xa_available; }
    void SetAvailable(bool available) { xa_available = available; }
    u16 GetSongId() const { return x8_songId; }
    void SetSongId(u16 songId) { x8_songId = songId; }
  };
  using CMidiHandle = std::shared_ptr<CMidiWrapper>;

  static void StopAll();
  static void Stop(const CMidiHandle& handle, float fadeTime);
  static std::unordered_set<CMidiHandle>::iterator Stop(std::unordered_set<CMidiHandle>::iterator handle,
                                                        float fadeTime);
  static CMidiHandle Play(const CMidiData& data, float fadeTime, bool stopExisting, float volume);

private:
  static std::unordered_set<CMidiHandle> m_MidiWrappers;
};

CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& parms,
                                  CObjectReference* selfRef);

using CMidiHandle = CMidiManager::CMidiHandle;

} // namespace metaforce
