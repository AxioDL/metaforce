#include "MetroidPrime/CMemoryCard.hpp"

#include "MetroidPrime/CGameHintInfo.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "rstl/algorithm.hpp"

CMemoryCard::CMemoryCard()
: mHints(gpSimplePool->GetObj("HINT_Hints"))
, mScanCategoryCounts(0u) {
  mHints.Lock();
  mWorldInter = rs_new rstl::vector< CSaveWorldIntermediate >;
  rstl::vector< CSaveWorldIntermediate >& worlds = *mWorldInter;
  mMemoryWorlds.reserve(16);
  worlds.reserve(16);

  const rstl::vector< rstl::pair< rstl::string, SObjectTag > > resources =
      gpResourceFactory->GetResourceIdToNameList();
  for (AUTO(it, resources.begin()); it != resources.end(); ++it) {
    CAssetId worldId = it->second.GetId();
    if (gpResourceFactory->GetResourceTypeById(worldId) == 'MLVL') {
      AUTO(existing, rstl::lower_bound(
                         mMemoryWorlds.begin(), mMemoryWorlds.end(), worldId,
                         rstl::default_pair_sorter_finder<
                             rstl::vector< rstl::pair< CAssetId, CSaveWorldMemory > > >()));
      if (existing == mMemoryWorlds.end() || existing->first != worldId) {
        mMemoryWorlds.insert(existing,
                              rstl::pair< CAssetId, CSaveWorldMemory >(worldId, CSaveWorldMemory()));
        worlds.push_back(CSaveWorldIntermediate(worldId, kInvalidAssetId));
      }
    }
  }
}

CMemoryCard::~CMemoryCard() {}

CSaveWorldIntermediate::CSaveWorldIntermediate(CAssetId mlvlId, CAssetId savwId) {
  if (savwId == kInvalidAssetId) {
    mDummyWorld = rs_new CDummyWorld(mlvlId, false);
  } else {
    mSaveWorld = rs_new TCachedToken< CWorldSaveGameInfo >(
        gpSimplePool->GetObj(SObjectTag('SAVW', savwId)));
    mSaveWorld->Lock();
  }
  mMlvlId = mlvlId;
  mWorldNameId = kInvalidAssetId;
  mSaveWorldId = savwId;
}

bool CSaveWorldIntermediate::InitializePump() {
  if (!mDummyWorld.null()) {
    if (mDummyWorld->ICheckWorldComplete()) {
      CDummyWorld* dummyWorld = mDummyWorld.get();
      IWorld& world = *dummyWorld;
      mWorldNameId = dummyWorld->IGetStringTableAssetId();
      mSaveWorldId = world.IGetSaveWorldAssetId();
      int areaCount = world.IGetAreaCount();
      mAreaIds.reserve(areaCount);
      for (int i = 0; i < areaCount; ++i) {
        mAreaIds.push_back(world.IGetAreaAlways(TAreaId(i))->IGetAreaSaveId());
      }
      CWorldState& state = gpGameState->StateForWorld(world.IGetWorldAssetId());
      mDefaultLayerStates = state.GetLayerState()->GetAreaLayers();
      if (mSaveWorldId != kInvalidAssetId) {
        mSaveWorld = rs_new TCachedToken< CWorldSaveGameInfo >(
            gpSimplePool->GetObj(SObjectTag('SAVW', mSaveWorldId)));
        mSaveWorld->Lock();
      }
      mDummyWorld = nullptr;
    }
  } else {
    if (!mSaveWorld.null()) {
      if (mSaveWorld->TryCache()) {
        return true;
      }
    } else {
      return true;
    }
  }
  return false;
}

bool CMemoryCard::InitializePump() {
  if (mWorldInter.null()) {
    for (AUTO(it, mMemoryWorlds.begin()); it != mMemoryWorlds.end(); ++it) {
      if (it->second.mWorldName.valid() && !it->second.mWorldName->TryCache()) {
        return false;
      }
    }
    return mHints.TryCache();
  }

  bool done = true;
  rstl::vector< CSaveWorldIntermediate >& worlds = *mWorldInter;
  for (AUTO(it, worlds.begin()); it != worlds.end(); ++it) {
    CSaveWorldIntermediate& world = *it;
    if (world.InitializePump()) {
      if (world.mSaveWorld.null()) {
        continue;
      }
      CSaveWorldMemory& memory = const_cast< CSaveWorldMemory& >(GetSaveWorldMemory(world.mMlvlId));
      if (memory.mSaveWorldId == kInvalidAssetId) {
        memory.mSaveWorldId = world.mSaveWorldId;
      }
      if (memory.mWorldNameId == kInvalidAssetId) {
        memory.mWorldNameId = world.mWorldNameId;
      }
      memory.mAreaIds = world.mAreaIds;
      memory.mDefaultLayerStates = world.mDefaultLayerStates;
      const CWorldSaveGameInfo& saveInfo = *world.mSaveWorld->GetObject();
      memory.mAreaCount = saveInfo.GetAreaCount();
      mScanStates.reserve(mScanStates.size() + saveInfo.GetScans().size());
      for (AUTO(scan, saveInfo.GetScans().begin()); scan != saveInfo.GetScans().end(); ++scan) {
        if (rstl::find(mScanStates.begin(), mScanStates.end(), *scan) == mScanStates.end()) {
          mScanStates.push_back(*scan);
          ++mScanCategoryCounts[scan->second];
        }
      }
      memory.mSaveWorld = *world.mSaveWorld;
      world.mSaveWorld = nullptr;
      SObjectTag worldName('STRG', memory.mWorldNameId);
      if (gpResourceFactory->CanBuild(worldName)) {
        memory.mWorldName = TCachedToken< CStringTable >(gpSimplePool->GetObj(worldName));
        memory.mWorldName->Lock();
      }
    } else {
      done = false;
    }
  }
  if (done) {
    mWorldInter = nullptr;
    rstl::sort_by_key(mScanStates);
  }
  return false;
}

bool CMemoryCard::HasSaveWorldMemory(CAssetId worldId) const {
  AUTO(it, rstl::find_by_key(mMemoryWorlds, worldId));
  return it != mMemoryWorlds.end();
}

const CSaveWorldMemory& CMemoryCard::GetSaveWorldMemory(CAssetId worldId) const {
  AUTO(it, rstl::find_by_key(mMemoryWorlds, worldId));
  return it->second;
}

const wchar_t* CSaveWorldMemory::GetFrontEndName() const {
  if (mWorldName.valid() && mWorldName->GetObject() != nullptr) {
    return mWorldName->GetObject()->GetString(0);
  }
  return nullptr;
}

rstl::pair< CAssetId, int > CMemoryCard::GetAreaAndWorldIdForSaveId(int saveId) const {
  for (AUTO(it, mMemoryWorlds.begin()); it != mMemoryWorlds.end(); ++it) {
    const rstl::vector< uint >& areas = it->second.mAreaIds;
    AUTO(area, rstl::find(areas.begin(), areas.end(), static_cast< uint >(saveId)));
    if (area != areas.end()) {
      return rstl::pair< CAssetId, int >(it->first, rstl::distance(areas.begin(), area));
    }
  }
  const CAssetId invalidWorld = kInvalidAssetId;
  return rstl::pair< CAssetId, int >(invalidWorld, -1);
}
