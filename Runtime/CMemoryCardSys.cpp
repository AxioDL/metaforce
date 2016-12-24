#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CGameState.hpp"
#include "GuiSys/CStringTable.hpp"
#include "CCRC32.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

CSaveWorldIntermediate::CSaveWorldIntermediate(ResId mlvl, ResId savw)
: x0_mlvlId(mlvl), x8_savwId(savw)
{
    if (savw == -1)
        x2c_dummyWorld = std::make_unique<CDummyWorld>(mlvl, false);
    else
        x34_saveWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), savw});
}

bool CSaveWorldIntermediate::InitializePump()
{
    if (x2c_dummyWorld)
    {
        CDummyWorld& wld = *x2c_dummyWorld;
        if (!wld.ICheckWorldComplete())
            return false;

        x4_strgId = wld.IGetStringTableAssetId();
        x8_savwId = wld.IGetSaveWorldAssetId();
        u32 areaCount = wld.IGetAreaCount();

        xc_areaIds.reserve(areaCount);
        for (u32 i=0 ; i<areaCount ; ++i)
        {
            const IGameArea* area = wld.IGetAreaAlways(i);
            xc_areaIds.push_back(area->IGetAreaId());
        }

        ResId mlvlId = wld.IGetWorldAssetId();
        CWorldState& mlvlState = g_GameState->StateForWorld(mlvlId);
        x1c_defaultLayerStates = mlvlState.GetLayerState()->x0_areaLayers;

        x34_saveWorld = g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), x8_savwId});
        x2c_dummyWorld.reset();
    }
    else
    {
        if (!x34_saveWorld)
            return true;
        if (x34_saveWorld.IsLoaded() && x34_saveWorld.GetObj())
            return true;
    }

    return false;
}

CMemoryCardSys::CMemoryCardSys()
{
    x0_hints = g_SimplePool->GetObj("HINT_Hints");
    xc_memoryWorlds.reserve(16);
    x1c_worldInter.emplace();
    x1c_worldInter->reserve(16);
    g_ResFactory->EnumerateNamedResources([&](const std::string& name, const SObjectTag& tag) -> bool
    {
        if (tag.type == FOURCC('MLVL'))
        {
            auto existingSearch =
                std::find_if(xc_memoryWorlds.cbegin(), xc_memoryWorlds.cend(), [&](const auto& wld)
                { return wld.first == tag.id; });
            if (existingSearch == xc_memoryWorlds.cend())
            {
                xc_memoryWorlds.emplace_back(tag.id, CSaveWorldMemory{});
                x1c_worldInter->emplace_back(tag.id, -1);
            }
        }
        return true;
    });
}

bool CMemoryCardSys::InitializePump()
{
    if (!x1c_worldInter)
    {
        for (const auto& world : xc_memoryWorlds)
        {
            const CSaveWorldMemory& wld = world.second;
            if (!wld.GetWorldName())
                continue;
            if (!wld.GetWorldName().IsLoaded() ||
                !wld.GetWorldName().GetObj())
                return false;
        }

        if (!x0_hints.IsLoaded() || !x0_hints.GetObj())
            return false;
        return true;
    }

    bool done = true;
    for (CSaveWorldIntermediate& world : *x1c_worldInter)
    {
        if (world.InitializePump())
        {
            if (!world.x34_saveWorld)
                continue;

            auto existingSearch =
                std::find_if(xc_memoryWorlds.begin(), xc_memoryWorlds.end(), [&](const auto& test)
                { return test.first == world.x0_mlvlId; });
            CSaveWorldMemory& wldMemOut = existingSearch->second;
            wldMemOut.x4_savwId = world.x8_savwId;
            wldMemOut.x0_strgId = world.x4_strgId;
            wldMemOut.xc_areaIds = world.xc_areaIds;
            wldMemOut.x1c_defaultLayerStates = world.x1c_defaultLayerStates;

            CSaveWorld& savw = *world.x34_saveWorld;
            wldMemOut.x8_areaCount = savw.GetAreaCount();

            x20_scanStates.reserve(x20_scanStates.size() + savw.GetScans().size());
            for (const CSaveWorld::SScanState& scan : savw.GetScans())
                x20_scanStates.emplace_back(scan.x0_id, scan.x4_category);

            wldMemOut.x3c_saveWorld = std::move(world.x34_saveWorld);
            wldMemOut.x2c_worldName = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), wldMemOut.x0_strgId});
        }
        else
            done = false;
    }

    if (done)
        x1c_worldInter = std::experimental::nullopt;

    return false;
}

void CMemoryCardSys::CCardFileInfo::LockBannerToken(ResId bannerTxtr, CSimplePool& sp)
{
    x3c_bannerTex = bannerTxtr;
    x40_bannerTok.emplace(sp.GetObj({FOURCC('TXTR'), bannerTxtr}, m_texParam));
}

CMemoryCardSys::CCardFileInfo::Icon::Icon(ResId id, u32 speed, CSimplePool& sp, const CVParamTransfer& cv)
: x0_id(id), x4_speed(speed), x8_tex(sp.GetObj({FOURCC('TXTR'), id}, cv)) {}

void CMemoryCardSys::CCardFileInfo::LockIconToken(ResId iconTxtr, u32 speed, CSimplePool& sp)
{
    x50_iconToks.emplace_back(iconTxtr, speed, sp, m_texParam);
}

u32 CMemoryCardSys::CCardFileInfo::CalculateBannerDataSize() const
{
    u32 ret = 68;
    if (x3c_bannerTex != -1)
    {
        if ((*x40_bannerTok)->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3)
            ret = 6212;
        else
            ret = 3652;
    }

    bool paletteTex = false;
    for (const Icon& icon : x50_iconToks)
    {
        if (icon.x8_tex->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3)
            ret += 2048;
        else
        {
            ret += 1024;
            paletteTex = true;
        }
    }

    if (paletteTex)
        ret += 512;

    return ret;
}

u32 CMemoryCardSys::CCardFileInfo::CalculateTotalDataSize() const
{
    return (CalculateBannerDataSize() + xf4_saveBuffer.size() + 8191) & ~8191;
}

void CMemoryCardSys::CCardFileInfo::BuildCardBuffer()
{
    u32 bannerSz = CalculateBannerDataSize();
    x104_cardBuffer.resize((bannerSz + xf4_saveBuffer.size() + 8191) & ~8191);

    CMemoryOutStream w(x104_cardBuffer.data(), x104_cardBuffer.size());
    w.writeUint32Big(0);
    char comment[64];
    strncpy(comment, x28_comment.data(), 64);
    w.writeBytes(comment, 64);
    WriteBannerData(w);
    WriteIconData(w);
    memmove(x104_cardBuffer.data() + bannerSz, xf4_saveBuffer.data(), xf4_saveBuffer.size());
    reinterpret_cast<u32&>(*x104_cardBuffer.data()) =
        hecl::SBig(CCRC32::Calculate(x104_cardBuffer.data() + 4, x104_cardBuffer.size() - 4));

    xf4_saveBuffer.clear();
}

void CMemoryCardSys::CCardFileInfo::WriteBannerData(CMemoryOutStream& out) const
{
    if (x3c_bannerTex != -1)
    {
        const TLockedToken<CTexture>& tex = *x40_bannerTok;
        u32 bufSz;
        ETexelFormat fmt;
        std::unique_ptr<u8[]> palette;
        std::unique_ptr<u8[]> texels = tex->BuildMemoryCardTex(bufSz, fmt, palette);

        if (fmt == ETexelFormat::RGB5A3)
            out.writeBytes(texels.get(), 6144);
        else
            out.writeBytes(texels.get(), 3072);

        if (fmt == ETexelFormat::C8)
            out.writeBytes(palette.get(), 512);
    }
}

void CMemoryCardSys::CCardFileInfo::WriteIconData(CMemoryOutStream& out) const
{
    std::unique_ptr<u8[]> palette;
    for (const Icon& icon : x50_iconToks)
    {
        u32 bufSz;
        ETexelFormat fmt;
        std::unique_ptr<u8[]> texels = icon.x8_tex->BuildMemoryCardTex(bufSz, fmt, palette);

        if (fmt == ETexelFormat::RGB5A3)
            out.writeBytes(texels.get(), 2048);
        else
            out.writeBytes(texels.get(), 1024);
    }
    if (palette)
        out.writeBytes(palette.get(), 512);
}

CMemoryCardSys::ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer()
{
    if (x0_status == EStatus::Standby)
        return ECardResult::CARD_RESULT_READY;
    else if (x0_status == EStatus::Transferring)
    {
        ECardResult result = CMemoryCardSys::GetResultCode(GetCardPort());
        if (result != ECardResult::CARD_RESULT_BUSY)
            x104_cardBuffer.clear();
        if (result != ECardResult::CARD_RESULT_READY)
            return result;
        x0_status = EStatus::Done;
        CARDStat stat = {};
        result = GetStatus(stat);
        if (result != ECardResult::CARD_RESULT_READY)
            return result;
        result = CMemoryCardSys::SetStatus(GetCardPort(), GetFileNo(), stat);
        if (result != ECardResult::CARD_RESULT_READY)
            return result;
        return ECardResult::CARD_RESULT_BUSY;
    }
    else
    {
        ECardResult result = CMemoryCardSys::GetResultCode(GetCardPort());
        if (result == ECardResult::CARD_RESULT_READY)
            x0_status = EStatus::Standby;
        return result;
    }
}

CMemoryCardSys::ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(CARDStat& stat) const
{
    ECardResult result = CMemoryCardSys::GetStatus(GetCardPort(), GetFileNo(), stat);
    if (result != ECardResult::CARD_RESULT_READY)
        return result;

    stat.SetCommentAddr(4);
    stat.SetIconAddr(68);

    u32 bannerFmt;
    if (x3c_bannerTex != -1)
    {
        if ((*x40_bannerTok)->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3)
            bannerFmt = 2;
        else
            bannerFmt = 1;
    }
    else
        bannerFmt = 0;
    stat.SetBannerFormat(bannerFmt);

    int idx = 0;
    for (const Icon& icon : x50_iconToks)
    {
        stat.SetIconFormat(icon.x8_tex->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3 ? 2 : 1, idx);
        stat.SetIconSpeed(icon.x4_speed, idx);
        ++idx;
    }
    if (idx < 8)
    {
        stat.SetIconFormat(0, idx);
        stat.SetIconSpeed(0, idx);
    }

    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::CCardFileInfo::CreateFile()
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::CCardFileInfo::Write()
{
    BuildCardBuffer();
    //DCStoreRange(info.x104_cardBuffer.data(), info.x104_cardBuffer.size());
    //CARDWriteAsync(&info.x4_info, info.x104_cardBuffer.data(), info.x104_cardBuffer.size(), 0, 0);
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::CCardFileInfo::Close()
{
    EMemoryCardPort port = GetCardPort();
    //CARDClose(port);
    x4_info.chan = port;
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::CardProbeResults CMemoryCardSys::CardProbe(EMemoryCardPort port)
{
    return {};
}

CMemoryCardSys::ECardResult CMemoryCardSys::MountCard(EMemoryCardPort port)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::CheckCard(EMemoryCardPort port)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::GetNumFreeBytes(EMemoryCardPort port, s32& freeBytes, s32& freeFiles)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::GetSerialNo(EMemoryCardPort port, u64& serialOut)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::GetResultCode(EMemoryCardPort port)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::GetStatus(EMemoryCardPort port, int fileNo, CARDStat& statOut)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::SetStatus(EMemoryCardPort port, int fileNo, const CARDStat& stat)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::DeleteFile(EMemoryCardPort port, const char* name)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::FastDeleteFile(EMemoryCardPort port, int fileNo)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::Rename(EMemoryCardPort port, const char* oldName, const char* newName)
{
    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardSys::ECardResult CMemoryCardSys::FormatCard(EMemoryCardPort port)
{
    return ECardResult::CARD_RESULT_READY;
}

}
