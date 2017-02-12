#ifndef __URDE_CMEMORYCARDSYS_HPP__
#define __URDE_CMEMORYCARDSYS_HPP__

#include "CToken.hpp"
#include "World/CWorld.hpp"
#include "CGameHintInfo.hpp"
#include "CSaveWorld.hpp"
#include "GuiSys/CStringTable.hpp"
#include "kabufuda/Card.hpp"
#include <vector>

namespace urde
{
class CDummyWorld;
class CStringTable;
class CSimplePool;

class CSaveWorldMemory
{
    friend class CMemoryCardSys;
    ResId x0_strgId = -1;
    ResId x4_savwId = -1;
    u32 x8_areaCount;
    std::vector<ResId> xc_areaIds;
    std::vector<CWorldLayers::Area> x1c_defaultLayerStates;
    TLockedToken<CStringTable> x2c_worldName; /* used to be optional */
    TLockedToken<CSaveWorld> x3c_saveWorld; /* used to be optional */

public:
    ResId GetWorldNameId() const { return x0_strgId; }
    ResId GetSaveWorldAssetId() const { return x4_savwId; }
    u32 GetAreaCount() const { return x8_areaCount; }
    const std::vector<CWorldLayers::Area>& GetDefaultLayerStates() const { return x1c_defaultLayerStates; }
    const TLockedToken<CStringTable>& GetWorldName() const { return x2c_worldName; }
    const TLockedToken<CSaveWorld>& GetSaveWorld() const { return x3c_saveWorld; }
    const char16_t* GetFrontEndName() const
    {
        if (!x2c_worldName)
            return u"";
        return x2c_worldName->GetString(0);
    }
};

class CSaveWorldIntermediate
{
    friend class CMemoryCardSys;
    ResId x0_mlvlId;
    ResId x4_strgId;
    ResId x8_savwId;
    std::vector<ResId> xc_areaIds;
    std::vector<CWorldLayers::Area> x1c_defaultLayerStates;
    std::unique_ptr<CDummyWorld> x2c_dummyWorld;
    TLockedToken<CSaveWorld> x34_saveWorld; /* Used to be auto_ptr */

public:
    CSaveWorldIntermediate(ResId mlvl, ResId savw);

    bool InitializePump();
};

class CMemoryCardSys
{
    TLockedToken<CGameHintInfo> x0_hints;
    std::vector<std::pair<ResId, CSaveWorldMemory>> xc_memoryWorlds; /* MLVL as key */
    std::experimental::optional<std::vector<CSaveWorldIntermediate>> x1c_worldInter; /* used to be auto_ptr of vector */
    std::vector<std::pair<ResId, CSaveWorld::EScanCategory>> x20_scanStates;

public:
    static kabufuda::SystemString ResolveDolphinCardPath(kabufuda::ECardSlot slot);
    static kabufuda::SystemString CreateDolphinCard(kabufuda::ECardSlot slot);
    static kabufuda::SystemString _CreateDolphinCard(kabufuda::ECardSlot slot);

    using ECardResult = kabufuda::ECardResult;
    struct CardResult
    {
        ECardResult result;
        CardResult(ECardResult res) : result(res) {}
        operator ECardResult() const { return result; }
        operator bool() const { return result != ECardResult::READY; }
    };

    struct CardFileHandle
    {
        kabufuda::ECardSlot slot;
        kabufuda::FileHandle handle;
        CardFileHandle(kabufuda::ECardSlot slot) : slot(slot) {}
        int getFileNo() const { return handle.getFileNo(); }
    };

    using CardStat = kabufuda::CardStat;
    const std::vector<CGameHintInfo::CGameHint>& GetHints() const { return x0_hints->GetHints(); }
    const std::vector<std::pair<ResId, CSaveWorldMemory>>& GetMemoryWorlds() const { return xc_memoryWorlds; }
    const std::vector<std::pair<ResId, CSaveWorld::EScanCategory>>& GetScanStates() const { return x20_scanStates; }

    bool HasSaveWorldMemory(ResId wldId) const;
    const CSaveWorldMemory& GetSaveWorldMemory(ResId wldId) const;

    CMemoryCardSys();
    bool InitializePump();

    struct CCardFileInfo
    {
        struct Icon
        {
            ResId x0_id;
            kabufuda::EAnimationSpeed x4_speed;
            TLockedToken<CTexture> x8_tex;
            Icon(ResId id, kabufuda::EAnimationSpeed speed, CSimplePool& sp, const CVParamTransfer& cv);
        };

        enum class EStatus
        {
            Standby,
            Transferring,
            Done
        };

        EStatus x0_status = EStatus::Standby;
        //CARDFileInfo x4_info;
        CardFileHandle m_handle;
        std::string x18_fileName;
        std::string x28_comment;
        ResId x3c_bannerTex = -1;
        std::experimental::optional<TLockedToken<CTexture>> x40_bannerTok;
        rstl::reserved_vector<Icon, 8> x50_iconToks;
        std::vector<u8> xf4_saveBuffer;
        std::vector<u8> x104_cardBuffer;

        CVParamTransfer m_texParam = {new TObjOwnerParam<u32>(SBIG('OTEX'))};

        CCardFileInfo(kabufuda::ECardSlot port, const std::string& name)
        : m_handle(port), x18_fileName(name) {}

        void LockBannerToken(ResId bannerTxtr, CSimplePool& sp);
        void LockIconToken(ResId iconTxtr, kabufuda::EAnimationSpeed speed, CSimplePool& sp);

        kabufuda::ECardSlot GetCardPort() const { return m_handle.slot; }
        int GetFileNo() const { return m_handle.getFileNo(); }
        u32 CalculateBannerDataSize() const;
        u32 CalculateTotalDataSize() const;
        void BuildCardBuffer();
        void WriteBannerData(CMemoryOutStream& out) const;
        void WriteIconData(CMemoryOutStream& out) const;
        void SetComment(const std::string& c) { x28_comment = c; }
        ECardResult PumpCardTransfer();
        ECardResult GetStatus(CardStat& stat) const;
        ECardResult CreateFile();
        ECardResult WriteFile();
        ECardResult CloseFile();

        CMemoryOutStream BeginMemoryOut(u32 sz)
        {
            xf4_saveBuffer.resize(sz);
            return CMemoryOutStream(xf4_saveBuffer.data(), sz);
        }
    };

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

}

#endif // __URDE_CMEMORYCARDSYS_HPP__
