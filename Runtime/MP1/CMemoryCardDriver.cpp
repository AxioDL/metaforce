#include "CMemoryCardDriver.hpp"
#include "MP1.hpp"
#include "CCRC32.hpp"

namespace urde
{
namespace MP1
{

static const char* SaveFileNames[] =
{
    "MetroidPrime A",
    "MetroidPrime B"
};

using ECardResult = CMemoryCardSys::ECardResult;
using EMemoryCardPort = CMemoryCardSys::EMemoryCardPort;

ECardResult CMemoryCardDriver::SFileInfo::Open()
{
    //CARDOpen(GetFileCardPort(), x14_name.data(), &x0_fileInfo);
    return ECardResult::CARD_RESULT_READY;
}

ECardResult CMemoryCardDriver::SFileInfo::Close()
{
    auto backup = GetFileCardPort();
    ECardResult result = ECardResult::CARD_RESULT_READY;
    //result = CARDClose(backup);
    x0_fileInfo.x0_cardPort = backup;
    return result;
}

CMemoryCardSys::ECardResult CMemoryCardDriver::SFileInfo::StartRead()
{
    CMemoryCardSys::CARDStat stat = {};
    ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
    if (result != ECardResult::CARD_RESULT_READY)
        return result;

    u32 length = stat.GetFileLength();
    x34_saveData.clear();
    x24_saveFileData.resize(length);
    //return CARDReadAsync(&x0_fileInfo, x24_saveFileData.data(), length, 0, 0);
    return ECardResult::CARD_RESULT_READY;
}

ECardResult CMemoryCardDriver::SFileInfo::TryFileRead()
{
    ECardResult res = CMemoryCardSys::GetResultCode(GetFileCardPort());
    if (res == ECardResult::CARD_RESULT_READY)
        res = FileRead();
    return res;
}

ECardResult CMemoryCardDriver::SFileInfo::FileRead()
{
    x34_saveData.clear();
    u32 existingCrc = hecl::SBig(*reinterpret_cast<u32*>(x24_saveFileData.data()));
    u32 newCrc = CCRC32::Calculate(x24_saveFileData.data() + 4, x24_saveFileData.size() - 4);
    if (existingCrc == newCrc)
    {
        u32 saveDataOff;
        ECardResult result = GetSaveDataOffset(saveDataOff);
        if (result != ECardResult::CARD_RESULT_READY)
        {
            x24_saveFileData.clear();
            return result;
        }

        u32 saveSize = x24_saveFileData.size() - saveDataOff;
        x34_saveData.resize(saveSize);
        memmove(x34_saveData.data(), x24_saveFileData.data() + saveDataOff, saveSize);
        x24_saveFileData.clear();
        return ECardResult::CARD_RESULT_READY;
    }
    else
    {
        x24_saveFileData.clear();
        return ECardResult::CARD_RESULT_CRC_MISMATCH;
    }
}

ECardResult CMemoryCardDriver::SFileInfo::GetSaveDataOffset(u32& offOut)
{
    CMemoryCardSys::CARDStat stat = {};
    ECardResult result = CMemoryCardSys::GetStatus(GetFileCardPort(), GetFileNo(), stat);
    if (result != ECardResult::CARD_RESULT_READY)
    {
        offOut = -1;
        return result;
    }

    offOut = 4;
    offOut += 64;
    switch (stat.GetBannerFormat())
    {
    case 1:
        offOut += 3584;
        break;
    case 2:
        offOut += 6144;
        break;
    default: break;
    }

    int idx = 0;
    bool paletteIcon = false;
    while (u32 fmt = stat.GetIconFormat(idx))
    {
        if (fmt == 1)
        {
            paletteIcon = true;
            offOut += 1024;
        }
        else
            offOut += 2048;
        ++idx;
    }

    if (paletteIcon)
        offOut += 512;

    return ECardResult::CARD_RESULT_READY;
}

CMemoryCardDriver::SGameFileSlot::SGameFileSlot()
{
    InitializeFromGameState();
}

CMemoryCardDriver::SGameFileSlot::SGameFileSlot(CMemoryInStream& in)
{
    in.readBytesToBuf(x0_saveBuffer, 940);
    x944_fileInfo = CGameState::LoadGameFileState(x0_saveBuffer);
}

void CMemoryCardDriver::SGameFileSlot::InitializeFromGameState()
{
    CBitStreamWriter w(x0_saveBuffer, 940);
    g_GameState->PutTo(w);
    x944_fileInfo = CGameState::LoadGameFileState(x0_saveBuffer);
}

CMemoryCardDriver::SFileInfo::SFileInfo(EMemoryCardPort port,
                                        const std::string& name)
: x14_name(name)
{
    x0_fileInfo.x0_cardPort = port;
}

CMemoryCardDriver::CMemoryCardDriver(EMemoryCardPort cardPort, ResId saveBanner,
                                     ResId saveIcon0, ResId saveIcon1, bool importPersistent)
: x0_cardPort(cardPort), x4_saveBanner(saveBanner),
  x8_saveIcon0(saveIcon0), xc_saveIcon1(saveIcon1), x19d_doImportPersistent(importPersistent)
{
    x100_mcFileInfos.reserve(2);
    x100_mcFileInfos.emplace_back(0, SFileInfo(x0_cardPort, SaveFileNames[0]));
    x100_mcFileInfos.emplace_back(0, SFileInfo(x0_cardPort, SaveFileNames[1]));
}

void CMemoryCardDriver::FinishedLoading()
{
    x10_state = EState::TwentyFive;
    x14_error = EError::Zero;
    FinishedLoading2();
}

void CMemoryCardDriver::FinishedLoading2()
{
    auto result = CMemoryCardSys::CardProbe(x0_cardPort);
    switch (result.x0_error)
    {
    case ECardResult::CARD_RESULT_READY:
        if (result.x8_sectorSize != 0x2000)
        {
            x10_state = EState::Twelve;
            x14_error = EError::Seven;
            return;
        }
        x10_state = EState::CardNeedsMount;
        MountCard();
        break;
    case ECardResult::CARD_RESULT_BUSY:
        break;
    case ECardResult::CARD_RESULT_WRONGDEVICE:
        x10_state = EState::Twelve;
        x14_error = EError::Four;
        break;
    default:
        NoCardFound();
        break;
    }
}

void CMemoryCardDriver::NoCardFound()
{
    x10_state = EState::NoCard;
    static_cast<CMain*>(g_Main)->SetCardInserted(false);
}

void CMemoryCardDriver::MountCard()
{
    x10_state = EState::CardMount;
    x14_error = EError::Zero;
    ECardResult result = CMemoryCardSys::MountCard(x0_cardPort);
    if (result != ECardResult::CARD_RESULT_READY)
        Case26(result);
}

void CMemoryCardDriver::CheckCard()
{
    ECardResult result = CMemoryCardSys::CheckCard(x0_cardPort);
    if (result != ECardResult::CARD_RESULT_READY)
        Case27(result);
}

CGameState::GameFileStateInfo* CMemoryCardDriver::GetGameFileStateInfo(int idx)
{
    SGameFileSlot* slot = xe4_fileSlots[idx].get();
    if (!slot)
        return nullptr;
    return &slot->x944_fileInfo;
}

CMemoryCardDriver::SSaveHeader CMemoryCardDriver::LoadSaveHeader(CMemoryInStream& in)
{
    SSaveHeader ret;
    ret.x0_ = in.readUint32Big();
    for (int i=0 ; i<3 ; ++i)
        ret.x4_[i] = in.readBool();
    return ret;
}

std::unique_ptr<CMemoryCardDriver::SGameFileSlot> CMemoryCardDriver::LoadSaveFile(CMemoryInStream& in)
{
    return std::make_unique<CMemoryCardDriver::SGameFileSlot>(in);
}

void CMemoryCardDriver::ReadFinished()
{
    CMemoryCardSys::CARDStat stat = {};
    SFileInfo& fileInfo = x100_mcFileInfos[x194_fileIdx].second;
    if (CMemoryCardSys::GetStatus(x0_cardPort, fileInfo.GetFileNo(), stat) != ECardResult::CARD_RESULT_READY)
    {
        NoCardFound();
        return;
    }

    x20_fileTime = stat.GetTime();
    CMemoryInStream r(fileInfo.x34_saveData.data(), 3004);
    SSaveHeader header = LoadSaveHeader(r);
    r.readBytesToBuf(x30_systemData, 174);

    for (int i=0 ; i<3 ; ++i)
        xe4_fileSlots[i] = LoadSaveFile(r);

    if (x19d_doImportPersistent)
        ImportPersistentOptions();
}

void CMemoryCardDriver::ImportPersistentOptions()
{
    CBitStreamReader r(x30_systemData, 174);
    CPersistentOptions opts(r);
    g_GameState->ImportPersistentOptions(opts);
}

void CMemoryCardDriver::ExportPersistentOptions()
{
    CBitStreamReader r(x30_systemData, 174);
    CPersistentOptions opts(r);
    g_GameState->ExportPersistentOptions(opts);
    CBitStreamWriter w(x30_systemData, 174);
    opts.PutTo(w);
}

void CMemoryCardDriver::DeleteFile()
{
    x14_error = EError::Zero;
    x10_state = EState::Thirty;
    SFileInfo& fileInfo = x100_mcFileInfos[!bool(x194_fileIdx)].second;
    ECardResult result = CMemoryCardSys::FastDeleteFile(x0_cardPort, fileInfo.GetFileNo());
    if (result != ECardResult::CARD_RESULT_READY)
        Case30(result);
}

void CMemoryCardDriver::CheckCardCapacity()
{
    if (x18_cardFreeBytes < 0x2000 || !x1c_cardFreeFiles)
        x14_error = EError::Six;
}

void CMemoryCardDriver::Case26(ECardResult result)
{
    switch (result)
    {
    case ECardResult::CARD_RESULT_READY:
        x10_state = EState::CardMountDone;
        CheckCard();
        break;
    case ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::CardMountDone;
        x14_error = EError::One;
        CheckCard();
        break;
    default:
        HandleCardError(result, EState::Thirteen);
        break;
    }
}

void CMemoryCardDriver::Case27(ECardResult result)
{
    switch (result)
    {
    case ECardResult::CARD_RESULT_READY:
        x10_state = EState::SelectCardFile;
        if (!GetCardFreeBytes())
            return;
        if (CMemoryCardSys::GetSerialNo(x0_cardPort, x28_cardSerial) == ECardResult::CARD_RESULT_READY)
            return;
        NoCardFound();
        break;
    case ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::Fourteen;
        x14_error = EError::One;
        break;
    default:
        HandleCardError(result, EState::Fourteen);
    }
}

void CMemoryCardDriver::Case28(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x100_mcFileInfos[x194_fileIdx].first = 1;
        if (x100_mcFileInfos[!bool(x194_fileIdx)].first == 3)
        {
            x10_state = EState::Seventeen;
            GoTo28();
        }
        else
        {
            x10_state = EState::SelectCardFile;
            if (!GetCardFreeBytes())
                return;
            GoTo17();
        }
    }
    else
        HandleCardError(result, EState::Fifteen);
}

void CMemoryCardDriver::Case29(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        auto& fileInfo = x100_mcFileInfos[x194_fileIdx];
        ECardResult readRes = fileInfo.second.TryFileRead();
        if (fileInfo.second.Close() != ECardResult::CARD_RESULT_READY)
        {
            NoCardFound();
            return;
        }

        u32 fileIdx = !bool(x194_fileIdx);
        if (readRes == ECardResult::CARD_RESULT_READY)
        {
            x10_state = EState::Ready;
            ReadFinished();
            u32 fileId = x100_mcFileInfos[fileIdx].first;
            if (fileId == 1)
                CheckCardCapacity();
            else
                DeleteFile();
            return;
        }

        if (readRes == ECardResult::CARD_RESULT_CRC_MISMATCH)
        {
            x100_mcFileInfos[x194_fileIdx].first = 3;
            if (x100_mcFileInfos[fileIdx].first == 2)
            {
                x10_state = EState::SelectCardFile;
                GoTo17();
            }
            else
            {
                x10_state = EState::Seventeen;
                x14_error = EError::Nine;
            }
        }
    }
    else
        HandleCardError(result, EState::Seventeen);
}

void CMemoryCardDriver::Case30(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x10_state = EState::Ready;
        if (GetCardFreeBytes())
            CheckCardCapacity();
    }
    else
        HandleCardError(result, EState::Sixteen);
}

void CMemoryCardDriver::Case31(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x10_state = EState::WillWrite;
        GoTo32();
    }
    else
        HandleCardError(result, EState::Eighteen);
}

void CMemoryCardDriver::Case32(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        ECardResult xferResult = x198_fileInfo->PumpCardTransfer();
        if (xferResult == ECardResult::CARD_RESULT_READY)
        {
            x10_state = EState::Ready;
            if (x198_fileInfo->Close() == ECardResult::CARD_RESULT_READY)
                return;
            NoCardFound();
            return;
        }
        if (xferResult == ECardResult::CARD_RESULT_BUSY)
            return;
        if (xferResult == ECardResult::CARD_RESULT_IOERROR)
        {
            x10_state = EState::Nineteen;
            x14_error = EError::Three;
            return;
        }
        NoCardFound();
    }
    else
        HandleCardError(result, EState::Nineteen);
}

void CMemoryCardDriver::Case33(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x10_state = EState::Nine;
        GoTo34();
    }
    else
        HandleCardError(result, EState::Twenty);
}

void CMemoryCardDriver::Case34(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        ECardResult xferResult = x198_fileInfo->PumpCardTransfer();
        if (xferResult == ECardResult::CARD_RESULT_READY)
        {
            x10_state = EState::Ten;
            if (x198_fileInfo->Close() != ECardResult::CARD_RESULT_READY)
            {
                NoCardFound();
                return;
            }
            GoTo35();
            return;
        }
        if (xferResult == ECardResult::CARD_RESULT_BUSY)
            return;
        if (xferResult == ECardResult::CARD_RESULT_IOERROR)
        {
            x10_state = EState::TwentyOne;
            x14_error = EError::Three;
            return;
        }
        NoCardFound();
    }
    else
        HandleCardError(result, EState::TwentyOne);
}

void CMemoryCardDriver::Case35(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x10_state = EState::Eleven;
        if (GetCardFreeBytes())
            GoTo36();
    }
    else
        HandleCardError(result, EState::TwentyTwo);
}

void CMemoryCardDriver::Case36(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
    {
        x10_state = EState::RuntimeBackup;
        WriteBackupBuf();
    }
    else
        HandleCardError(result, EState::TwentyThree);
}

void CMemoryCardDriver::Case37(ECardResult result)
{
    if (result == ECardResult::CARD_RESULT_READY)
        x10_state = EState::CardFormatted;
    else if (result == ECardResult::CARD_RESULT_BROKEN)
    {
        x10_state = EState::CardFormatBroken;
        x14_error = EError::Three;
    }
    else
        HandleCardError(result, EState::CardFormatBroken);
}

void CMemoryCardDriver::GoTo17()
{
    x14_error = EError::Zero;
    for (std::pair<u32, SFileInfo>& info : x100_mcFileInfos)
    {
        if (info.first == 0)
        {
            ECardResult result = info.second.Open();
            if (result == ECardResult::CARD_RESULT_NOFILE)
            {
                info.first = 1;
                continue;
            }
            else if (result == ECardResult::CARD_RESULT_READY)
            {
                CMemoryCardSys::CARDStat stat = {};
                if (CMemoryCardSys::GetStatus(x0_cardPort, info.second.GetFileNo(), stat) ==
                        ECardResult::CARD_RESULT_READY)
                {
                    u32 comment = stat.GetCommentAddr();
                    if (comment == -1)
                        info.first = 3;
                    else
                        info.first = 2;
                }
                else
                {
                    NoCardFound();
                    return;
                }
                if (info.second.Close() == ECardResult::CARD_RESULT_NOCARD)
                {
                    NoCardFound();
                    return;
                }
            }
            else
            {
                NoCardFound();
                return;
            }
        }
    }

    if (x100_mcFileInfos[0].first == 2)
    {
        if (x100_mcFileInfos[1].first == 2)
        {
            CMemoryCardSys::CARDStat stat = {};
            if (CMemoryCardSys::GetStatus(x0_cardPort, x100_mcFileInfos[0].second.GetFileNo(), stat) ==
                    ECardResult::CARD_RESULT_READY)
            {
                u32 timeA = stat.GetTime();
                if (CMemoryCardSys::GetStatus(x0_cardPort, x100_mcFileInfos[1].second.GetFileNo(), stat) ==
                        ECardResult::CARD_RESULT_READY)
                {
                    u32 timeB = stat.GetTime();
                    if (timeA > timeB)
                        x194_fileIdx = 0;
                    else
                        x194_fileIdx = 1;
                    GoTo29();
                    return;
                }
                NoCardFound();
                return;
            }
            NoCardFound();
            return;
        }
        x194_fileIdx = 0;
        GoTo29();
        return;
    }

    if (x100_mcFileInfos[1].first == 2)
    {
        x194_fileIdx = 1;
        GoTo29();
        return;
    }

    if (x100_mcFileInfos[0].first == 3 || x100_mcFileInfos[1].first == 3)
    {
        x10_state = EState::Seventeen;
        x14_error = EError::Nine;
    }
    else
    {
        x10_state = EState::Seventeen;
        x14_error = EError::Eight;
    }
}

void CMemoryCardDriver::GoTo28()
{
    x14_error = EError::Zero;
    x10_state = EState::TwentyEight;
    int idx = 0;
    for (std::pair<u32, SFileInfo>& info : x100_mcFileInfos)
    {
        if (info.first == 3)
        {
            x194_fileIdx = idx;
            ECardResult result = CMemoryCardSys::FastDeleteFile(x0_cardPort, info.second.GetFileNo());
            if (result != ECardResult::CARD_RESULT_READY)
            {
                Case28(result);
                return;
            }
        }
        ++idx;
    }
}

void CMemoryCardDriver::GoTo29()
{
    x14_error = EError::Zero;
    x10_state = EState::TwentyNine;
    ECardResult result = x100_mcFileInfos[x194_fileIdx].second.Open();
    if (result != ECardResult::CARD_RESULT_READY)
    {
        Case29(result);
        return;
    }

    result = x100_mcFileInfos[x194_fileIdx].second.StartRead();
    if (result != ECardResult::CARD_RESULT_READY)
        Case29(result);
}

void CMemoryCardDriver::GoTo32()
{
    x14_error = EError::Zero;
    x10_state = EState::Write;
    ECardResult result = x198_fileInfo->Write();
    if (result != ECardResult::CARD_RESULT_READY)
        Case32(result);
}

void CMemoryCardDriver::GoTo33()
{
    x14_error = EError::Zero;
    x10_state = EState::ThirtyThree;
    ClearFileInfo();
    if (x18_cardFreeBytes < 8192 || !x1c_cardFreeFiles)
    {
        x10_state = EState::Twenty;
        x14_error = EError::Five;
        return;
    }

    x198_fileInfo = std::make_unique<CMemoryCardSys::CCardFileInfo>(x0_cardPort, SaveFileNames[x194_fileIdx]);
    InitializeFileInfo();
    ECardResult result = x198_fileInfo->CreateFile();
    if (result != ECardResult::CARD_RESULT_READY)
        Case33(result);
}

void CMemoryCardDriver::GoTo34()
{
    x14_error = EError::Zero;
    x10_state = EState::ThirtyFour;
    ECardResult result = x198_fileInfo->Write();
    if (result != ECardResult::CARD_RESULT_READY)
        Case34(result);
}

void CMemoryCardDriver::GoTo35()
{
    x14_error = EError::Zero;
    x10_state = EState::ThirtyFive;
    ECardResult result = CMemoryCardSys::DeleteFile(x0_cardPort,
                                                    SaveFileNames[!bool(x194_fileIdx)]);
    if (result != ECardResult::CARD_RESULT_READY)
        Case35(result);
}

void CMemoryCardDriver::GoTo36()
{
    if (x194_fileIdx == 1)
    {
        x14_error = EError::Zero;
        x10_state = EState::ThirtySix;
        ECardResult result = CMemoryCardSys::Rename(x0_cardPort,
                                                    SaveFileNames[x194_fileIdx],
                                                    SaveFileNames[!bool(x194_fileIdx)]);
        if (result != ECardResult::CARD_RESULT_READY)
            Case36(result);
    }
    else
    {
        x10_state = EState::RuntimeBackup;
        WriteBackupBuf();
    }
}

void CMemoryCardDriver::GoTo37()
{
    x14_error = EError::Zero;
    x10_state = EState::CardFormat;
    ECardResult result = CMemoryCardSys::FormatCard(x0_cardPort);
    if (result != ECardResult::CARD_RESULT_READY)
        Case37(result);
}

void CMemoryCardDriver::InitializeFileInfo()
{
    ExportPersistentOptions();
    /* TODO: Finish */
}

void CMemoryCardDriver::WriteBackupBuf()
{
    g_GameState->WriteBackupBuf();
    g_GameState->SetCardSerial(x28_cardSerial);
}

bool CMemoryCardDriver::GetCardFreeBytes()
{
    ECardResult result = CMemoryCardSys::GetNumFreeBytes(x0_cardPort,
                                                         x18_cardFreeBytes,
                                                         x1c_cardFreeFiles);
    if (result == ECardResult::CARD_RESULT_READY)
        return true;
    NoCardFound();
    return false;
}

void CMemoryCardDriver::HandleCardError(ECardResult result, EState state)
{
    switch (result)
    {
    case ECardResult::CARD_RESULT_WRONGDEVICE:
        x10_state = state;
        x14_error = EError::Four;
        break;
    case ECardResult::CARD_RESULT_NOCARD:
        NoCardFound();
        break;
    case ECardResult::CARD_RESULT_IOERROR:
        x10_state = state;
        x14_error = EError::Three;
    case ECardResult::CARD_RESULT_ENCODING:
        x10_state = state;
        x14_error = EError::Two;
        break;
    default: break;
    }
}

void CMemoryCardDriver::Update()
{
    CMemoryCardSys::CardProbeResults result = CMemoryCardSys::CardProbe(x0_cardPort);

    if (result.x0_error == ECardResult::CARD_RESULT_NOCARD)
    {
        if (x10_state != EState::NoCard)
            NoCardFound();
        static_cast<CMain*>(g_Main)->SetCardInserted(false);
        return;
    }

    if (x10_state == EState::TwentyFive)
    {
        FinishedLoading2();
        static_cast<CMain*>(g_Main)->SetCardInserted(false);
        return;
    }


    ECardResult resultCode = g_MemoryCardSys->GetResultCode(x0_cardPort);
    bool cardInserted = false;

    if (InCardInsertedRange(x10_state))
    {
        cardInserted = true;

        switch (x10_state)
        {
        case EState::CardMount:
            Case26(resultCode);
            break;
        case EState::TwentySeven:
            Case27(resultCode);
            break;
        case EState::TwentyEight:
            Case28(resultCode);
            break;
        case EState::TwentyNine:
            Case29(resultCode);
            break;
        case EState::Thirty:
            Case30(resultCode);
            break;
        case EState::ThirtyOne:
            Case31(resultCode);
            break;
        case EState::Write:
            Case32(resultCode);
            break;
        case EState::ThirtyThree:
            Case33(resultCode);
            break;
        case EState::ThirtyFour:
            Case34(resultCode);
            break;
        case EState::ThirtyFive:
            Case35(resultCode);
            break;
        case EState::ThirtySix:
            Case36(resultCode);
            break;
        case EState::CardFormat:
            Case37(resultCode);
            break;
        default: break;
        }
    }

    static_cast<CMain*>(g_Main)->SetCardInserted(cardInserted);
}

}
}
