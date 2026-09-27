#ifndef _CMEMORYCARD
#define _CMEMORYCARD

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/CGameHintInfo.hpp"
#include "MetroidPrime/CSaveWorldMemory.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CDummyWorld;
class CStringTable;

class CSaveWorldIntermediate {
  friend class CMemoryCard;

public:
  CSaveWorldIntermediate(CAssetId mlvlId, CAssetId savwId);
  bool InitializePump();

private:
  CAssetId mMlvlId;
  CAssetId mWorldNameId;
  CAssetId mSaveWorldId;
  rstl::vector< uint > mAreaIds;
  rstl::vector< CWorldLayers::Area > mDefaultLayerStates;
  rstl::auto_ptr< CDummyWorld > mDummyWorld;
  rstl::auto_ptr< TCachedToken< CWorldSaveGameInfo > > mSaveWorld;
};
CHECK_SIZEOF(CSaveWorldIntermediate, 0x3c)

class CMemoryCard {
public:
  CMemoryCard();
  ~CMemoryCard();
  bool InitializePump();
  bool HasSaveWorldMemory(CAssetId worldId) const;
  const CSaveWorldMemory& GetSaveWorldMemory(CAssetId worldId) const;
  rstl::pair< CAssetId, int > GetAreaAndWorldIdForSaveId(int saveId) const;

  const rstl::vector< CGameHintInfo::CGameHint >& GetHints() const {
    return mHints.GetObject()->GetHints();
  }

  typedef rstl::pair< CAssetId, CSaveWorldMemory > MemoryWorld;
  const rstl::vector< MemoryWorld >& GetMemoryWorlds() const { return mMemoryWorlds; }

  typedef rstl::pair< CAssetId, uint > ScanState;
  const rstl::vector< ScanState >& GetScanStates() const { return mScanStates; }
  uint GetScanCategoryCount(CWorldSaveGameInfo::EScanCategory category) const {
    return mScanCategoryCounts[category];
  }

private:
  TCachedToken< CGameHintInfo > mHints;
  rstl::vector< rstl::pair< CAssetId, CSaveWorldMemory > > mMemoryWorlds;
  rstl::single_ptr< rstl::vector< CSaveWorldIntermediate > > mWorldInter;
  rstl::vector< ScanState > mScanStates;
  rstl::reserved_vector< uint, 6 > mScanCategoryCounts;
};
CHECK_SIZEOF(CMemoryCard, 0x4c)

extern CMemoryCard* gpMemoryCard;

#endif // _CMEMORYCARD
