#ifndef __URDE_CMEMORYCARDSYS_HPP__
#define __URDE_CMEMORYCARDSYS_HPP__

#include "CToken.hpp"
#include "World/CWorld.hpp"
#include "CGameHintInfo.hpp"
#include "CSaveWorld.hpp"
#include "GuiSys/CStringTable.hpp"
#include <vector>

// longest file name string excluding terminating zero
#define CARD_FILENAME_MAX 32

#define CARD_ICON_MAX 8

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
    ResId GetSaveWorldAssetId() const { return x4_savwId; }
    u32 GetAreaCount() const { return x8_areaCount; }

    const TLockedToken<CStringTable>& GetWorldName() const { return x2c_worldName; }
    const TLockedToken<CSaveWorld>& GetSaveWorld() const { return x3c_saveWorld; }
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
    const std::vector<CGameHintInfo::CGameHint>& GetHints() const { return x0_hints->GetHints(); }
    const std::vector<std::pair<ResId, CSaveWorldMemory>>& GetMemoryWorlds() const { return xc_memoryWorlds; }
    const std::vector<std::pair<ResId, CSaveWorld::EScanCategory>>& GetScanStates() const { return x20_scanStates; }
    CMemoryCardSys();
    bool InitializePump();

    enum class EMemoryCardPort
    {
        SlotA,
        SlotB
    };

    enum class ECardResult
    {
        CARD_RESULT_CRC_MISMATCH = -1003,
        CARD_RESULT_FATAL_ERROR = -128,
        CARD_RESULT_ENCODING = -13,
        CARD_RESULT_BROKEN = -6,
        CARD_RESULT_IOERROR = -5,
        CARD_RESULT_NOFILE = -4,
        CARD_RESULT_NOCARD = -3,
        CARD_RESULT_WRONGDEVICE = -2,
        CARD_RESULT_BUSY = -1,
        CARD_RESULT_READY = 0
    };

    struct CardProbeResults
    {
        ECardResult x0_error;
        u32 x4_cardSize; // in megabits
        u32 x8_sectorSize; // in bytes
    };

    struct CARDStat
    {
        // read-only (Set by CARDGetStatus)
        char x0_fileName[CARD_FILENAME_MAX];
        u32  x20_length;
        u32  x24_time;           // seconds since 01/01/2000 midnight
        u8   x28_gameName[4];
        u8   x2c_company[2];

        // read/write (Set by CARDGetStatus/CARDSetStatus)
        u8   x2e_bannerFormat;
        u8   x2f___padding;
        u32  x30_iconAddr;      // offset to the banner, bannerTlut, icon, iconTlut data set.
        u16  x34_iconFormat;
        u16  x36_iconSpeed;
        u32  x38_commentAddr;   // offset to the pair of 32 byte character strings.

        // read-only (Set by CARDGetStatus)
        u32  x3c_offsetBanner;
        u32  x40_offsetBannerTlut;
        u32  x44_offsetIcon[CARD_ICON_MAX];
        u32  x64_offsetIconTlut;
        u32  x68_offsetData;

        u32 GetFileLength() const { return x20_length; }
        u32 GetTime() const { return x24_time; }
        u32 GetBannerFormat() const { return x2e_bannerFormat & 0x3; }
        void SetBannerFormat(u32 fmt) { x2e_bannerFormat = (x2e_bannerFormat & ~0x3) | fmt; }
        u32 GetIconFormat(int idx) const { return (x34_iconFormat >> (idx * 2)) & 0x3; }
        void SetIconFormat(u32 fmt, int idx)
        {
            x34_iconFormat &= ~(0x3 << (idx * 2));
            x34_iconFormat |= fmt << (idx * 2);
        }
        void SetIconSpeed(u32 sp, int idx)
        {
            x36_iconSpeed &= ~(0x3 << (idx * 2));
            x36_iconSpeed |= sp << (idx * 2);
        }
        u32 GetIconAddr() const { return x30_iconAddr; }
        void SetIconAddr(u32 addr) { x30_iconAddr = addr; }
        u32 GetCommentAddr() const { return x38_commentAddr; }
        void SetCommentAddr(u32 addr) { x38_commentAddr = addr; }
    };

    struct CARDFileInfo
    {
        EMemoryCardPort chan;
        s32 fileNo = -1;

        s32 offset;
        s32 length;
        u16 iBlock;
        u16 __padding;

        CARDFileInfo(EMemoryCardPort port) : chan(port) {}
    };

    struct CCardFileInfo
    {
        struct Icon
        {
            ResId x0_id;
            u32 x4_speed;
            TLockedToken<CTexture> x8_tex;
            Icon(ResId id, u32 speed, CSimplePool& sp, const CVParamTransfer& cv);
        };

        enum class EStatus
        {
            Standby,
            Transferring,
            Done
        };

        EStatus x0_status = EStatus::Standby;
        CARDFileInfo x4_info;
        std::string x18_name;
        std::string x28_name2;
        ResId x3c_bannerTex = -1;
        std::experimental::optional<TLockedToken<CTexture>> x40_bannerTok;
        rstl::reserved_vector<Icon, 8> x50_iconToks;
        std::vector<u8> xf4_saveBuffer;
        std::vector<u8> x104_cardBuffer;

        CVParamTransfer m_texParam = {new TObjOwnerParam<u32>(SBIG('OTEX'))};

        CCardFileInfo(EMemoryCardPort port, const std::string& name)
        : x4_info(port), x18_name(name) {}

        void LockBannerToken(ResId bannerTxtr, CSimplePool& sp);
        void LockIconToken(ResId iconTxtr, u32 speed, CSimplePool& sp);

        EMemoryCardPort GetCardPort() const { return x4_info.chan; }
        int GetFileNo() const { return x4_info.fileNo; }
        u32 CalculateBannerDataSize() const;
        u32 CalculateTotalDataSize() const;
        void BuildCardBuffer();
        void WriteBannerData(CMemoryOutStream& out) const;
        void WriteIconData(CMemoryOutStream& out) const;
        ECardResult PumpCardTransfer();
        ECardResult GetStatus(CARDStat& stat) const;
        ECardResult CreateFile();
        ECardResult Write();
        ECardResult Close();
    };

    static CardProbeResults CardProbe(EMemoryCardPort port);
    static ECardResult MountCard(EMemoryCardPort port);
    static ECardResult CheckCard(EMemoryCardPort port);
    static ECardResult GetNumFreeBytes(EMemoryCardPort port, s32& freeBytes, s32& freeFiles);
    static ECardResult GetSerialNo(EMemoryCardPort port, u64& serialOut);
    static ECardResult GetResultCode(EMemoryCardPort port);
    static ECardResult GetStatus(EMemoryCardPort port, int fileNo, CARDStat& statOut);
    static ECardResult SetStatus(EMemoryCardPort port, int fileNo, const CARDStat& stat);
    static ECardResult DeleteFile(EMemoryCardPort port, const char* name);
    static ECardResult FastDeleteFile(EMemoryCardPort port, int fileNo);
    static ECardResult Rename(EMemoryCardPort port, const char* oldName, const char* newName);
    static ECardResult FormatCard(EMemoryCardPort port);
};

}

#endif // __URDE_CMEMORYCARDSYS_HPP__
