#ifndef _CSCRIPTLAYERMANAGER
#define _CSCRIPTLAYERMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/vector.hpp"
#include "rstl/bit_vector.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CInputStream;
class CWorldSaveGameInfo;
class COutputStream;

class CWorldLayers {
public:
  class Area {
  public:
    explicit Area(CInputStream& in)
    : m_layerCount(in.Get< int >()), m_layerBits(in.ReadLongLong()) {}
    Area(int layerCount, u64 layerBits) : m_layerCount(layerCount), m_layerBits(layerBits) {}

    int m_layerCount;
    u64 m_layerBits;
  };
  static void ReadWorldLayers(CInputStream& in, int version, CAssetId mlvlId);
};
NESTED_CHECK_SIZEOF(CWorldLayers, Area, 0x10)

struct TLayerId {
  int value;
  TLayerId(int value) : value(value) {}
  int Value() const { return value; }
};
CHECK_SIZEOF(TLayerId, 4)

class CScriptLayerManager {
public:
  CScriptLayerManager();
  CScriptLayerManager(CInputStream&, const CWorldSaveGameInfo&);

  void PutTo(COutputStream&, const CWorldSaveGameInfo&) const;

  void SetLayerActive(TAreaId areaIdx, TLayerId layerIdx, bool active);
  bool IsLayerActive(TAreaId areaIdx, TLayerId layerIdx) const;

  const rstl::vector< CWorldLayers::Area >& GetAreaLayers() const;

  void InitializeWorldLayers(const rstl::vector< CWorldLayers::Area >&,
                             const rstl::rc_ptr< rstl::vector< rstl::string > >&,
                             const rstl::rc_ptr< rstl::vector< int > >&);
  int GetAreaLayerCount(TAreaId) const;

private:
  rstl::vector< CWorldLayers::Area > mAreaLayers;
  rstl::bit_vector< rstl::rmemory_allocator > mSaveLayers;
};

CHECK_SIZEOF(CScriptLayerManager, 0x24)

#endif // _CSCRIPTLAYERMANAGER
