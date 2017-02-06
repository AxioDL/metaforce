#include "CMemoryCardSys.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CGameState.hpp"
#include "GuiSys/CStringTable.hpp"
#include "CCRC32.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

using ECardResult = kabufuda::ECardResult;

static kabufuda::SystemString g_CardImagePaths[2] = {};
static kabufuda::Card g_CardStates[2] = {};
static kabufuda::ECardResult g_OpResults[2] = {};

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

bool CMemoryCardSys::HasSaveWorldMemory(ResId wldId) const
{
    auto existingSearch =
        std::find_if(xc_memoryWorlds.cbegin(), xc_memoryWorlds.cend(), [&](const auto& wld)
        { return wld.first == wldId; });
    return existingSearch != xc_memoryWorlds.cend();
}

const CSaveWorldMemory& CMemoryCardSys::GetSaveWorldMemory(ResId wldId) const
{
    auto existingSearch =
        std::find_if(xc_memoryWorlds.cbegin(), xc_memoryWorlds.cend(), [&](const auto& wld)
        { return wld.first == wldId; });
    return existingSearch->second;
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
            if (!HasSaveWorldMemory(tag.id))
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
            {
                auto existingSearch =
                    std::find_if(x20_scanStates.begin(), x20_scanStates.end(), [&](const auto& test)
                    { return test.first == scan.x0_id && test.second == scan.x4_category; });
                if (existingSearch == x20_scanStates.end())
                    x20_scanStates.emplace_back(scan.x0_id, scan.x4_category);
            }

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

CMemoryCardSys::CCardFileInfo::Icon::Icon(ResId id, kabufuda::EAnimationSpeed speed,
                                          CSimplePool& sp, const CVParamTransfer& cv)
: x0_id(id), x4_speed(speed), x8_tex(sp.GetObj({FOURCC('TXTR'), id}, cv)) {}

void CMemoryCardSys::CCardFileInfo::LockIconToken(ResId iconTxtr, kabufuda::EAnimationSpeed speed, CSimplePool& sp)
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

ECardResult CMemoryCardSys::CCardFileInfo::PumpCardTransfer()
{
    if (x0_status == EStatus::Standby)
        return ECardResult::READY;
    else if (x0_status == EStatus::Transferring)
    {
        ECardResult result = CMemoryCardSys::GetResultCode(GetCardPort());
        if (result != ECardResult::BUSY)
            x104_cardBuffer.clear();
        if (result != ECardResult::READY)
            return result;
        x0_status = EStatus::Done;
        kabufuda::CardStat stat = {};
        result = GetStatus(stat);
        if (result != ECardResult::READY)
            return result;
        result = CMemoryCardSys::SetStatus(m_handle.slot, m_handle.getFileNo(), stat);
        if (result != ECardResult::READY)
            return result;
        return ECardResult::BUSY;
    }
    else
    {
        ECardResult result = CMemoryCardSys::GetResultCode(GetCardPort());
        if (result == ECardResult::READY)
            x0_status = EStatus::Standby;
        return result;
    }
}

ECardResult CMemoryCardSys::CCardFileInfo::GetStatus(kabufuda::CardStat& stat) const
{
    ECardResult result = CMemoryCardSys::GetStatus(m_handle.slot, m_handle.getFileNo(), stat);
    if (result != ECardResult::READY)
        return result;

    stat.SetCommentAddr(4);
    stat.SetIconAddr(68);

    kabufuda::EImageFormat bannerFmt;
    if (x3c_bannerTex != -1)
    {
        if ((*x40_bannerTok)->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3)
            bannerFmt = kabufuda::EImageFormat::RGB5A3;
        else
            bannerFmt = kabufuda::EImageFormat::C8;
    }
    else
        bannerFmt = kabufuda::EImageFormat::None;
    stat.SetBannerFormat(bannerFmt);

    int idx = 0;
    for (const Icon& icon : x50_iconToks)
    {
        stat.SetIconFormat(icon.x8_tex->GetMemoryCardTexelFormat() == ETexelFormat::RGB5A3 ?
                           kabufuda::EImageFormat::RGB5A3 : kabufuda::EImageFormat::C8, idx);
        stat.SetIconSpeed(icon.x4_speed, idx);
        ++idx;
    }
    if (idx < 8)
    {
        stat.SetIconFormat(kabufuda::EImageFormat::None, idx);
        stat.SetIconSpeed(kabufuda::EAnimationSpeed::End, idx);
    }

    return ECardResult::READY;
}

ECardResult CMemoryCardSys::CCardFileInfo::CreateFile()
{
    return CMemoryCardSys::CreateFile(m_handle.slot, x18_fileName.c_str(),
                                      CalculateTotalDataSize(), m_handle);
}

ECardResult CMemoryCardSys::CCardFileInfo::WriteFile()
{
    BuildCardBuffer();
    //DCStoreRange(x104_cardBuffer.data(), x104_cardBuffer.size());
    x0_status = EStatus::Transferring;
    return CMemoryCardSys::WriteFile(m_handle, x104_cardBuffer.data(), x104_cardBuffer.size(), 0);
}

ECardResult CMemoryCardSys::CCardFileInfo::CloseFile()
{
    return CMemoryCardSys::CloseFile(m_handle);
}

kabufuda::ProbeResults CMemoryCardSys::CardProbe(kabufuda::ECardSlot port)
{
    g_CardImagePaths[0] = ResolveDolphinCardPath(kabufuda::ECardSlot::SlotA);
    g_CardImagePaths[1] = ResolveDolphinCardPath(kabufuda::ECardSlot::SlotB);

    kabufuda::ProbeResults res = kabufuda::Card::probeCardFile(g_CardImagePaths[int(port)]);
    g_OpResults[int(port)] = res.x0_error;
    return res;
}

ECardResult CMemoryCardSys::MountCard(kabufuda::ECardSlot port)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    card.close();
    card = kabufuda::Card(g_CardImagePaths[int(port)], "GM8E", "01");
    ECardResult result = card.getError();
    g_OpResults[int(port)] = result;
    if (result == ECardResult::READY)
        return ECardResult::READY;
    return result;
}

ECardResult CMemoryCardSys::UnmountCard(kabufuda::ECardSlot port)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    card = kabufuda::Card();
    g_OpResults[int(port)] = ECardResult::READY;
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::CheckCard(kabufuda::ECardSlot port)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    ECardResult result = card.getError();
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::CreateFile(kabufuda::ECardSlot port, const char* name, u32 size, CardFileHandle& info)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    info.slot = port;
    ECardResult result = card.createFile(name, size, info.handle);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::OpenFile(kabufuda::ECardSlot port, const char* name, CardFileHandle& info)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    info.slot = port;
    ECardResult result = card.openFile(name, info.handle);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::FastOpenFile(kabufuda::ECardSlot port, int fileNo, CardFileHandle& info)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    info.slot = port;
    ECardResult result = card.openFile(fileNo, info.handle);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::CloseFile(CardFileHandle& info)
{
    kabufuda::Card& card = g_CardStates[int(info.slot)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(info.slot)] = err;
        return err;
    }
    card.closeFile(info.handle);
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::ReadFile(CardFileHandle& info, void* buf, s32 length, s32 offset)
{
    kabufuda::Card& card = g_CardStates[int(info.slot)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(info.slot)] = err;
        return err;
    }
    card.seek(info.handle, offset, kabufuda::SeekOrigin::Begin);
    card.read(info.handle, buf, length);
    g_OpResults[int(info.slot)] = ECardResult::READY;
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::WriteFile(CardFileHandle& info, const void* buf, s32 length, s32 offset)
{
    kabufuda::Card& card = g_CardStates[int(info.slot)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(info.slot)] = err;
        return err;
    }
    card.seek(info.handle, offset, kabufuda::SeekOrigin::Begin);
    card.write(info.handle, buf, length);
    g_OpResults[int(info.slot)] = ECardResult::READY;
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::GetNumFreeBytes(kabufuda::ECardSlot port, s32& freeBytes, s32& freeFiles)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    card.getFreeBlocks(freeBytes, freeFiles);
    g_OpResults[int(port)] = ECardResult::READY;
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::GetSerialNo(kabufuda::ECardSlot port, u64& serialOut)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    card.getSerial(serialOut);
    g_OpResults[int(port)] = ECardResult::READY;
    return ECardResult::READY;
}

ECardResult CMemoryCardSys::GetResultCode(kabufuda::ECardSlot port)
{
    return g_OpResults[int(port)];
}

ECardResult CMemoryCardSys::GetStatus(kabufuda::ECardSlot port, int fileNo, CardStat& statOut)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    ECardResult result = card.getStatus(fileNo, statOut);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::SetStatus(kabufuda::ECardSlot port, int fileNo, const CardStat& stat)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    ECardResult result = card.setStatus(fileNo, stat);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::DeleteFile(kabufuda::ECardSlot port, const char* name)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    ECardResult result = card.deleteFile(name);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::FastDeleteFile(kabufuda::ECardSlot port, int fileNo)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    ECardResult result = card.deleteFile(fileNo);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::Rename(kabufuda::ECardSlot port, const char* oldName, const char* newName)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    ECardResult result = card.renameFile(oldName, newName);
    g_OpResults[int(port)] = result;
    return result;
}

ECardResult CMemoryCardSys::FormatCard(kabufuda::ECardSlot port)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    card.format(port);
    if (CardResult err = card.getError())
    {
        g_OpResults[int(port)] = err;
        return err;
    }
    g_OpResults[int(port)] = ECardResult::READY;
    return ECardResult::READY;
}

void CMemoryCardSys::CommitToDisk(kabufuda::ECardSlot port)
{
    kabufuda::Card& card = g_CardStates[int(port)];
    card.commit();
}

kabufuda::SystemString CMemoryCardSys::CreateDolphinCard(kabufuda::ECardSlot slot)
{
    kabufuda::SystemString path = _CreateDolphinCard(slot);
    CardProbe(slot);
    MountCard(slot);
    FormatCard(slot);
    return path;
}

void CMemoryCardSys::Shutdown()
{
    UnmountCard(kabufuda::ECardSlot::SlotA);
    UnmountCard(kabufuda::ECardSlot::SlotB);
}

}
