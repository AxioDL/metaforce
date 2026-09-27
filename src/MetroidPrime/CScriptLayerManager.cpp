#include "MetroidPrime/CScriptLayerManager.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "rstl/string.hpp"

static const rstl::string skEmptyString(rstl::string::literal_t(), "");

CScriptLayerManager::CScriptLayerManager() {}

CScriptLayerManager::CScriptLayerManager(CInputStream& in, const CWorldSaveGameInfo&) {
  const uint count = in.ReadBits(10);
  mSaveLayers.reserve(count);
  for (uint i = 0; i < count; ++i) {
    mSaveLayers.push_back(in.ReadBits(1) != 0);
  }
}

void CScriptLayerManager::PutTo(COutputStream& out, const CWorldSaveGameInfo&) const {
  uint totalLayerCount = 0;
  const int areaCount = mAreaLayers.size();
  for (int i = 0; i < areaCount; ++i) {
    totalLayerCount += GetAreaLayerCount(TAreaId(i)) - 1;
  }
  out.WriteBits(totalLayerCount, 10);
  for (int i = 0; i < areaCount; ++i) {
    int layerCount = GetAreaLayerCount(TAreaId(i));
    for (int l = 1; l < layerCount; ++l) {
      out.WriteBits(IsLayerActive(TAreaId(i), TLayerId(l)) ? 1 : 0, 1);
    }
  }
}

void CScriptLayerManager::SetLayerActive(TAreaId areaIdx, TLayerId layerIdx, bool active) {
  CWorldLayers::Area& area = mAreaLayers[areaIdx.Value()];
  int layerId = layerIdx.Value();
  if (active) {
#if NONMATCHING
    area.m_layerBits |= u64(1) << layerId;
#else
    area.m_layerBits |= 1 << layerId;
#endif
  } else {
#if NONMATCHING
    area.m_layerBits &= ~(u64(1) << layerId);
#else
    area.m_layerBits &= ~(1 << layerId);
#endif
  }
}

bool CScriptLayerManager::IsLayerActive(TAreaId areaIdx, TLayerId layerIdx) const {
  const u64& layerBits = mAreaLayers[areaIdx.Value()].m_layerBits;
#if NONMATCHING
  return (layerBits & (u64(1) << layerIdx.Value())) != 0;
#else
  return (layerBits & (1 << layerIdx.Value())) != 0;
#endif
}

void CScriptLayerManager::InitializeWorldLayers(
    const rstl::vector< CWorldLayers::Area >& layers,
    const rstl::rc_ptr< rstl::vector< rstl::string > >&,
    const rstl::rc_ptr< rstl::vector< int > >&) {
  if (!mAreaLayers.empty()) {
    return;
  }

  mAreaLayers = layers;
  if (mSaveLayers.size() == 0) {
    return;
  }

  int bit = 0;
  const int areaCount = mAreaLayers.size();
  for (int i = 0; i < areaCount; ++i) {
    int layerCount = GetAreaLayerCount(TAreaId(i));
    for (int l = 1; l < layerCount; ++l) {
      SetLayerActive(TAreaId(i), TLayerId(l), mSaveLayers[bit++]);
    }
  }

  mSaveLayers = rstl::bit_vector< rstl::rmemory_allocator >();
}

int CScriptLayerManager::GetAreaLayerCount(TAreaId areaId) const {
  return mAreaLayers[areaId.Value()].m_layerCount;
}

const rstl::vector< CWorldLayers::Area >& CScriptLayerManager::GetAreaLayers() const {
  return mAreaLayers;
}
