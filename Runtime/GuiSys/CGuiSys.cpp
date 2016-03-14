#include "CGuiSys.hpp"
#include "CGuiWidget.hpp"
#include "CGuiHeadWidget.hpp"
#include "CGuiBackground.hpp"
#include "CGuiLight.hpp"
#include "CGuiCamera.hpp"
#include "CGuiGroup.hpp"
#include "CGuiStaticImage.hpp"
#include "CGuiPane.hpp"
#include "CAuiImagePane.hpp"
#include "CAuiMeter.hpp"
#include "CGuiModel.hpp"
#include "CGuiTableGroup.hpp"
#include "CGuiSliderGroup.hpp"
#include "CGuiTextPane.hpp"
#include "CAuiEnergyBarT01.hpp"

namespace urde
{

void CGuiSys::AddFactories(EUsageMode /* mode */)
{
    x8_factoryMgr.m_factories.clear();
}

void CGuiSys::LoadWidgetFunctions()
{
}

CGuiWidget* CGuiSys::CreateWidgetInGame(FourCC type, CInputStream& in, CGuiFrame* frame)
{
    switch (type)
    {
    case SBIG('BWIG'):
        return CGuiWidget::Create(frame, in, false);
    case SBIG('HWIG'):
        return CGuiHeadWidget::Create(frame, in, false);
    case SBIG('BGND'):
        return CGuiBackground::Create(frame, in, false);
    case SBIG('LITE'):
        return CGuiLight::Create(frame, in, false);
    case SBIG('CAMR'):
        return CGuiCamera::Create(frame, in, false);
    case SBIG('GRUP'):
        return CGuiGroup::Create(frame, in, false);
    case SBIG('IMAG'):
        return CGuiStaticImage::Create(frame, in, false);
    case SBIG('PANE'):
        return CGuiPane::Create(frame, in, false);
    case SBIG('IMGP'):
        return CAuiImagePane::Create(frame, in, false);
    case SBIG('METR'):
        return CAuiMeter::Create(frame, in, false);
    case SBIG('MODL'):
        return CGuiModel::Create(frame, in, false);
    case SBIG('TBGP'):
        return CGuiTableGroup::Create(frame, in, false);
    case SBIG('SLGP'):
        return CGuiSliderGroup::Create(frame, in, false);
    case SBIG('TXPN'):
        return CGuiTextPane::Create(frame, in, false);
    case SBIG('ENRG'):
        return CAuiEnergyBarT01::Create(frame, in, false);
    default: break;
    }
    return nullptr;
}

CGuiSys::CGuiSys(IFactory& resFactory, CSimplePool& resStore, EUsageMode mode)
: x0_resFactory(resFactory), x4_resStore(resStore), x2c_mode(mode),
  x38_frameFactoryParams(new TObjOwnerParam<CGuiResFrameData>(CGuiResFrameData(*this))),
  x40_constructTime(std::chrono::steady_clock::now())
{
    AddFactories(mode);
    LoadWidgetFunctions();

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::Seventeen,
    CGuiAutoRepeatData(EPhysicalControllerID::Seventeen, EPhysicalControllerID::Eighteen)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::Nineteen,
    CGuiAutoRepeatData(EPhysicalControllerID::Nineteen, EPhysicalControllerID::Twenty)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::TwentyOne,
    CGuiAutoRepeatData(EPhysicalControllerID::TwentyOne, EPhysicalControllerID::TwentyTwo)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::TwentyThree,
    CGuiAutoRepeatData(EPhysicalControllerID::TwentyThree, EPhysicalControllerID::TwentyFour)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::ThirtyFive,
    CGuiAutoRepeatData(EPhysicalControllerID::ThirtyFive, EPhysicalControllerID::ThirtySix)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::ThirtySeven,
    CGuiAutoRepeatData(EPhysicalControllerID::ThirtySeven, EPhysicalControllerID::ThirtyEight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::ThirtyNine,
    CGuiAutoRepeatData(EPhysicalControllerID::ThirtyNine, EPhysicalControllerID::Forty)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::FortyOne,
    CGuiAutoRepeatData(EPhysicalControllerID::FortyOne, EPhysicalControllerID::FortyTwo)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::FortyThree,
    CGuiAutoRepeatData(EPhysicalControllerID::FortyThree, EPhysicalControllerID::FortyFour)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::FortyFive,
    CGuiAutoRepeatData(EPhysicalControllerID::FortyFive, EPhysicalControllerID::FortySix)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::FortySeven,
    CGuiAutoRepeatData(EPhysicalControllerID::FortySeven, EPhysicalControllerID::FortyEight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::TwentyFive,
    CGuiAutoRepeatData(EPhysicalControllerID::TwentyFive, EPhysicalControllerID::TwentySix)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::TwentySeven,
    CGuiAutoRepeatData(EPhysicalControllerID::TwentySeven, EPhysicalControllerID::TwentyEight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::TwentyNine,
    CGuiAutoRepeatData(EPhysicalControllerID::TwentyNine, EPhysicalControllerID::Thirty)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::ThirtyOne,
    CGuiAutoRepeatData(EPhysicalControllerID::ThirtyOne, EPhysicalControllerID::ThirtyTwo)));
}

}
