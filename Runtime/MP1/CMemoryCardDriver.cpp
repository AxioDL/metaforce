#include "CMemoryCardDriver.hpp"
#include "MP1.hpp"

namespace urde
{
namespace MP1
{

CMemoryCardDriver::SMemoryCardSlotInfo::SMemoryCardSlotInfo(CMemoryCardSys::EMemoryCardPort parentIdx,
                                                            const std::string& name)
: x0_cardPort(parentIdx), x14_name(name)
{}

CMemoryCardDriver::CMemoryCardDriver(CMemoryCardSys::EMemoryCardPort cardPort, ResId saveBanner,
                                     ResId saveIcon0, ResId saveIcon1, bool flag)
: x0_cardPort(cardPort), x4_saveBanner(saveBanner),
  x8_saveIcon0(saveIcon0), xc_saveIcon1(saveIcon1), x19d_flag(flag)
{
    x100_mcSlotInfos.reserve(2);
    x100_mcSlotInfos.emplace_back(0, SMemoryCardSlotInfo(x0_cardPort, "MetroidPrime A"));
    x100_mcSlotInfos.emplace_back(0, SMemoryCardSlotInfo(x0_cardPort, "MetroidPrime B"));
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
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        if (result.x8_sectorSize != 0x2000)
        {
            x10_state = EState::Twelve;
            x14_error = EError::Seven;
            return;
        }
        x10_state = EState::Five;
        MountCard();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BUSY:
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_WRONGDEVICE:
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
    x10_state = EState::Two;
    static_cast<CMain*>(g_Main)->SetCardInserted(false);
}

void CMemoryCardDriver::MountCard()
{
    x10_state = EState::TwentySix;
    x14_error = EError::Zero;
    CMemoryCardSys::ECardResult result = CMemoryCardSys::MountCard(x0_cardPort);
    if (result != CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        MountCardFailed(result);
}

void CMemoryCardDriver::MountCardFailed(CMemoryCardSys::ECardResult result)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        x10_state = EState::Six;
        CheckCard();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::Six;
        x14_error = EError::One;
        CheckCard();
        break;
    default:
        HandleCardError(result, EState::Thirteen);
        break;
    }
}

void CMemoryCardDriver::CheckCard()
{
    CMemoryCardSys::ECardResult result = CMemoryCardSys::CheckCard(x0_cardPort);
    if (result != CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        CheckCardFailed(result);
}

void CMemoryCardDriver::CheckCardFailed(CMemoryCardSys::ECardResult result)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_READY:
        x10_state = EState::Seven;
        if (!GetCardFreeBytes())
            return;
        if (CMemoryCardSys::GetSerialNo(x0_cardPort, x28_cardSerial) ==
                CMemoryCardSys::ECardResult::CARD_RESULT_READY)
            return;
        NoCardFound();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_BROKEN:
        x10_state = EState::Fourteen;
        x14_error = EError::One;
        break;
    default:
        HandleCardError(result, EState::Fourteen);
    }
}

bool CMemoryCardDriver::GetCardFreeBytes()
{
    CMemoryCardSys::ECardResult result = CMemoryCardSys::GetNumFreeBytes(x0_cardPort,
                                                                         x18_cardFreeBytes,
                                                                         x1c_cardFreeFiles);
    if (result == CMemoryCardSys::ECardResult::CARD_RESULT_READY)
        return true;
    NoCardFound();
    return false;
}

void CMemoryCardDriver::HandleCardError(CMemoryCardSys::ECardResult result, EState state)
{
    switch (result)
    {
    case CMemoryCardSys::ECardResult::CARD_RESULT_WRONGDEVICE:
        x10_state = state;
        x14_error = EError::Four;
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_NOCARD:
        NoCardFound();
        break;
    case CMemoryCardSys::ECardResult::CARD_RESULT_IOERROR:
        x10_state = state;
        x14_error = EError::Three;
    case CMemoryCardSys::ECardResult::CARD_RESULT_ENCODING:
        x10_state = state;
        x14_error = EError::Two;
        break;
    default: break;
    }
}

void CMemoryCardDriver::Update()
{
    auto result = CMemoryCardSys::CardProbe(x0_cardPort);

    if (result.x0_error == CMemoryCardSys::ECardResult::CARD_RESULT_NOCARD)
    {
        if (x10_state != EState::Two)
            NoCardFound();
        static_cast<CMain*>(g_Main)->SetCardInserted(false);
        return;
    }

    switch (x10_state)
    {
    case EState::TwentySix:
    case EState::TwentySeven:
    case EState::TwentyEight:
    case EState::TwentyNine:
    case EState::Thirty:
    case EState::ThirtyOne:
    case EState::ThirtyTwo:
    case EState::ThirtyThree:
    case EState::ThirtyFour:
    case EState::ThirtyFive:
    case EState::ThirtySix:
    case EState::ThirtySeven:
    default: break;
    }
}

}
}
