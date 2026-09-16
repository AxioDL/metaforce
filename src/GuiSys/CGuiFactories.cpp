#include "GuiSys/CAuiEnergyBarT01.hpp"
#include "GuiSys/CAuiImagePane.hpp"
#include "GuiSys/CAuiMeter.hpp"
#include "GuiSys/CGuiCamera.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiGroup.hpp"
#include "GuiSys/CGuiHeadWidget.hpp"
#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiPane.hpp"
#include "GuiSys/CGuiSliderGroup.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CVParamTransfer.hpp"

const CFactoryFnReturn RGuiFrameFactoryInGame(const SObjectTag& tag, CInputStream& in,
                                      const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CGuiFrame* const frame =
      CGuiFrame::CreateFrame(tag.GetId(), *CGuiSys::GetGlobalGuiSys(), in, pool);
  return frame;
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
CGuiWidget* FGuiWidgetFactoryInGame(FourCC type, CGuiFrame* frame, CInputStream& in,
                                    CSimplePool* sp, uint version) {
#else
CGuiWidget* FGuiWidgetFactoryInGame(FourCC type, CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
#endif
  switch (type) {
  case 'HWIG':
    return CGuiHeadWidget::Create(frame, in, sp);
  case 'BWIG':
    return CGuiWidget::Create(frame, in, sp);
  case 'CAMR':
    return CGuiCamera::Create(frame, in, sp);
  case 'GRUP':
    return CGuiGroup::Create(frame, in, sp);
  case 'MODL':
    return CGuiModel::Create(frame, in, sp);
  case 'SLGP':
    return CGuiSliderGroup::Create(frame, in, sp);
  case 'TBGP':
    return CGuiTableGroup::Create(frame, in, sp);
  case 'PANE':
    return CGuiPane::Create(frame, in, sp);
  case 'TXPN':
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    return CGuiTextPane::Create(frame, in, sp, version);
#else
    return CGuiTextPane::Create(frame, in, sp);
#endif
  case 'LITE':
    return CGuiLight::Create(frame, in, sp);
  case 'ENRG':
    return CAuiEnergyBarT01::Create(frame, in, sp);
  case 'METR':
    return CAuiMeter::Create(frame, in, sp);
  case 'IMGP':
    return CAuiImagePane::Create(frame, in, sp);
  default:
    return nullptr;
  }
}
