#include "Runtime/Audio/CAudioGroupSet.hpp"

#include <cstring>

#include "Runtime/Streams/CMemoryInStream.hpp"

#include <musyx/seq.h>
#include <musyx/synthdata.h>
#include <musyx/hardware.h>
#include <musyx/stream.h>

namespace metaforce {

static inline u16 SwapU16(u16 v) { return __builtin_bswap16(v); }
static inline u32 SwapU32(u32 v) { return __builtin_bswap32(v); }
static inline s16 SwapS16(s16 v) { return static_cast<s16>(__builtin_bswap16(static_cast<u16>(v))); }

void CAudioGroupSet::SwapProjectEndian() {
  // Project data is a linked list of GROUP_DATA structures
  // Each GROUP_DATA contains offsets to sub-tables within the project
  u8* base = m_proj;
  u32 offset = 0;

  while (offset < m_projLen) {
    GROUP_DATA* g = reinterpret_cast<GROUP_DATA*>(base + offset);

    // Swap GROUP_DATA fields
    g->nextOff = SwapU32(g->nextOff);
    g->id = SwapU16(g->id);
    g->type = SwapU16(g->type);
    g->macroOff = SwapU32(g->macroOff);
    g->sampleOff = SwapU32(g->sampleOff);
    g->curveOff = SwapU32(g->curveOff);
    g->keymapOff = SwapU32(g->keymapOff);
    g->layerOff = SwapU32(g->layerOff);
    g->data.song.normpageOff = SwapU32(g->data.song.normpageOff);
    g->data.song.drumpageOff = SwapU32(g->data.song.drumpageOff);
    g->data.song.midiSetupOff = SwapU32(g->data.song.midiSetupOff);

    // Swap the ID reference lists (u16 lists terminated by 0xFFFF)
    // macroOff points to a list of u16 macro IDs
    auto swapU16List = [&](u32 listOff) {
      if (listOff == 0 || listOff >= m_projLen) return;
      u16* list = reinterpret_cast<u16*>(base + listOff);
      while (reinterpret_cast<u8*>(list) < base + m_projLen) {
        *list = SwapU16(*list);
        if (*list == 0xFFFF) break;
        list++;
      }
    };

    swapU16List(g->macroOff);
    swapU16List(g->sampleOff);
    swapU16List(g->curveOff);
    swapU16List(g->keymapOff);
    swapU16List(g->layerOff);

    if (g->type == 0) {
      auto swapPages = [&](u32 pageOff) {
        if (pageOff == 0 || pageOff >= m_projLen) return;
        PAGE* page = reinterpret_cast<PAGE*>(base + pageOff);
        while (reinterpret_cast<u8*>(page) + sizeof(PAGE) <= base + m_projLen) {
          page->macro = SwapU16(page->macro);
          if (page->index == 0xFF)
            break;
          ++page;
        }
      };

      auto swapMidiSetups = [&](u32 setupOff) {
        if (setupOff == 0 || setupOff >= m_projLen) return;
        MIDISETUP* setup = reinterpret_cast<MIDISETUP*>(base + setupOff);
        while (reinterpret_cast<u8*>(setup) + sizeof(MIDISETUP) <= base + m_projLen) {
          setup->songId = SwapU16(setup->songId);
          setup->reserved = SwapU16(setup->reserved);
          if (setup->songId == 0xFFFF)
            break;
          ++setup;
        }
      };

      swapPages(g->data.song.normpageOff);
      swapPages(g->data.song.drumpageOff);
      swapMidiSetups(g->data.song.midiSetupOff);
    }

    // Swap FX table if type == 1 (FX group)
    if (g->type == 1 && g->data.song.normpageOff < m_projLen) {
      FX_DATA* fxData = reinterpret_cast<FX_DATA*>(base + g->data.song.normpageOff);
      fxData->num = SwapU16(fxData->num);
      fxData->reserverd = SwapU16(fxData->reserverd);
      u16 numFx = fxData->num;
      for (u16 i = 0; i < numFx; ++i) {
        FX_TAB* fx = &fxData->fx[i];
        fx->id = SwapU16(fx->id);
        fx->macro = SwapU16(fx->macro);
        // volume, panning, key, vGroup, maxVoices, priority are u8 - no swap needed
      }
    }

    if (g->nextOff == 0xFFFFFFFF)
      break;
    offset = g->nextOff;
  }
}

// Walk a MEM_DATA linked list starting at base+startOff, swapping only the headers.
// nextOff is a RELATIVE offset from the current entry to the next entry.
// sectionEnd bounds how far to walk (prevents walking into adjacent sections).
static void SwapMemDataHeaders(u8* base, u32 startOff, u32 sectionEnd) {
  u32 offset = startOff;
  while (offset + 8 <= sectionEnd) {
    MEM_DATA* mem = reinterpret_cast<MEM_DATA*>(base + offset);
    mem->nextOff = SwapU32(mem->nextOff);
    mem->id = SwapU16(mem->id);
    mem->reserved = SwapU16(mem->reserved);

    if (mem->nextOff == 0xFFFFFFFF || mem->nextOff == 0)
      break;
    u32 nextOffset = offset + mem->nextOff;
    if (nextOffset <= offset || nextOffset + 8 > sectionEnd)
      break; // would go past section boundary
    offset = nextOffset;
  }
}

// Compute the end boundary for a section: the smallest offset among other sections
// that is greater than startOff, or poolLen if none.
static u32 ComputeSectionEnd(u32 startOff, const POOL_DATA* pool, u32 poolLen) {
  u32 end = poolLen;
  u32 offsets[] = {pool->macroOff, pool->curveOff, pool->keymapOff, pool->layerOff};
  for (int i = 0; i < 4; ++i) {
    if (offsets[i] > startOff && offsets[i] < end)
      end = offsets[i];
  }
  return end;
}

// Swap macro data: each entry contains MSTEP cmd[][2] = pairs of u32
// nextOff is relative from current entry to next entry; data payload is nextOff - 8 bytes.
static void SwapMacroData(u8* base, u32 startOff, u32 sectionEnd) {
  u32 offset = startOff;
  while (offset + 8 <= sectionEnd) {
    MEM_DATA* mem = reinterpret_cast<MEM_DATA*>(base + offset);
    u32 nextOff = mem->nextOff; // relative offset to next entry

    u32 dataSize;
    if (nextOff != 0xFFFFFFFF && nextOff > 8) {
      dataSize = nextOff - 8; // payload = entry size minus 8-byte header
    } else {
      dataSize = sectionEnd - offset - 8;
    }
    // Clamp to section boundary
    if (offset + 8 + dataSize > sectionEnd) {
      dataSize = sectionEnd - offset - 8;
    }

    u32* data32 = reinterpret_cast<u32*>(mem->data.tab);
    u32 numU32s = dataSize / 4;
    for (u32 i = 0; i < numU32s; ++i) {
      data32[i] = SwapU32(data32[i]);
    }

    if (nextOff == 0xFFFFFFFF || nextOff == 0)
      break;
    offset += nextOff; // relative advance
  }
}

// Swap keymap data: KEYMAP[128] per entry; swap u16 id and s16 prioOffset only
static void SwapKeymapData(u8* base, u32 startOff, u32 sectionEnd) {
  u32 offset = startOff;
  while (offset + 8 <= sectionEnd) {
    MEM_DATA* mem = reinterpret_cast<MEM_DATA*>(base + offset);

    for (int i = 0; i < 128; ++i) {
      KEYMAP* km = &mem->data.map[i];
      km->id = SwapU16(km->id);
      km->prioOffset = SwapS16(km->prioOffset);
      // transpose (s8), panning (u8), reserved (u8[2]) are single bytes — no swap
    }

    if (mem->nextOff == 0xFFFFFFFF || mem->nextOff == 0)
      break;
    offset += mem->nextOff; // relative advance
  }
}

// Swap layer data: u32 num followed by LAYER entry[] per entry
static void SwapLayerData(u8* base, u32 startOff, u32 sectionEnd) {
  u32 offset = startOff;
  while (offset + 8 <= sectionEnd) {
    MEM_DATA* mem = reinterpret_cast<MEM_DATA*>(base + offset);

    // Swap the count
    mem->data.layer.num = SwapU32(mem->data.layer.num);
    u32 num = mem->data.layer.num;

    // Swap each LAYER entry
    for (u32 i = 0; i < num; ++i) {
      LAYER* l = &mem->data.layer.entry[i];
      l->id = SwapU16(l->id);
      l->prioOffset = SwapS16(l->prioOffset);
      // keyLow, keyHigh, transpose, volume, panning, reserved are single bytes
    }

    if (mem->nextOff == 0xFFFFFFFF || mem->nextOff == 0)
      break;
    offset += mem->nextOff; // relative advance
  }
}

void CAudioGroupSet::SwapPoolEndian() {
  if (m_poolLen < sizeof(POOL_DATA)) return;

  u8* base = m_pool;

  // First, swap the POOL_DATA header (4 x u32 offsets)
  POOL_DATA* pool = reinterpret_cast<POOL_DATA*>(base);
  pool->macroOff = SwapU32(pool->macroOff);
  pool->curveOff = SwapU32(pool->curveOff);
  pool->keymapOff = SwapU32(pool->keymapOff);
  pool->layerOff = SwapU32(pool->layerOff);

  // Swap MEM_DATA headers for each sub-list.
  // Offset 0 means "no data of this type" — skip to avoid treating POOL_DATA header as MEM_DATA.
  // Also must be past the POOL_DATA header (>= sizeof(POOL_DATA)).
  auto validOff = [&](u32 off) { return off >= sizeof(POOL_DATA) && off < m_poolLen; };

  if (validOff(pool->macroOff))
    SwapMemDataHeaders(base, pool->macroOff, ComputeSectionEnd(pool->macroOff, pool, m_poolLen));
  if (validOff(pool->curveOff))
    SwapMemDataHeaders(base, pool->curveOff, ComputeSectionEnd(pool->curveOff, pool, m_poolLen));
  if (validOff(pool->keymapOff))
    SwapMemDataHeaders(base, pool->keymapOff, ComputeSectionEnd(pool->keymapOff, pool, m_poolLen));
  if (validOff(pool->layerOff))
    SwapMemDataHeaders(base, pool->layerOff, ComputeSectionEnd(pool->layerOff, pool, m_poolLen));

  // Now swap the typed data within each sub-list
  // Each swap function gets the section end (start of next section) to avoid overrun
  if (validOff(pool->macroOff))
    SwapMacroData(base, pool->macroOff, ComputeSectionEnd(pool->macroOff, pool, m_poolLen));
  // Curves are u8 arrays — no swap needed
  if (validOff(pool->keymapOff))
    SwapKeymapData(base, pool->keymapOff, ComputeSectionEnd(pool->keymapOff, pool, m_poolLen));
  if (validOff(pool->layerOff))
    SwapLayerData(base, pool->layerOff, ComputeSectionEnd(pool->layerOff, pool, m_poolLen));
}

void CAudioGroupSet::SwapSdirEndian() {
  // SDIR is an array of SDIR_DATA_INTER (32-bit version) terminated by id == 0xFFFF
  // On disk from GameCube: each entry is 0x20 bytes
  // SDIR_DATA_INTER: {u16 id, u16 ref_cnt, u32 offset, u32 addr,
  //                   SAMPLE_HEADER{u32 info, u32 length, u32 loopOffset, u32 loopLength},
  //                   u32 extraData}

  u8* base = m_sdir;
  u32 offset = 0;
  u32 entrySize = sizeof(SDIR_DATA_INTER); // 0x20

  while (offset + entrySize <= m_sdirLen) {
    SDIR_DATA_INTER* entry = reinterpret_cast<SDIR_DATA_INTER*>(base + offset);

    entry->id = SwapU16(entry->id);
    if (entry->id == 0xFFFF)
      break;

    entry->ref_cnt = SwapU16(entry->ref_cnt);
    entry->offset = SwapU32(entry->offset);
    entry->addr = SwapU32(entry->addr);
    entry->header.info = SwapU32(entry->header.info);
    entry->header.length = SwapU32(entry->header.length);
    entry->header.loopOffset = SwapU32(entry->header.loopOffset);
    entry->header.loopLength = SwapU32(entry->header.loopLength);
    entry->extraData = SwapU32(entry->extraData);

    offset += entrySize;
  }

  // Swap sample extra data. ADPCM+ samples (compType 1) carry a variable-length
  // DSPADPCMplusInfo block table that must be byte-swapped for every block entry.
  offset = 0;
  while (offset + entrySize <= m_sdirLen) {
    SDIR_DATA_INTER* entry = reinterpret_cast<SDIR_DATA_INTER*>(base + offset);
    if (entry->id == 0xFFFF)
      break;

    if (entry->extraData != 0) {
      u32 infoOffset = entry->extraData;
      if (infoOffset + sizeof(SNDADPCMinfo) <= m_sdirLen) {
        const u8 compType = static_cast<u8>(entry->header.length >> 24);
        const u32 sampleLen = entry->header.length & 0xFFFFFF;

        if (compType == 1) {
          const u32 blockCount = (sampleLen + 13) / 14;
          const u32 infoSize = static_cast<u32>(offsetof(DSPADPCMplusInfo, blk)) +
                               blockCount * static_cast<u32>(sizeof(DSPADPCMblock));
          if (infoOffset + infoSize <= m_sdirLen) {
            DSPADPCMplusInfo* info = reinterpret_cast<DSPADPCMplusInfo*>(base + infoOffset);
            info->numCoef = SwapU16(info->numCoef);
            info->loopY0 = SwapS16(info->loopY0);
            info->loopY1 = SwapS16(info->loopY1);
            for (int c = 0; c < 8; ++c) {
              info->coefTab[c][0] = SwapS16(info->coefTab[c][0]);
              info->coefTab[c][1] = SwapS16(info->coefTab[c][1]);
            }
            for (u32 block = 0; block < blockCount; ++block) {
              info->blk[block].Y0 = SwapS16(info->blk[block].Y0);
              info->blk[block].Y1 = SwapS16(info->blk[block].Y1);
            }
          }
        } else {
          SNDADPCMinfo* info = reinterpret_cast<SNDADPCMinfo*>(base + infoOffset);
          info->numCoef = SwapU16(info->numCoef);
          info->loopY0 = SwapS16(info->loopY0);
          info->loopY1 = SwapS16(info->loopY1);
          for (int c = 0; c < 8; ++c) {
            info->coefTab[c][0] = SwapS16(info->coefTab[c][0]);
            info->coefTab[c][1] = SwapS16(info->coefTab[c][1]);
          }
        }
      }
    }

    offset += entrySize;
  }
}

void CAudioGroupSet::SwapPCM16Samples() {
  // Walk SDIR entries, find PCM16 samples (compType == 2), and byte-swap their sample data
  u8* base = m_sdir;
  u32 offset = 0;
  u32 entrySize = sizeof(SDIR_DATA_INTER);

  while (offset + entrySize <= m_sdirLen) {
    SDIR_DATA_INTER* entry = reinterpret_cast<SDIR_DATA_INTER*>(base + offset);
    if (entry->id == 0xFFFF)
      break;

    u8 compType = static_cast<u8>(entry->header.length >> 24);
    u32 sampleLen = entry->header.length & 0xFFFFFF;

    if (compType == 2 && sampleLen > 0) {
      // PCM16: sample data is s16 values at m_samp + entry->offset
      u32 sampleOffset = entry->offset;
      u32 numBytes = sampleLen * 2; // length is in samples, each is 2 bytes
      if (sampleOffset + numBytes <= m_sampLen) {
        u16* samples = reinterpret_cast<u16*>(m_samp + sampleOffset);
        for (u32 i = 0; i < sampleLen; ++i) {
          samples[i] = SwapU16(samples[i]);
        }
      }
    }

    offset += entrySize;
  }
}

void CAudioGroupSet::ConvertSdirTo64Bit() {
  // Count entries
  u32 entrySize32 = sizeof(SDIR_DATA_INTER);
  u32 numEntries = 0;
  u8* base = m_sdir;
  u32 offset = 0;
  while (offset + entrySize32 <= m_sdirLen) {
    SDIR_DATA_INTER* entry = reinterpret_cast<SDIR_DATA_INTER*>(base + offset);
    ++numEntries;
    if (entry->id == 0xFFFF)
      break;
    offset += entrySize32;
  }

  if (numEntries == 0) return;

  // The original SDIR section layout:
  //   [SDIR_DATA_INTER entries (numEntries * entrySize32 bytes)]
  //   [Extra data: DSPADPCMplusInfo structs etc.]
  // extraData offsets in each entry point from the section start into the extra data region.
  // After conversion, entry sizes change, so we must append the extra data and adjust offsets.
  u32 origEntriesSize = numEntries * entrySize32;
  u32 extraDataSize = (m_sdirLen > origEntriesSize) ? (m_sdirLen - origEntriesSize) : 0;
  u32 newEntriesSize = numEntries * static_cast<u32>(sizeof(SDIR_DATA));

  // Allocate: converted entries + appended extra data
  m_convertedSdir.resize(newEntriesSize + extraDataSize);
  SDIR_DATA* sdir64 = reinterpret_cast<SDIR_DATA*>(m_convertedSdir.data());

  offset = 0;
  for (u32 i = 0; i < numEntries; ++i) {
    SDIR_DATA_INTER* src = reinterpret_cast<SDIR_DATA_INTER*>(base + offset);
    sdir64[i].id = src->id;
    sdir64[i].ref_cnt = src->ref_cnt;
    sdir64[i].offset = src->offset;
    sdir64[i].addr = reinterpret_cast<void*>(static_cast<size_t>(src->addr));
    sdir64[i].header = src->header;
    // Adjust extraData offset: it pointed into the original buffer at offset X,
    // which is now at (X - origEntriesSize + newEntriesSize) in the new buffer.
    if (src->extraData >= origEntriesSize) {
      sdir64[i].extraData = src->extraData - origEntriesSize + newEntriesSize;
    } else {
      sdir64[i].extraData = src->extraData;
    }
    offset += entrySize32;
  }

  // Copy the extra data (DSPADPCMplusInfo structs) after the converted entries
  if (extraDataSize > 0) {
    std::memcpy(m_convertedSdir.data() + newEntriesSize, base + origEntriesSize, extraDataSize);
  }

  // Point m_sdir to the converted data
  m_sdir = m_convertedSdir.data();
  m_sdirLen = static_cast<u32>(m_convertedSdir.size());
}

void CAudioGroupSet::LoadData() {
  const auto readU32 = [](const u8* ptr) {
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return SBig(value);
  };

  CMemoryInStream r(m_buffer.get(), INT32_MAX, CMemoryInStream::EOwnerShip::NotOwned);
  x10_baseName = r.Get<std::string>();
  x20_name = r.Get<std::string>();

  u8* buf = m_buffer.get() + r.GetReadPosition();

  m_poolLen = readU32(buf);
  m_pool = buf + 4;
  buf += m_poolLen + 4;

  m_projLen = readU32(buf);
  m_proj = buf + 4;
  buf += m_projLen + 4;

  m_sampLen = readU32(buf);
  m_samp = buf + 4;
  buf += m_sampLen + 4;

  m_sdirLen = readU32(buf);
  m_sdir = buf + 4;

  // Byte-swap big-endian GameCube data to native little-endian
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  SwapProjectEndian();
  SwapPoolEndian();
  SwapSdirEndian();
  SwapPCM16Samples();
#endif


  // Convert 32-bit SDIR to 64-bit on 64-bit platforms
  if (sizeof(void*) > 4) {
    ConvertSdirTo64Bit();
  }
}

CAudioGroupSet::CAudioGroupSet(std::unique_ptr<u8[]>&& in) : m_buffer(std::move(in)) {
  LoadData();
}

CFactoryFnReturn FAudioGroupSetDataFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  return TToken<CAudioGroupSet>::GetIObjObjectFor(std::make_unique<CAudioGroupSet>(std::move(in)));
}

} // namespace metaforce
