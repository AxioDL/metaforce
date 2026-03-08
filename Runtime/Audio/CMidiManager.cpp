#include "Runtime/Audio/CMidiManager.hpp"

#include "Runtime/Audio/CAudioSys.hpp"
#include "Runtime/Streams/CInputStream.hpp"

#include <cstddef>
#include <vector>

#include <musyx/seq.h>

namespace metaforce {

namespace {

static inline u16 SwapU16(u16 v) { return __builtin_bswap16(v); }
static inline u32 SwapU32(u32 v) { return __builtin_bswap32(v); }

static bool IsRangeValid(u32 off, u32 size, u32 len) { return off <= len && size <= len - off; }

static void SwapNoteData(NOTE_DATA* note) {
  while (true) {
    note->time = SwapU16(note->time);
    note->length = SwapU16(note->length);
    if (note->key == 0xFF && note->velocity == 0xFF)
      break;
    ++note;
  }
}

static void SwapArrEndian(u8* data, u32 len) {
  if (data == nullptr || len < sizeof(ARR))
    return;

  ARR* arr = reinterpret_cast<ARR*>(data);
  arr->tTab = SwapU32(arr->tTab);
  arr->pTab = SwapU32(arr->pTab);
  arr->tmTab = SwapU32(arr->tmTab);
  arr->mTrack = SwapU32(arr->mTrack);
  arr->info = SwapU32(arr->info);
  for (u32& loopPoint : arr->loopPoint)
    loopPoint = SwapU32(loopPoint);
  arr->tsTab = SwapU32(arr->tsTab);

  if (!IsRangeValid(arr->tTab, 64 * sizeof(u32), len))
    return;

  u32* trackTab = ARR_GET_TYPE(arr, arr->tTab, u32*);
  u16 maxPattern = 0;
  bool foundPattern = false;

  for (u32 i = 0; i < 64; ++i) {
    trackTab[i] = SwapU32(trackTab[i]);
    if (trackTab[i] == 0 || !IsRangeValid(trackTab[i], sizeof(TENTRY), len))
      continue;

    TENTRY* entry = ARR_GET_TYPE(arr, trackTab[i], TENTRY*);
    while (IsRangeValid(static_cast<u32>(reinterpret_cast<u8*>(entry) - data), sizeof(TENTRY), len)) {
      entry->time = SwapU32(entry->time);
      entry->pattern = SwapU16(entry->pattern);
      if (entry->time == 0xFFFFFFFFu)
        break;
      if (!foundPattern || entry->pattern > maxPattern) {
        maxPattern = entry->pattern;
        foundPattern = true;
      }
      ++entry;
    }
  }

  if (foundPattern && IsRangeValid(arr->pTab, (static_cast<u32>(maxPattern) + 1) * sizeof(u32), len)) {
    u32* patternTab = ARR_GET_TYPE(arr, arr->pTab, u32*);
    for (u32 i = 0; i <= maxPattern; ++i)
      patternTab[i] = SwapU32(patternTab[i]);

    std::vector<bool> patternVisited(maxPattern + 1, false);
    for (u32 i = 0; i < 64; ++i) {
      if (trackTab[i] == 0 || !IsRangeValid(trackTab[i], sizeof(TENTRY), len))
        continue;

      TENTRY* entry = ARR_GET_TYPE(arr, trackTab[i], TENTRY*);
      while (IsRangeValid(static_cast<u32>(reinterpret_cast<u8*>(entry) - data), sizeof(TENTRY), len)) {
        if (entry->time == 0xFFFFFFFFu)
          break;

        if (entry->pattern <= maxPattern && !patternVisited[entry->pattern]) {
          patternVisited[entry->pattern] = true;
          u32 patternOff = patternTab[entry->pattern];
          if (patternOff != 0 && IsRangeValid(patternOff, sizeof(SEQ_PATTERN), len)) {
            SEQ_PATTERN* pattern = ARR_GET_TYPE(arr, patternOff, SEQ_PATTERN*);
            pattern->headerLen = SwapU32(pattern->headerLen);
            pattern->pitchBend = SwapU32(pattern->pitchBend);
            pattern->modulation = SwapU32(pattern->modulation);
            if (IsRangeValid(patternOff + offsetof(SEQ_PATTERN, noteData), sizeof(NOTE_DATA), len))
              SwapNoteData(reinterpret_cast<NOTE_DATA*>(&pattern->noteData));
          }
        }

        ++entry;
      }
    }
  }

  if (arr->mTrack != 0 && IsRangeValid(arr->mTrack, sizeof(MTRACK_DATA), len)) {
    MTRACK_DATA* track = ARR_GET_TYPE(arr, arr->mTrack, MTRACK_DATA*);
    while (IsRangeValid(static_cast<u32>(reinterpret_cast<u8*>(track) - data), sizeof(MTRACK_DATA), len)) {
      track->time = SwapU32(track->time);
      track->bpm = SwapU32(track->bpm);
      if (track->time == 0xFFFFFFFFu)
        break;
      ++track;
    }
  }
}

} // namespace

std::unordered_set<CMidiHandle> CMidiManager::m_MidiWrappers = {};

void CMidiManager::StopAll() {
  for (auto it = m_MidiWrappers.begin(); it != m_MidiWrappers.end();)
    it = Stop(it, 0.f);
}

void CMidiManager::Stop(const CMidiHandle& handle, float fadeTime) {
  u32 sysHandle = handle->GetAudioSysHandle();
  u16 fadeMs = static_cast<u16>(fadeTime * 1000.f);
  if (fadeMs == 0) {
    CAudioSys::SeqStop(sysHandle);
  } else {
    CAudioSys::SeqVolume(0, fadeMs, sysHandle, SND_SEQVOL_STOP);
  }
  handle->SetAvailable(true);
  m_MidiWrappers.erase(handle);
}

std::unordered_set<CMidiHandle>::iterator CMidiManager::Stop(std::unordered_set<CMidiHandle>::iterator handle,
                                                             float fadeTime) {
  const CMidiHandle& h = *handle;
  u32 sysHandle = h->GetAudioSysHandle();
  u16 fadeMs = static_cast<u16>(fadeTime * 1000.f);
  if (fadeMs == 0) {
    CAudioSys::SeqStop(sysHandle);
  } else {
    CAudioSys::SeqVolume(0, fadeMs, sysHandle, SND_SEQVOL_STOP);
  }
  h->SetAvailable(true);
  return m_MidiWrappers.erase(handle);
}

CMidiHandle CMidiManager::Play(const CMidiData& data, float fadeTime, bool stopExisting, float volume) {
  if (stopExisting)
    for (auto it = m_MidiWrappers.begin(); it != m_MidiWrappers.end();)
      it = Stop(it, fadeTime);

  CMidiHandle handle = *m_MidiWrappers.insert(std::make_shared<CMidiWrapper>()).first;

  u16 fadeMs = static_cast<u16>(fadeTime * 1000.f);
  u8 vol = static_cast<u8>(volume * 127.f);

  u32 sysHandle = CAudioSys::SeqPlayEx(data.GetGroupId(), data.GetSetupId(),
                                       const_cast<u8*>(data.GetArrData()), nullptr, 0);
  if (fadeMs != 0) {
    CAudioSys::SeqVolume(0, 0, sysHandle, SND_SEQVOL_CONTINUE);
  }
  CAudioSys::SeqVolume(vol, fadeMs, sysHandle, SND_SEQVOL_CONTINUE);

  handle->SetAudioSysHandle(sysHandle);
  handle->SetSongId(data.GetSetupId());
  handle->SetAvailable(false);

  return handle;
}

CMidiManager::CMidiData::CMidiData(CInputStream& in) {
  in.ReadLong();
  x0_setupId = in.ReadLong();
  x2_groupId = in.ReadLong();
  x4_agscId = in.Get<CAssetId>();
  u32 length = in.ReadLong();
  x8_arrData.reset(new u8[length]);
  in.ReadBytes(reinterpret_cast<char*>(x8_arrData.get()), length);
  SwapArrEndian(x8_arrData.get(), length);
}

CFactoryFnReturn FMidiDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& parms,
                                  CObjectReference* selfRef) {
  return TToken<CMidiManager::CMidiData>::GetIObjObjectFor(std::make_unique<CMidiManager::CMidiData>(in));
}

} // namespace metaforce
