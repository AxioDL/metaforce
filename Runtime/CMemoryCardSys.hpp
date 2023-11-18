#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/Streams/CMemoryStreamOut.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/CWorldSaveGameInfo.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/rstl.hpp"

#include <kabufuda/Card.hpp>

namespace metaforce {
class CDummyWorld;
class CSimplePool;
class CStringTable;

class CSaveWorldMemory {
  friend class CMemoryCardSys;
  CAssetId x0_strgId;
  CAssetId x4_savwId;
  u32 x8_areaCount;
  std::vector<s32> xc_areaIds;
  std::vector<CWorldLayers::Area> x1c_defaultLayerStates;
  TLockedToken<CStringTable> x2c_worldName;       /* used to be optional */
  TLockedToken<CWorldSaveGameInfo> x3c_saveWorld; /* used to be optional */

public:
  CAssetId GetWorldNameId() const { return x0_strgId; }
  CAssetId GetSaveWorldAssetId() const { return x4_savwId; }
  u32 GetAreaCount() const { return x8_areaCount; }
  const std::vector<CWorldLayers::Area>& GetDefaultLayerStates() const { return x1c_defaultLayerStates; }
  const TLockedToken<CStringTable>& GetWorldName() const { return x2c_worldName; }
  const TLockedToken<CWorldSaveGameInfo>& GetSaveWorld() const { return x3c_saveWorld; }
  const char16_t* GetFrontEndName() const {
    if (!x2c_worldName)
      return u"";
    return x2c_worldName->GetString(0);
  }
};

class CSaveWorldIntermediate {
  friend class CMemoryCardSys;
  CAssetId x0_mlvlId;
  CAssetId x4_strgId;
  CAssetId x8_savwId;
  std::vector<s32> xc_areaIds;
  std::vector<CWorldLayers::Area> x1c_defaultLayerStates;
  std::unique_ptr<CDummyWorld> x2c_dummyWorld;
  TLockedToken<CWorldSaveGameInfo> x34_saveWorld; /* Used to be auto_ptr */

public:
  CSaveWorldIntermediate(CAssetId mlvl, CAssetId savw);

  bool InitializePump();
};

class CMemoryCardSys {
  TLockedToken<CGameHintInfo> x0_hints;
  std::vector<std::pair<CAssetId, CSaveWorldMemory>> xc_memoryWorlds; /* MLVL as key */
  std::optional<std::vector<CSaveWorldIntermediate>> x1c_worldInter;  /* used to be auto_ptr of vector */
  std::vector<std::pair<CAssetId, CWorldSaveGameInfo::EScanCategory>> x20_scanStates;
  rstl::reserved_vector<u32, 6> x30_scanCategoryCounts;

public:
  static void _ResetCVar(kabufuda::ECardSlot slot);
  static void _ResolveDolphinCardPath(const CVar* cv, kabufuda::ECardSlot slot);
  static std::string ResolveDolphinCardPath(kabufuda::ECardSlot slot);
  static bool CreateDolphinCard(kabufuda::ECardSlot slot);
  static std::string _GetDolphinCardPath(kabufuda::ECardSlot slot);
  static std::string _CreateDolphinCard(kabufuda::ECardSlot slot, bool dolphin);

  using ECardResult = kabufuda::ECardResult;
  struct CardResult {
    ECardResult result;
    CardResult(ECardResult res) : result(res) {}
    operator ECardResult() const { return result; }
    explicit operator bool() const { return result != ECardResult::READY; }
  };

  struct CardFileHandle {
    kabufuda::ECardSlot slot;
    kabufuda::FileHandle handle;
    CardFileHandle(kabufuda::ECardSlot slot) : slot(slot) {}
    int getFileNo() const { return handle.getFileNo(); }
  };

  using CardStat = kabufuda::CardStat;
  const std::vector<CGameHintInfo::CGameHint>& GetHints() const { return x0_hints->GetHints(); }
  const std::vector<std::pair<CAssetId, CSaveWorldMemory>>& GetMemoryWorlds() const { return xc_memoryWorlds; }
  const std::vector<std::pair<CAssetId, CWorldSaveGameInfo::EScanCategory>>& GetScanStates() const {
    return x20_scanStates;
  }
  u32 GetScanCategoryCount(CWorldSaveGameInfo::EScanCategory cat) const { return x30_scanCategoryCounts[int(cat)]; }

  std::vector<std::pair<CAssetId, CWorldSaveGameInfo::EScanCategory>>::const_iterator
  LookupScanState(CAssetId id) const {
    return rstl::binary_find(x20_scanStates.cbegin(), x20_scanStates.cend(), id,
                             [](const std::pair<CAssetId, CWorldSaveGameInfo::EScanCategory>& p) { return p.first; });
  }

  bool HasSaveWorldMemory(CAssetId wldId) const;
  const CSaveWorldMemory& GetSaveWorldMemory(CAssetId wldId) const;

  CMemoryCardSys();
  bool InitializePump();

  struct CCardFileInfo {
    struct Icon {
      CAssetId x0_id;
      kabufuda::EAnimationSpeed x4_speed;
      TLockedToken<CTexture> x8_tex;
      Icon(CAssetId id, kabufuda::EAnimationSpeed speed, CSimplePool& sp);
    };

    enum class EStatus { Standby, Transferring, Done };

    EStatus x0_status = EStatus::Standby;
    // CARDFileInfo x4_info;
    CardFileHandle m_handle;
    std::string x18_fileName;
    std::string x28_comment;
    CAssetId x3c_bannerTex;
    std::optional<TLockedToken<CTexture>> x40_bannerTok;
    rstl::reserved_vector<Icon, 8> x50_iconToks;
    std::vector<u8> xf4_saveBuffer;
    std::vector<u8> x104_cardBuffer;

    CCardFileInfo(kabufuda::ECardSlot port, std::string_view name) : m_handle(port), x18_fileName(name) {}

    void LockBannerToken(CAssetId bannerTxtr, CSimplePool& sp);
    void LockIconToken(CAssetId iconTxtr, kabufuda::EAnimationSpeed speed, CSimplePool& sp);

    [[nodiscard]] kabufuda::ECardSlot GetCardPort() const { return m_handle.slot; }
    [[nodiscard]] int GetFileNo() const { return m_handle.getFileNo(); }
    [[nodiscard]] u32 CalculateBannerDataSize() const;
    [[nodiscard]] u32 CalculateTotalDataSize() const;
    void BuildCardBuffer();
    void WriteBannerData(COutputStream& out) const;
    void WriteIconData(COutputStream& out) const;
    void SetComment(const std::string& c) { x28_comment = c; }
    ECardResult PumpCardTransfer();
    ECardResult GetStatus(CardStat& stat) const;
    ECardResult CreateFile();
    ECardResult WriteFile();
    ECardResult CloseFile();

    CMemoryStreamOut BeginMemoryOut(u32 sz) {
      xf4_saveBuffer.resize(sz);
      return CMemoryStreamOut(xf4_saveBuffer.data(), sz, CMemoryStreamOut::EOwnerShip::NotOwned, sz);
    }
  };

  std::pair<CAssetId, TAreaId> GetAreaAndWorldIdForSaveId(s32 saveId) const;
  static kabufuda::ProbeResults CardProbe(kabufuda::ECardSlot port);
  static ECardResult MountCard(kabufuda::ECardSlot port);
  static ECardResult UnmountCard(kabufuda::ECardSlot port);
  static ECardResult CheckCard(kabufuda::ECardSlot port);
  static ECardResult CreateFile(kabufuda::ECardSlot port, const char* name, u32 size, CardFileHandle& info);
  static ECardResult OpenFile(kabufuda::ECardSlot port, const char* name, CardFileHandle& info);
  static ECardResult FastOpenFile(kabufuda::ECardSlot port, int fileNo, CardFileHandle& info);
  static ECardResult CloseFile(CardFileHandle& info);
  static ECardResult ReadFile(CardFileHandle& info, void* buf, s32 length, s32 offset);
  static ECardResult WriteFile(CardFileHandle& info, const void* buf, s32 length, s32 offset);
  static ECardResult GetNumFreeBytes(kabufuda::ECardSlot port, s32& freeBytes, s32& freeFiles);
  static ECardResult GetSerialNo(kabufuda::ECardSlot port, u64& serialOut);
  static ECardResult GetResultCode(kabufuda::ECardSlot port);
  static ECardResult GetStatus(kabufuda::ECardSlot port, int fileNo, CardStat& statOut);
  static ECardResult SetStatus(kabufuda::ECardSlot port, int fileNo, const CardStat& stat);
  static ECardResult DeleteFile(kabufuda::ECardSlot port, const char* name);
  static ECardResult FastDeleteFile(kabufuda::ECardSlot port, int fileNo);
  static ECardResult Rename(kabufuda::ECardSlot port, const char* oldName, const char* newName);
  static ECardResult FormatCard(kabufuda::ECardSlot port);

  static void CommitToDisk(kabufuda::ECardSlot port);
  static void Shutdown();
};

} // namespace metaforce
