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
#include "CTextParser.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CGuiSys* g_GuiSys = nullptr;

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
  x34_textParser(new CTextParser(resStore)),
  x40_constructTime(std::chrono::steady_clock::now())
{
    AddFactories(mode);
    LoadWidgetFunctions();

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::DPadUpInst,
    CGuiAutoRepeatData(EPhysicalControllerID::DPadUpInst, EPhysicalControllerID::DPadUp)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::DPadRightInst,
    CGuiAutoRepeatData(EPhysicalControllerID::DPadRightInst, EPhysicalControllerID::DPadRight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::DPadDownInst,
    CGuiAutoRepeatData(EPhysicalControllerID::DPadDownInst, EPhysicalControllerID::DPadDown)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::DPadLeftInst,
    CGuiAutoRepeatData(EPhysicalControllerID::DPadLeftInst, EPhysicalControllerID::DPadLeft)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftStickRightInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftStickRightInst, EPhysicalControllerID::LeftStickRight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftStickDownInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftStickDownInst, EPhysicalControllerID::LeftStickDown)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftStickLeftInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftStickLeftInst, EPhysicalControllerID::LeftStickLeft)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::RightStickUpInst,
    CGuiAutoRepeatData(EPhysicalControllerID::RightStickUpInst, EPhysicalControllerID::RightStickUp)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::RightStickRightInst,
    CGuiAutoRepeatData(EPhysicalControllerID::RightStickRightInst, EPhysicalControllerID::RightStickRight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::RightStickDownInst,
    CGuiAutoRepeatData(EPhysicalControllerID::RightStickDownInst, EPhysicalControllerID::RightStickDown)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::RightStickLeftInst,
    CGuiAutoRepeatData(EPhysicalControllerID::RightStickLeftInst, EPhysicalControllerID::RightStickLeft)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftUpInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftUpInst, EPhysicalControllerID::LeftUp)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftRightInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftRightInst, EPhysicalControllerID::LeftRight)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftDownInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftDownInst, EPhysicalControllerID::LeftDown)));

    x18_repeatMap.emplace(std::make_pair(EPhysicalControllerID::LeftLeftInst,
    CGuiAutoRepeatData(EPhysicalControllerID::LeftLeftInst, EPhysicalControllerID::LeftLeft)));
}

}
