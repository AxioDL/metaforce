#include "Kyoto/Audio/CSfxHandle.hpp"
#include <Kyoto/Audio/CMidiManager.hpp>

#include "Kyoto/Audio/CAudioSys.hpp"

#include <Kyoto/CFactoryFnReturn.hpp>
#include <Kyoto/Streams/CInputStream.hpp>

#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include "Metaforce/Common.hpp"

namespace {
constexpr borealis::Log Log{"CMidiManager"};
}
#endif

rstl::reserved_vector< CMidiManager::CMidiWrapper, 3 > CMidiManager::mMidiWrappers;

CMidiManager::CMidiWrapper::CMidiWrapper()
: mSysHandle(0)
#if defined(TARGET_PC)
, mSongId(-1)
#endif
, mAvailable(true) {
}

const CSfxHandle& CMidiManager::CMidiWrapper::GetManagerHandle() const { return mMidiHandle; }

const u32 CMidiManager::CMidiWrapper::GetAudioSysHandle() const { return mSysHandle; }

const bool CMidiManager::CMidiWrapper::IsAvailable() const { return mAvailable; }

const short CMidiManager::CMidiWrapper::GetSongId() const { return mSongId; }

void CMidiManager::CMidiWrapper::SetAvailable(const bool v) { mAvailable = v; }

void CMidiManager::CMidiWrapper::SetAudioSysHandle(const u32 handle) { mSysHandle = handle; }

void CMidiManager::CMidiWrapper::SetMidiHandle(const CSfxHandle& handle) { mMidiHandle = handle; }

void CMidiManager::CMidiWrapper::SetSongId(const short id) { mSongId = id; }

CSfxHandle CMidiManager::Play(const CMidiData& data, unsigned short fadeTime, bool stopExisting,
                              short volume) {
  bool foundExisting = false;
  u32 sysHandle = 0;
  CSfxHandle handle = LocateHandle();
  if (!handle) {
    return CSfxHandle();
  }
  CMidiWrapper& wrapper = mMidiWrappers[handle.GetIndex()];
  wrapper.SetAvailable(false);
  wrapper.SetMidiHandle(handle);
  if (stopExisting) {
    for (int i = 0; i < mMidiWrappers.size(); ++i) {
#if defined(TARGET_PC)
      if (&mMidiWrappers[i] == &wrapper) {
        continue;
      }
#endif
      if (mMidiWrappers[i].IsAvailable()) {
        continue;
      }

      if (data.GetSongId() == mMidiWrappers[i].GetSongId()) {
        foundExisting = true;
        sysHandle = mMidiWrappers[i].GetAudioSysHandle();
        mMidiWrappers[i].SetAvailable(true);
      } else {
        Stop(mMidiWrappers[i].GetManagerHandle(), fadeTime);
      }
    }
  }

  if (foundExisting) {
    wrapper.SetAudioSysHandle(sysHandle);
    wrapper.SetSongId(data.GetSongId());
  } else {
    u32 sysHandle =
        CAudioSys::SeqPlayEx(data.GetGroupId(), data.GetSongId(), data.GetData(), nullptr, 0);
#if defined(TARGET_PC)
    if (sysHandle == SND_ID_ERROR) {
      wrapper.SetAvailable(true);
      return {};
    }
#endif
    if (fadeTime != 0) {
      CAudioSys::SeqVolume(0, 0, sysHandle, 0);
    }
    CAudioSys::SeqVolume(volume, fadeTime, sysHandle, 0);
    wrapper.SetAudioSysHandle(sysHandle);
    wrapper.SetSongId(data.GetSongId());
  }

  return handle;
}

void CMidiManager::Stop(const CSfxHandle& handle, ushort fadeTime) {
  if (!handle) {
    return;
  }

  if (handle != mMidiWrappers[handle.GetIndex()].GetManagerHandle()) {
    return;
  }

  u32 sysHandle = mMidiWrappers[handle.GetIndex()].GetAudioSysHandle();
  if (fadeTime == 0) {
    CAudioSys::SeqStop(sysHandle);
  } else {
    CAudioSys::SeqVolume(0, fadeTime, sysHandle, 1);
  }

  mMidiWrappers[handle.GetIndex()].SetAvailable(true);
}

void CMidiManager::StopAll() {
  for (int i = 0; i < mMidiWrappers.size(); ++i) {
    if (!mMidiWrappers[i].IsAvailable()) {
      Stop(mMidiWrappers[i].GetManagerHandle(), 0);
    }
  }
}

CSfxHandle CMidiManager::LocateHandle() {
  for (int i = 0; i < mMidiWrappers.size(); ++i) {
    if (mMidiWrappers[i].IsAvailable()) {
      return CSfxHandle(i);
    }
  }

  if (mMidiWrappers.size() == mMidiWrappers.capacity()) {
    return CSfxHandle();
  }

  mMidiWrappers.push_back(CMidiWrapper());
  return CSfxHandle(mMidiWrappers.size() - 1);
}

CMidiManager::CMidiData::CMidiData(CInputStream& in) : mSongId(-1), mGroupId(-1), mAgscId(-1) {
#if defined(TARGET_PC)
  u8 bytes[20];
  metaforce::AudioSongHeader header{};
  REQUIRE(in.ReadBytes(bytes, sizeof(bytes)) == sizeof(bytes) &&
              metaforce::ReadAudioSongHeader(bytes, header),
          "Invalid CSNG header");
  mSongId = header.song;
  mGroupId = header.group;
  mAgscId = header.audioGroup;
  mData = rs_new uchar[header.length];
  REQUIRE(in.ReadBytes(mData.get(), header.length) == header.length,
          "Truncated CSNG arrangement");
  SND_PC_ASSET_ERROR error{};
  REQUIRE(sndPCValidateArrangement({mData.get(), header.length}, &error),
          "Invalid CSNG arrangement at {}: {}", error.offset, error.reason ? error.reason : "");
#else
  in.ReadLong();
  mSongId = in.ReadLong();
  mGroupId = in.ReadLong();
  mAgscId = in.ReadLong();
  int len = in.ReadInt32();
  mData = rs_new uchar[len];
  in.Get(mData.get(), len);
#endif
}

const CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in,
                                        const CVParamTransfer&) {
  return rs_new CMidiManager::CMidiData(in);
}
