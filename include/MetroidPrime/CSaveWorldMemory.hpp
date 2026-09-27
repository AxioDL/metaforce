#ifndef _CSAVEWORLDMEMORY
#define _CSAVEWORLDMEMORY

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CStringTable;

class CSaveWorldMemory {
  friend class CMemoryCard;

public:
  CSaveWorldMemory()
  : mWorldNameId(kInvalidAssetId)
  , mSaveWorldId(kInvalidAssetId)
  , mAreaCount(0) {}
  int GetAreaCount() const { return mAreaCount; }
  CAssetId GetWorldNameId() const { return mWorldNameId; }
  CAssetId GetSaveWorldAssetId() const { return mSaveWorldId; }
  const rstl::vector< CWorldLayers::Area >& GetDefaultLayerStates() const { return mDefaultLayerStates; }
  const wchar_t* GetFrontEndName() const;

private:
  CAssetId mWorldNameId;
  CAssetId mSaveWorldId;
  int mAreaCount;
  rstl::vector< uint > mAreaIds;
  rstl::vector< CWorldLayers::Area > mDefaultLayerStates;
  rstl::optional_object< TCachedToken< CStringTable > > mWorldName;
  rstl::optional_object< CToken > mSaveWorld;
};
CHECK_SIZEOF(CSaveWorldMemory, 0x48)

#endif // _CSAVEWORLDMEMORY
